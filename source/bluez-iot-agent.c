///////////////////////////////////////////////////////////////////////////////
// NAME:            bluez-iot-agent.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the program
//
// CREATED:         11/05/2021
//
// LAST EDITED:     11/16/2021
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
#include <stdio.h>
#include <unistd.h>

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <ev.h>

#include <bluez-proxy.h>
#include <logger.h>
#include <web-server.h>

static const int HTTP_PORT = 8888;
static const char* SERVICE_NAME = "org.bluez.iot-agent";

static void exit_signal_callback(struct ev_loop* loop, ev_signal* watcher,
    int signal_name)
{
    // Just exit the loop.
    ev_break(loop, EVBREAK_ALL);
}

static void default_log_handler(enum LogLevel level, const char* message) {
    switch (level) {
    case INFO:
    case WARNING:
        printf("%s", message);
        break;
    case ERROR:
        fprintf(stderr, "%s", message);
        break;
    }
}

static DBusConnection* open_dbus_connection(Logger* logger, DBusError* error,
    const char* service_name)
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
    int result = dbus_bus_request_name(connection, service_name,
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

int main() {
    // Use the standard event loop
    struct ev_loop* loop = EV_DEFAULT;

    Logger logger;
    logger_initialize(&logger, default_log_handler);

    // Open D-Bus connection
    DBusError error;
    dbus_error_init(&error);

    DBusConnection* connection = open_dbus_connection(&logger, &error,
        SERVICE_NAME);
    if (NULL == connection) {
        return 1;
    }

    // Prepare AgentServer
    BluezProxy* bluez_proxy = bluez_proxy_initialize(&logger, connection,
        &error);
    if (NULL == bluez_proxy) {
        dbus_connection_unref(connection);
        return 1;
    }

    // web server initialization
    WebServer* web_server = web_server_start(&logger, HTTP_PORT);
    ev_io* web_watcher = web_server_get_watcher(web_server);
    ev_io_start(loop, web_watcher);

    // signal handling
    ev_signal exit_signal;
    ev_init(&exit_signal, exit_signal_callback);
    ev_signal_set(&exit_signal, SIGINT);
    ev_signal_start(loop, &exit_signal);

    // Run event loop
    ev_run(loop, 0);

    web_server_stop(&web_server);
    bluez_proxy_free(&bluez_proxy);
    dbus_connection_unref(connection);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
