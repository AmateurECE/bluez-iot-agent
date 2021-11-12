///////////////////////////////////////////////////////////////////////////////
// NAME:            dbus-bluez.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of helper methods for D-Bus/BlueZ comm
//
// CREATED:         11/11/2021
//
// LAST EDITED:     11/11/2021
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

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>

#include "dbus-bluez.h"
#include "logger.h"

static const char* ORG_BLUEZ_SERVICE = "org.bluez";
static const char* ORG_BLUEZ_OBJECT = "/org/bluez";
static const char* AGENT_MANAGER_INTERFACE = "org.bluez.AgentManager1";

static const char* AGENT_OBJECT = "/org/bluez/agent";
static const char* AGENT_CAPABILITY = "NoInputNoOutput";

// Assuming that server is on the bus, invoke the RegisterAgent() method of
// org.bluez service to register an agent for ourselves.
int bluez_register_agent(AgentServer* server) {
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
    if (!dbus_message_iter_append_basic(&arguments, DBUS_TYPE_OBJECT_PATH,
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

int bluez_make_default_agent(AgentServer* server __attribute__((unused))) {
    return -1;
}

///////////////////////////////////////////////////////////////////////////////
