///////////////////////////////////////////////////////////////////////////////
// NAME:            agent-server.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the agent-server.
//
// CREATED:         11/07/2021
//
// LAST EDITED:     11/07/2021
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

AgentServer* agent_server_start() {
    AgentServer* server = malloc(sizeof(AgentServer));
    if (NULL == server) {
        return NULL;
    }

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
        fprintf(stderr, "Connection Error (%s)\n", server->error->message);
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
        fprintf(stderr, "Name Error (%s)\n", server->error->message);
        dbus_error_free(server->error);
    }
    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != result) {
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

    dbus_connection_close((*server)->connection);
    free((*server)->error);
    free(*server);
    *server = NULL;
}

///////////////////////////////////////////////////////////////////////////////
