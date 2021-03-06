///////////////////////////////////////////////////////////////////////////////
// NAME:            bluez-client.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the BlueZ Client
//
// CREATED:         11/27/2021
//
// LAST EDITED:     11/29/2021
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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <bluez-client.h>
#include <bluez.h>
#include <config.h>
#include <state.h>

typedef struct BluezClient {
    AgentManager1* manager;
    Adapter1* adapter;
} BluezClient;

static const char* BLUEZ_SERVICE = "org.bluez";
static const char* BLUEZ_OBJECT_PATH = "/org/bluez";

///////////////////////////////////////////////////////////////////////////////
// Private API
////

static void do_enter_connection_wait(BluezClient* bluez_client)
{
    g_info("BluezClient: State CONNECTION_WAIT");
    // Configure Bluez to automatically disable discoverable mode after 60
    // seconds
    adapter1_set_discoverable(bluez_client->adapter, true);
}

static void do_enter_connected(BluezClient* bluez_client)
{ g_info("BluezClient: State CONNECTED"); }

static void do_enter_pairable(BluezClient* bluez_client)
{ g_info("BluezClient: State PAIRABLE"); }

static void do_enter_shutdown(BluezClient* bluez_client)
{
    g_info("BluezClient: State SHUTDOWN");
    adapter1_set_discoverable(bluez_client->adapter, false);
}

static void priv_on_entry(enum State state, void* user_data) {
    BluezClient* client = (BluezClient*)user_data;
    switch (state) {
    case STATE_CONNECTION_WAIT:
        do_enter_connection_wait(client);
        break;
    case STATE_CONNECTED:
        do_enter_connected(client);
        break;
    case STATE_PAIRABLE:
        do_enter_pairable(client);
        break;
    case STATE_SHUTDOWN:
        do_enter_shutdown(client);
        break;
    default: break;
    }
}

static void priv_on_exit(enum State state, void* user_data)
{}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

BluezClient* bluez_client_init(StatePublisher* state_publisher,
    GDBusConnection* connection, const char* device)
{
    BluezClient* client = malloc(sizeof(BluezClient));
    if (NULL == client) {
        return NULL;
    }

    // Set up Agent
    GError* error = NULL;
    client->manager = agent_manager1_proxy_new_sync(connection,
        G_DBUS_PROXY_FLAGS_NONE, BLUEZ_SERVICE, BLUEZ_OBJECT_PATH, NULL,
        &error);
    if (NULL != error) {
        g_error("Failed to set up bluetoothd D-Bus proxy: %s", error->message);
        g_error_free(error);
    }

    const size_t prefix_length = strlen(CONFIG_ADAPTER_PATH_PREFIX);
    const size_t device_length = strlen(device);
    char* object_path = malloc(prefix_length + 1 + device_length + 1);
    if (NULL == object_path) {
        goto error;
    }

    strcpy(object_path, CONFIG_ADAPTER_PATH_PREFIX);
    object_path[prefix_length] = '/';
    strcpy(object_path + prefix_length + 1, device);
    object_path[prefix_length + 1 + device_length] = '\0';
    client->adapter = adapter1_proxy_new_sync(connection,
        G_DBUS_PROXY_FLAGS_NONE, BLUEZ_SERVICE, object_path, NULL,
        &error);
    free(object_path);
    if (NULL == client->adapter) {
        g_error("Failed to set up bluetoothd D-Bus proxy: %s", error->message);
        g_error_free(error);
    }

    if (0 != state_add_observer(state_publisher, priv_on_exit, priv_on_entry,
            client)) {
        goto error;
    }

    return client;
 error:
    free(client);
    return NULL;
}

void bluez_client_setup_agent(BluezClient* bluez_client,
    const char* object_path, const char* capability)
{
    GError* error = NULL;

    // Register agent with BlueZ service
    agent_manager1_call_register_agent_sync(bluez_client->manager, object_path,
        capability, NULL, &error);
    if (NULL != error) {
        g_error("Failed to regster agent with bluetoothd: %s", error->message);
        g_error_free(error);
    }

    // Request to become the default agent
    agent_manager1_call_request_default_agent_sync(bluez_client->manager,
        object_path, NULL, &error);
    if (NULL != error) {
        g_error("Failed to become the default agent: %s", error->message);
        g_error_free(error);
    }
}

void bluez_client_free(BluezClient** client) {
    if (NULL == *client) {
        return;
    }

    free(*client);
    *client = NULL;
}

///////////////////////////////////////////////////////////////////////////////
