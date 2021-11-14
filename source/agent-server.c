///////////////////////////////////////////////////////////////////////////////
// NAME:            agent-server.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the agent-server.
//
// CREATED:         11/07/2021
//
// LAST EDITED:     11/14/2021
//
// Copyright 2021, Ethan D. Twardy
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
////

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <libseastar/vector.h>
#include <libseastar/error.h>

#include "agent-callbacks.h"
#include "agent-server.h"
#include "dbus-bluez.h"
#include "logger.h"

static const char* SERVICE_BUS_NAME = "org.soundsystem.agent";
static const char* AGENT_OBJECT_PATH = "/org/bluez/agent";
static const char* AGENT_CAPABILITY = "NoInputNoOutput";

static const DBusObjectPathVTable agent_object_interface = {
    .unregister_function = agent_object_path_unregister_function,
    .message_function = agent_object_path_message_function,
    0,
};

// Obtain a connection to the bus and request our service name
static DBusConnection* agent_setup_dbus_connection(Logger* logger,
    DBusError* error)
{
    // Get a connection to the bus
    DBusConnection* connection = dbus_bus_get(DBUS_BUS_SYSTEM, error);
    if (dbus_error_is_set(error)) {
        LOG_ERROR(logger, "connection error: %s", error->message);
        dbus_error_free(error);
    }
    if (NULL == connection) {
        return NULL;
    }

    // Request a name on the bus
    int result = dbus_bus_request_name(connection, SERVICE_BUS_NAME,
        DBUS_NAME_FLAG_REPLACE_EXISTING, error);
    if (dbus_error_is_set(error)) {
        LOG_ERROR(logger, "name error: %s", error->message);
        dbus_error_free(error);
    }
    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != result) {
        dbus_connection_unref(connection);
        return NULL;
    }
    return connection;
}

// Register the object path implementing the org.bluez.Agent1 interface
static int agent_setup_dbus_interface(AgentServer* server) {
    if (!dbus_connection_set_timeout_functions(server->connection,
            agent_add_timeout_function, agent_remove_timeout_function,
            agent_timeout_toggled_function, server, NULL)) {
        LOG_ERROR(server->logger, "out of memory or callback failure");
        return 1;
    }

    if (!dbus_connection_set_watch_functions(server->connection,
            agent_add_watch_function, agent_remove_watch_function,
            agent_watch_toggled_function, server, NULL)) {
        LOG_ERROR(server->logger, "out of memory or callback failure");
        return 1;
    }

    if (!dbus_connection_register_object_path(server->connection,
            AGENT_OBJECT_PATH, &agent_object_interface, server)) {
        LOG_ERROR(server->logger, "either no memory is available or object "
            "path is already in use");
        return 1;
    }
    return 0;
}

// Entrypoint for the server.
AgentServer* agent_server_start(Logger* logger) {
    AgentServer* server = malloc(sizeof(AgentServer));
    if (NULL == server) {
        return NULL;
    }

    server->logger = logger;
    server->epoll_fd = epoll_create(1);
    if (-1 == server->epoll_fd) {
        LOG_ERROR(logger, "Couldn't create an epoll file descriptor: %s",
            strerror(errno));
        goto error_epoll_create;
    }

    server->watches = malloc(sizeof(Vector));
    if (NULL == server->watches) {
        LOG_ERROR(logger, "Couldn't allocate container for watches: %s",
            strerror(errno));
        goto error_malloc_watches;
    }

    VoidResult result = cs_vector_init(server->watches);
    if (!result.ok) {
        if (result.error & SEASTAR_ERRNO_SET) {
            LOG_ERROR(logger, "Failure initializing vector: %s",
                strerror(result.error ^ SEASTAR_ERRNO_SET));
        } else {
            LOG_ERROR(logger, "Failure initializing vector: %s",
                cs_strerror(result.error));
        }
        goto error_vector_init;
    }

    server->timeouts = malloc(sizeof(PriorityQueue));
    if (NULL == server->timeouts) {
        LOG_ERROR(logger, "Failure initializing timeout queue: %s",
            strerror(errno));
        goto error_malloc_timeouts;
    }

    result = cs_pqueue_init(server->timeouts, agent_timeout_comparator);
    if (!result.ok) {
        if (result.error & SEASTAR_ERRNO_SET) {
            LOG_ERROR(logger, "failure to initialize pqueue: %s",
                strerror(result.error ^ SEASTAR_ERRNO_SET));
        } else {
            LOG_ERROR(logger, "Failure initializing timeout queue: %s",
                cs_strerror(result.error));
        }
        goto error_pqueue_init;
    }

    server->error = malloc(sizeof(DBusError));
    if (NULL == server->error) {
        goto error_malloc_error;
    }

    dbus_error_init(server->error);
    server->connection = agent_setup_dbus_connection(logger, server->error);
    if (NULL == server->connection) {
        goto error_dbus_connect;
    }

    LOG_INFO(logger, "Setting up Agent interface");
    if (0 != agent_setup_dbus_interface(server)) {
        goto error_dbus_setup;
    }

    LOG_INFO(logger, "Registering the agent with org.bluez");
    if (0 != bluez_register_agent(server, AGENT_OBJECT_PATH,
            AGENT_CAPABILITY)) {
        goto error_dbus_setup;
    }

    LOG_INFO(logger, "Requesting default-agent");
    if (0 != bluez_make_default_agent(server, AGENT_OBJECT_PATH)) {
        goto error_dbus_setup;
    }

    return server;

 error_dbus_setup:
    dbus_connection_unref(server->connection);
 error_dbus_connect:
    free(server->error);
 error_malloc_error:
    cs_pqueue_free(server->timeouts);
 error_pqueue_init:
    free(server->timeouts);
 error_malloc_timeouts:
    cs_vector_free(server->watches);
 error_malloc_watches:
    free(server->watches);
 error_vector_init:
    close(server->epoll_fd);
 error_epoll_create:
    free(server);
    return NULL;
}

// Get a file descriptor that can be used with epoll_wait to wait for events
int agent_server_get_epoll_fd(AgentServer* server) {
    return server->epoll_fd;
}

// Dispatch a request, to handle the epoll event set in events.
int agent_server_dispatch(AgentServer* server, uint32_t events) {
    printf("agent_server_dispatch called!\n");
    return 0;
}

// Free server resources
void agent_server_stop(AgentServer** server) {
    if (NULL == *server) {
        return;
    }

    dbus_connection_unref((*server)->connection);
    free((*server)->error);
    cs_pqueue_free((*server)->timeouts);
    free((*server)->timeouts);
    cs_vector_free((*server)->watches);
    free((*server)->watches);
    close((*server)->epoll_fd);
    free(*server);
    *server = NULL;
}

///////////////////////////////////////////////////////////////////////////////
