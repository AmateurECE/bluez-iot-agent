///////////////////////////////////////////////////////////////////////////////
// NAME:            web-server.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the web server
//
// CREATED:         11/07/2021
//
// LAST EDITED:     11/15/2021
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <ev.h>
#include <microhttpd.h>

#include "web-server.h"
#include "logger.h"

// Callback for libmicrohttpd to respond to connections
static enum MHD_Result answer_to_connection(void* cls,
    struct MHD_Connection* connection, const char* url, const char* method,
    const char* version, const char* upload_data, size_t* upload_data_size,
    void** con_cls)
{
    const char* page = "<html><body>Hello, browser!</body></html>";
    struct MHD_Response* response = MHD_create_response_from_buffer(
        strlen(page), (void*)page, MHD_RESPMEM_PERSISTENT);

    int result = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return result;
}

static void web_server_dispatch(struct ev_loop* loop, ev_io* watcher,
    int events)
{
    WebServer* server = (WebServer*)watcher->data;
    fd_set read, write;
    int max_fd = server->epoll_fd;
    if (events & EV_READ) {
        FD_SET(max_fd, &read);
    }
    if (events & EV_WRITE) {
        FD_SET(max_fd, &write);
    }

    enum MHD_Result result = MHD_run_from_select(server->daemon, &read, &write,
        NULL);
    if (MHD_YES != result) {
        LOG_ERROR(server->logger, "Something bad happened in run loop");
        ev_break(loop, EVBREAK_ALL);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

WebServer* web_server_start(Logger* logger, int port) {
    WebServer* server = malloc(sizeof(WebServer));
    if (NULL == server) {
        return NULL;
    }

    server->logger = logger;
    server->daemon = MHD_start_daemon(MHD_USE_EPOLL, port, NULL, NULL,
        &answer_to_connection, NULL, MHD_OPTION_END);
    if (NULL == server->daemon) {
        free(server);
        return NULL;
    }
    LOG_INFO(logger, "Web server started successfully");

    fd_set read, write, except;
    int max_fd;
    enum MHD_Result result = MHD_get_fdset(server->daemon, &read, &write,
        &except, &max_fd);
    if (MHD_YES != result) {
        LOG_ERROR(server->logger, "Failed to get file descriptor from daemon");
    }

    // In our case (using epoll(7)), only the epoll fd is set in each set. So,
    // max_fd is the epoll fd. Then, add this fd to the watcher.
    server->epoll_fd = max_fd;
    ev_init(&server->watcher, web_server_dispatch);
    ev_io_set(&server->watcher, server->epoll_fd, EV_READ | EV_WRITE);
    server->watcher.data = server;

    return server;
}

ev_io* web_server_get_watcher(WebServer* server) {
    return &server->watcher;
}

void web_server_stop(WebServer** server) {
    if (NULL != *server) {
        LOG_INFO((*server)->logger, "Stopping web server");
        MHD_stop_daemon((*server)->daemon);
        free(*server);
        *server = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
