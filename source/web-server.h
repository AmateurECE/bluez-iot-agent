///////////////////////////////////////////////////////////////////////////////
// NAME:            web-server.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     A web server for the user interface
//
// CREATED:         11/20/2021
//
// LAST EDITED:     11/20/2021
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

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

typedef struct _SoupServer SoupServer;
typedef struct _SoupServerMessage SoupServerMessage;
typedef struct _GHashTable GHashTable;
typedef void* gpointer;

typedef struct WebServer {
    void (*handle_connection)(SoupServer* server, SoupServerMessage* message,
        const char* path, GHashTable* query, gpointer user_data);
} WebServer;

WebServer* web_server_init();
void web_server_free(WebServer**);

#endif // WEB_SERVER_H

///////////////////////////////////////////////////////////////////////////////
