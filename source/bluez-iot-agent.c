///////////////////////////////////////////////////////////////////////////////
// NAME:            bluez-iot-agent.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the application
//
// CREATED:         11/20/2021
//
// LAST EDITED:     11/27/2021
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
#include <stdbool.h>

#include <glib.h>
#include <glib-unix.h>
#include <libsoup/soup.h>

#include <agent-server.h>
#include <bluez.h>
#include <bluez-agent.h>
#include <bluez-client.h>
#include <config.h>
#include <state.h>
#include <web-server.h>

const char* argp_program_name = CONFIG_PROGRAM_NAME " " CONFIG_PROGRAM_VERSION;
const char* argp_program_bug_address = "<ethan.twardy@gmail.com>";

static char doc[] = "Modern pairing wizard for Bluetooth IoT devices on Linux";

static error_t parse_opt(int, char*, struct argp_state*);
static const struct argp_option options[] = {
    { "no-register-name", 'n', NULL, OPTION_ARG_OPTIONAL,
      "Don't attempt to register the service name with D-Bus", 0 },
    { 0 },
};
static struct argp argp = { options, parse_opt, NULL, doc, NULL, NULL, NULL };

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

static void register_handlers(GDBusConnection* connection, const gchar* name,
    const gpointer user_data)
{
    AgentServer* agent_server = (AgentServer*)user_data;
    IotAgentAgent1* interface = iot_agent_agent1_skeleton_new();
    GError* error = NULL;
    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(interface),
        connection, CONFIG_OBJECT_PATH, &error);
    g_signal_connect(interface, "handle-cancel",
        G_CALLBACK(agent_server->Cancel), NULL);
    g_info("Agent listening on D-Bus at dest=%s,path=%s", name,
        CONFIG_OBJECT_PATH);
    if (NULL != error) {
        g_error("Couldn't register object: %s", error->message);
    }
}

static void name_lost(GDBusConnection* connection, const gchar* name,
    gpointer user_data)
{
    g_error("Lost name on connection, or unable to own name");
}

static int signal_handler(gpointer user_data) {
    // All attached signal sources just cause the loop to exit gracefully
    StatePublisher* publisher = (StatePublisher*)user_data;
    state_set(publisher, STATE_SHUTDOWN);
    return 0;
}

int main(int argc, char** argv) {
    bool register_name = true;
    argp_parse(&argp, argc, argv, 0, 0, &register_name);

    GError* error = NULL;
    GDBusConnection* connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL,
        &error);
    if (NULL != error) {
        g_error("Couldn't connect to bus: %s", error->message);
    }

    // State machine
    StatePublisher* state_publisher = state_init();

    AgentServer* agent_server = agent_server_init(state_publisher);
    if (NULL == agent_server) {
        g_error("Couldn't initialize agent server: %s", strerror(errno));
    }

    if (register_name) {
        g_bus_own_name_on_connection(connection, CONFIG_SERVICE_NAME,
            G_BUS_NAME_OWNER_FLAGS_NONE, register_handlers, name_lost,
            agent_server, NULL);
    } else {
        const gchar* service_name = g_dbus_connection_get_unique_name(
            connection);
        register_handlers(connection, service_name, agent_server);
    }

    GMainLoop* main_loop = g_main_loop_new(NULL, FALSE);
    GMainContext* main_context = g_main_loop_get_context(main_loop);

    // Signal handlers for graceful shutdown
    GSource* signal_source = g_unix_signal_source_new(SIGINT);
    g_source_set_callback(signal_source, signal_handler, state_publisher,
        NULL);
    g_source_attach(signal_source, main_context);

    // Web Server
    const char* webroot_path = getenv("AGENT_WEBROOT");
    if (NULL == webroot_path) {
        webroot_path = CONFIG_WEBROOT_PATH;
    }
    WebServer* web_server = web_server_init(webroot_path, state_publisher);
    SoupServer* soup_server = soup_server_new("tls-certificate", NULL,
        "raw-paths", FALSE, "server-header", argp_program_name, NULL);
    soup_server_add_handler(soup_server, "/", web_server->handle_connection,
        web_server, NULL);
    soup_server_listen_all(soup_server, CONFIG_WEB_SERVER_PORT, 0, &error);
    g_info("Web server listening at 0.0.0.0:%d", CONFIG_WEB_SERVER_PORT);

    // bluetoothd D-Bus client
    BluezClient* bluez_client = bluez_client_init(state_publisher, connection);
    bluez_client_setup_agent(bluez_client, CONFIG_OBJECT_PATH,
        CONFIG_AGENT_CAPABILITY);

    // Bring up in STATE_CONNECTION_WAIT, then do the main loop
    state_set(state_publisher, STATE_CONNECTION_WAIT);
    while (STATE_SHUTDOWN != state_get(state_publisher)) {
        state_do_entry(state_publisher);
        g_main_context_iteration(main_context, FALSE);
        state_do_exit(state_publisher);
    }

    g_info("Exiting gracefully");
    state_do_entry(state_publisher); // <- need to "enter" STATE_SHUTDOWN
    bluez_client_free(&bluez_client);
    web_server_free(&web_server);
    agent_server_free(&agent_server);
    state_deref(&state_publisher);
}

///////////////////////////////////////////////////////////////////////////////
