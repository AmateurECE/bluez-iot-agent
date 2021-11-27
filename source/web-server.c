///////////////////////////////////////////////////////////////////////////////
// NAME:            web-server.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the web server interface
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

#include <stdlib.h>
#include <stdio.h>

#include <state.h>
#include <web-server.h>

///////////////////////////////////////////////////////////////////////////////
// Private API
////

void handle_connection(SoupServer* server, SoupServerMessage* message,
    const char* path, GHashTable* query, gpointer user_data)
{}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

WebServer* web_server_init(StatePublisher* state_publisher) {
    WebServer* server = malloc(sizeof(WebServer));
    if (NULL == server) {
        return NULL;
    }

    server->handle_connection = handle_connection;

    state_ref(state_publisher);
    server->state_publisher = state_publisher;
    return server;
}

void web_server_free(WebServer** server) {
    if (NULL != *server) {
        state_deref(&(*server)->state_publisher);
        free(*server);
        *server = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
