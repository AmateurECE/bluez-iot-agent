///////////////////////////////////////////////////////////////////////////////
// NAME:            web-server.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Web server, serving the user interface to the agent
//
// CREATED:         11/07/2021
//
// LAST EDITED:     11/14/2021
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

#include <stdint.h>

static const int HTTP_PORT = 8888;

struct MHD_Daemon;
typedef struct Logger Logger;

typedef struct WebServer {
    struct MHD_Daemon* daemon;
    Logger* logger;
    int epoll_fd;
} WebServer;

///////////////////////////////////////////////////////////////////////////////
// Public Interface
///

// Start up the web server
int web_server_start(WebServer* server, Logger* logger);

// Get a file descriptor that can be used with epoll_wait to wait for events
int web_server_get_epoll_fd(WebServer* server);

// Dispatch a request, to handle the epoll event set in events.
int web_server_dispatch(WebServer* server, uint32_t events);

// Stop the web server
void web_server_stop(WebServer* server);

#endif // WEB_SERVER_H

///////////////////////////////////////////////////////////////////////////////
