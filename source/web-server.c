///////////////////////////////////////////////////////////////////////////////
// NAME:            web-server.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the web server
//
// CREATED:         11/07/2021
//
// LAST EDITED:     11/11/2021
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

#include <microhttpd.h>

#include "web-server.h"
#include "logger.h"

static enum MHD_Result answer_to_connection(void* cls __attribute__((unused)),
    struct MHD_Connection* connection, const char* url __attribute__((unused)),
    const char* method __attribute__((unused)),
    const char* version __attribute__((unused)),
    const char* upload_data __attribute__((unused)),
    size_t* upload_data_size __attribute__((unused)),
    void** con_cls __attribute__((unused)))
{
    const char* page = "<html><body>Hello, browser!</body></html>";
    struct MHD_Response* response = MHD_create_response_from_buffer(
        strlen(page), (void*)page, MHD_RESPMEM_PERSISTENT);

    int result = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return result;
}

WebServer* web_server_start(Logger* logger) {
    WebServer* server = malloc(sizeof(WebServer));
    if (NULL == server) {
        return NULL;
    }

    server->logger = logger;
    server->daemon = MHD_start_daemon(MHD_USE_EPOLL, HTTP_PORT, NULL, NULL,
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
    // max_fd is the epoll fd.
    server->epoll_fd = max_fd;

    return server;
}

int web_server_get_epoll_fd(WebServer* server) {
    return server->epoll_fd;
}

int web_server_dispatch(WebServer* server, uint32_t events) {
    fd_set read, write, except;
    int max_fd = server->epoll_fd;
    if (events & EPOLLIN) {
        FD_SET(max_fd, &read);
    }
    if (events & EPOLLOUT) {
        FD_SET(max_fd, &write);
    }
    if (events & (EPOLLRDHUP | EPOLLPRI | EPOLLERR | EPOLLHUP)) {
        FD_SET(max_fd, &except);
    }

    enum MHD_Result result = MHD_run_from_select(server->daemon, &read, &write,
        &except);
    if (MHD_YES != result) {
        LOG_ERROR(server->logger, "Something bad happened in run loop");
        return 1;
    }
    return 0;
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
