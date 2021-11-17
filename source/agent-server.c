///////////////////////////////////////////////////////////////////////////////
// NAME:            agent-server.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the agent-server.
//
// CREATED:         11/07/2021
//
// LAST EDITED:     11/17/2021
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

#include <agent-server.h>
#include <bluez-proxy.h>
#include <logger.h>

static const char* AGENT_OBJECT_PATH = "/org/bluez/agent";
static const char* AGENT_CAPABILITY = "NoInputNoOutput";

static void agent_object_path_unregister_function(DBusConnection* connection,
    void* user_data);
static DBusHandlerResult agent_object_path_message_function(
    DBusConnection* connection, DBusMessage* message, void* user_data);

static const DBusObjectPathVTable agent_object_interface = {
    .unregister_function = agent_object_path_unregister_function,
    .message_function = agent_object_path_message_function,
    0,
};

///////////////////////////////////////////////////////////////////////////////
// Private Interface
////

static void agent_object_path_unregister_function(DBusConnection* connection,
    void* user_data)
{}

static DBusHandlerResult agent_object_path_message_function(
    DBusConnection* connection, DBusMessage* message, void* user_data)
{
    return DBUS_HANDLER_RESULT_HANDLED;
}

///////////////////////////////////////////////////////////////////////////////
// Public Interface
////

AgentServer* agent_server_start(Logger* logger, DBusConnection* connection,
    DBusError* error, BluezProxy* bluez_proxy, struct ev_loop* event_loop)
{
    AgentServer* server = malloc(sizeof(AgentServer));
    if (NULL == server) {
        return NULL;
    }

    server->logger = logger;
    server->connection = connection;
    server->error = error;
    server->event_loop = event_loop;

    if (!dbus_connection_register_object_path(server->connection,
            AGENT_OBJECT_PATH, &agent_object_interface, server)) {
        LOG_ERROR(server->logger, "either no memory is available or object "
            "path is already in use");
        free(server);
        return NULL;
    }

    if (0 != bluez_proxy->RegisterAgent(bluez_proxy, AGENT_OBJECT_PATH,
            AGENT_CAPABILITY)) {
        free(server);
        return NULL;
    }

    if (0 != bluez_proxy->RequestDefaultAgent(bluez_proxy,
            AGENT_OBJECT_PATH)) {
        free(server);
        return NULL;
    }

    return server;
}

// Free server resources
void agent_server_stop(AgentServer** server) {
    if (NULL != *server) {
        free(*server);
        *server = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
