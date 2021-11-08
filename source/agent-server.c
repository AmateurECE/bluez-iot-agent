///////////////////////////////////////////////////////////////////////////////
// NAME:            agent-server.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the agent-server.
//
// CREATED:         11/07/2021
//
// LAST EDITED:     11/08/2021
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

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>

#include "agent-server.h"
#include "logger.h"

static const char* ORG_BLUEZ_SERVICE = "org.bluez";
static const char* ORG_BLUEZ_OBJECT = "/org/bluez";
static const char* AGENT_MANAGER_INTERFACE = "org.bluez.AgentManager1";

static const char* AGENT_OBJECT = "/org/bluez/agent";
static const char* AGENT_CAPABILITY = "NoInputNoOutput";

// Assuming that server is on the bus, invoke the RegisterAgent() method of
// org.bluez service to register an agent for ourselves.
static int bluez_register_agent(AgentServer* server) {
    DBusMessage* message = dbus_message_new_method_call(ORG_BLUEZ_SERVICE,
        ORG_BLUEZ_OBJECT, AGENT_MANAGER_INTERFACE, "RegisterAgent");
    Logger* logger = server->logger;
    if (NULL == message) {
        LOG_ERROR(logger, "message is null");
        return 1;
    }

    // Append arguments to message
    DBusMessageIter arguments;
    dbus_message_iter_init_append(message, &arguments);
    // TODO: This needs to be an object path
    if (!dbus_message_iter_append_basic(&arguments, DBUS_TYPE_STRING,
            &AGENT_OBJECT)
        || !dbus_message_iter_append_basic(&arguments, DBUS_TYPE_STRING,
            &AGENT_CAPABILITY)) {
        LOG_ERROR(logger, "out of memory");
        dbus_message_unref(message);
        return 1;
    }

    DBusPendingCall* pending;
    if (!dbus_connection_send_with_reply(server->connection, message, &pending,
            -1)) {
        LOG_ERROR(logger, "out of memory");
        dbus_message_unref(message);
        return 1;
    }
    if (NULL == pending) {
        LOG_ERROR(logger, "out of memory");
        dbus_message_unref(message);
        return 1;
    }
    dbus_connection_flush(server->connection);

    dbus_message_unref(message);

    // If something bad happened, we'll get a response with a string back.
    dbus_pending_call_block(pending);
    message = dbus_pending_call_steal_reply(pending);
    if (NULL == message) {
        LOG_ERROR(logger, "out of memory");
        return 1;
    }
    dbus_pending_call_unref(pending);

    if (DBUS_MESSAGE_TYPE_ERROR == dbus_message_get_type(message)) {
        dbus_set_error_from_message(server->error, message);
        const char* error_name = dbus_message_get_error_name(message);
        dbus_message_unref(message);

        LOG_ERROR(logger, "Got D-Bus Error while attempting to register agent:"
            " (%s) %s", error_name, server->error->message);
        dbus_error_free(server->error);

        return 1;
    }

    dbus_message_unref(message);
    return 0;
}

// Start the agent server:
//  1. Allocate data structures
//  2. Attach to the bus
//  3. Request the name of our service
//  4. Register agent with bluez
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

    // Initialize error member
    dbus_error_init(server->error);

    // Get a connection to the bus
    server->connection = dbus_bus_get(DBUS_BUS_SYSTEM, server->error);
    if (dbus_error_is_set(server->error)) {
        LOG_ERROR(logger, "connection error: %s", server->error->message);
        dbus_error_free(server->error);
    }
    if (NULL == server->connection) {
        free(server->error);
        free(server);
        return NULL;
    }

    // Request a name on the bus
    int result = dbus_bus_request_name(server->connection, BUS_NAME,
        DBUS_NAME_FLAG_REPLACE_EXISTING, server->error);
    if (dbus_error_is_set(server->error)) {
        LOG_ERROR(logger, "name error: %s", server->error->message);
        dbus_error_free(server->error);
    }
    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != result) {
        free(server->error);
        free(server);
        return NULL;
    }

    LOG_INFO(logger, "Registering the agent with org.bluez");
    if (0 != bluez_register_agent(server)) {
        free(server->error);
        free(server);
        return NULL;
    }

    return server;
}

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
