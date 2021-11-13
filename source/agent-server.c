///////////////////////////////////////////////////////////////////////////////
// NAME:            agent-server.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the agent-server.
//
// CREATED:         11/07/2021
//
// LAST EDITED:     11/12/2021
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

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>

#include "agent-callbacks.h"
#include "agent-server.h"
#include "dbus-bluez.h"
#include "logger.h"

static const char* SERVICE_BUS_NAME = "org.soundsystem.agent";
static const char* AGENT_OBJECT_PATH = "/org/bluez/agent";
static const char* AGENT_CAPABILITY = "NoInputNoOutput";

static const DBusObjectPathVTable agent_object_interface = {
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
            agent_timeout_toggled_function, NULL, NULL)) {
        LOG_ERROR(server->logger, "out of memory");
        return 1;
    }

    if (!dbus_connection_set_watch_functions(server->connection,
            agent_add_watch_function, agent_remove_watch_function,
            agent_watch_toggled_function, NULL, NULL)) {
        LOG_ERROR(server->logger, "out of memory");
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
    server->error = malloc(sizeof(DBusError));
    if (NULL == server->error) {
        free(server);
        return NULL;
    }

    dbus_error_init(server->error);
    server->connection = agent_setup_dbus_connection(logger, server->error);
    if (NULL == server->connection) {
        free(server->error);
        free(server);
        return NULL;
    }

    LOG_INFO(logger, "Setting up Agent interface");
    if (0 != agent_setup_dbus_interface(server)) {
        goto error_encountered;
    }

    LOG_INFO(logger, "Registering the agent with org.bluez");
    if (0 != bluez_register_agent(server, AGENT_OBJECT_PATH,
            AGENT_CAPABILITY)) {
        goto error_encountered;
    }

    LOG_INFO(logger, "Requesting default-agent");
    if (0 != bluez_make_default_agent(server, AGENT_OBJECT_PATH)) {
        goto error_encountered;
    }

    return server;

 error_encountered:
    dbus_connection_unref(server->connection);
    free(server->error);
    free(server);
    return NULL;
}

// Free server resources
void agent_server_stop(AgentServer** server) {
    if (NULL == *server) {
        return;
    }

    dbus_connection_unref((*server)->connection);
    free((*server)->error);
    free(*server);
    *server = NULL;
}

///////////////////////////////////////////////////////////////////////////////
