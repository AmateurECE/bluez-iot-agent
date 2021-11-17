///////////////////////////////////////////////////////////////////////////////
// NAME:            bluez-iot-agent.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the program
//
// CREATED:         11/05/2021
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

#include <argp.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <ev.h>

#include <agent-server.h>
#include <bluez-proxy.h>
#include <config.h>
#include <logger.h>
#include <timeout-manager.h>
#include <watch-manager.h>
#include <web-server.h>

static const int HTTP_PORT = 8888;
static const char* SERVICE_NAME = "org.bluez.iot-agent";

const char* argp_program_version = PROGRAM_NAME " " PROGRAM_VERSION;
const char* argp_program_bug_address = "<ethan.twardy@gmail.com>";
static char doc[] = "Modern pairing wizard for Bluetooth devices on Linux";

static error_t parse_opt(int key, char* arg, struct argp_state* state);
static const struct argp_option options[] = {
    { "no-register-name", 'n', NULL, OPTION_ARG_OPTIONAL,
      "Don't attempt to register the service name with D-Bus", 0 },
    { 0 },
};
static struct argp argp = { options, parse_opt, NULL, doc, 0, 0, 0 };

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
    bool* register_name = state->input;
    switch (key) {
    case 'n':
        *register_name = false;
        break;
    case ARGP_KEY_END:
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

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
    if (NULL != service_name) {
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
    }
    return connection;
}

typedef struct BusEventManagers {
    bool ok;
    TimeoutManager* timeout_manager;
    WatchManager* watch_manager;
} BusEventManagers;

BusEventManagers setup_managers(Logger* logger, DBusConnection* connection,
    struct ev_loop* loop)
{
    WatchManager* watch_manager = watch_manager_init(logger, loop);
    if (NULL == watch_manager) {
        return (BusEventManagers){ .ok=false, 0 };
    }

    TimeoutManager* timeout_manager = timeout_manager_init(logger, loop);
    if (NULL == timeout_manager) {
        watch_manager_free(&watch_manager);
        return (BusEventManagers){ .ok=false, 0 };
    }

    if (!dbus_connection_set_timeout_functions(connection,
            timeout_manager->AddTimeout, timeout_manager->RemoveTimeout, NULL,
            timeout_manager, NULL)) {
        LOG_ERROR(logger, "out of memory or callback failure");
        timeout_manager_free(&timeout_manager);
        watch_manager_free(&watch_manager);
        return (BusEventManagers){ .ok=false, 0 };
    }

    if (!dbus_connection_set_watch_functions(connection,
            watch_manager->AddWatch, watch_manager->RemoveWatch, NULL,
            watch_manager, NULL)) {
        LOG_ERROR(logger, "out of memory or callback failure");
        timeout_manager_free(&timeout_manager);
        watch_manager_free(&watch_manager);
        return (BusEventManagers){ .ok=false, 0 };
    }

    return (BusEventManagers){ .ok=true, .timeout_manager=timeout_manager,
        .watch_manager=watch_manager };
}

int main(int argc, char** argv) {
    bool register_name = true;
    argp_parse(&argp, argc, argv, 0, 0, &register_name);

    // Use the standard event loop
    struct ev_loop* loop = EV_DEFAULT;
    int result = 0;
    Logger logger;
    logger_initialize(&logger, default_log_handler);
    DBusError error;
    dbus_error_init(&error);

    // Attach to D-Bus and register our name
    const char* service_name = SERVICE_NAME;
    if (!register_name) {
        service_name = NULL;
    }
    DBusConnection* connection = open_dbus_connection(&logger, &error,
        service_name);
    if (NULL == connection) {
        return 1;
    }

    // Set up Watch/Timeout managers
    BusEventManagers bus_event_managers = setup_managers(&logger, connection,
        loop);
    if (!bus_event_managers.ok) {
        goto error_manager_setup;
    }

    // Set up BlueZ bus proxy
    BluezProxy* bluez_proxy = bluez_proxy_init(&logger, connection, &error);
    if (NULL == bluez_proxy) {
        result = 1;
        goto error_bluez_proxy;
    }

    // Set up AgentServer
    AgentServer* agent_server = agent_server_start(&logger, connection, &error,
        bluez_proxy);
    if (NULL == agent_server) {
        result = 1;
        goto error_agent_server;
    }

    // web server initialization
    WebServer* web_server = web_server_start(&logger, HTTP_PORT);
    if (NULL == web_server) {
        result = 1;
        goto error_web_server;
    }

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
 error_web_server:
    agent_server_stop(&agent_server);
 error_agent_server:
    bluez_proxy_free(&bluez_proxy);
 error_bluez_proxy:
    timeout_manager_free(&bus_event_managers.timeout_manager);
    watch_manager_free(&bus_event_managers.watch_manager);
 error_manager_setup:
    dbus_connection_unref(connection);
    return result;
}

///////////////////////////////////////////////////////////////////////////////
