///////////////////////////////////////////////////////////////////////////////
// NAME:            bluez-proxy.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of helper methods for D-Bus/BlueZ comm
//
// CREATED:         11/11/2021
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

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>

#include <bluez-proxy.h>
#include <logger.h>

static const char* ORG_BLUEZ_SERVICE = "org.bluez";
static const char* ORG_BLUEZ_OBJECT = "/org/bluez";
static const char* AGENT_MANAGER_INTERFACE = "org.bluez.AgentManager1";

///////////////////////////////////////////////////////////////////////////////
// Private API
////

// Assuming that server is on the bus, invoke the RegisterAgent() method of
// org.bluez service to register an agent for ourselves.
static int register_agent(BluezProxy* server, const char* object_path,
    const char* capability)
{
    DBusMessage* message = dbus_message_new_method_call(ORG_BLUEZ_SERVICE,
        ORG_BLUEZ_OBJECT, AGENT_MANAGER_INTERFACE, "RegisterAgent");
    Logger* logger = server->logger;
    if (NULL == message) {
        LOG_ERROR(logger, "message is null");
        return 1;
    }

    // Append arguments to message
    if (!dbus_message_append_args(message,
            DBUS_TYPE_OBJECT_PATH, &object_path,
            DBUS_TYPE_STRING, &capability,
            DBUS_TYPE_INVALID)) {
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

    if (dbus_set_error_from_message(server->error, message)) {
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

static int request_default_agent(BluezProxy* server, const char* object_path)
{
    DBusMessage* message = dbus_message_new_method_call(ORG_BLUEZ_SERVICE,
        ORG_BLUEZ_OBJECT, AGENT_MANAGER_INTERFACE, "RequestDefaultAgent");
    Logger* logger = server->logger;
    if (NULL == message) {
        LOG_ERROR(logger, "dbus message is null");
        return 1;
    }

    if (!dbus_message_append_args(message,
            DBUS_TYPE_OBJECT_PATH, &object_path,
            DBUS_TYPE_INVALID)) {
        LOG_ERROR(logger, "out of memory");
        dbus_message_unref(message);
        return 1;
    }

    // TODO: Basically from here, this can be a generic static method
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

    // Wait for a response
    dbus_pending_call_block(pending);
    message = dbus_pending_call_steal_reply(pending);
    if (NULL == message) {
        LOG_ERROR(logger, "out of memory");
        return 1;
    }
    dbus_pending_call_unref(pending);

    if (dbus_set_error_from_message(server->error, message)) {
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

///////////////////////////////////////////////////////////////////////////////
// Public API
////

BluezProxy* bluez_proxy_init(Logger* logger, DBusConnection* connection,
    DBusError* error)
{
    BluezProxy* proxy = malloc(sizeof(BluezProxy));
    if (NULL == proxy) {
        LOG_ERROR(logger, "Failed to allocate memory for BlueZ proxy: %s",
            strerror(errno));
    }

    proxy->RegisterAgent = register_agent;
    proxy->RequestDefaultAgent = request_default_agent;
    proxy->connection = connection;
    proxy->error = error;
    proxy->logger = logger;
    return proxy;
}

DBusError* bluez_proxy_get_error(BluezProxy* proxy) {
    return proxy->error;
}

void bluez_proxy_free(BluezProxy** proxy) {
    if (NULL != *proxy) {
        free(*proxy);
        *proxy = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
