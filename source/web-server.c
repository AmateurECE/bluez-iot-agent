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

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include <libsoup/soup.h>

#include <state.h>
#include <web-server.h>

static const char* STYLESHEET_NAME = "style.css";
static const char* TEMPLATE_NAME = "index.html.hbs";

///////////////////////////////////////////////////////////////////////////////
// Private API
////

static void handle_connection(SoupServer* server, SoupServerMessage* message,
    const char* path, GHashTable* query, gpointer user_data)
{
    if (SOUP_METHOD_GET != soup_server_message_get_method(message)) {
        soup_server_message_set_status(message, SOUP_STATUS_NOT_IMPLEMENTED,
            NULL);
        return;
    }

    WebServer* web_server = (WebServer*)user_data;
    soup_server_message_set_status(message, SOUP_STATUS_OK, NULL);
    soup_server_message_set_response(message, "text/html", SOUP_MEMORY_COPY,
        web_server->html, web_server->html_length);
}

static char* priv_read_file(WebServer* server, const char* webroot,
    const char* filename, size_t* file_length)
{
    const size_t webroot_length = strlen(webroot) + 1;
    const size_t filename_length = strlen(filename) + 1;
    const size_t file_path_length = webroot_length + filename_length;
    char* file_path = malloc(file_path_length);
    if (NULL == file_path) {
        g_error("Couldn't allocate memory for path: %s", strerror(errno));
    }

    strncpy(file_path, webroot, webroot_length);
    file_path[webroot_length - 1] = '/';
    strncpy(file_path + webroot_length, filename, filename_length);

    int file_fd = open(file_path, O_RDONLY);
    if (0 > file_fd) {
        g_error("Couldn't open stylesheet file at %s: %s", file_path,
            strerror(errno));
    }
    free(file_path);

    struct stat file_stat = {0};
    if (0 != fstat(file_fd, &file_stat)) {
        close(file_fd);
        g_error("Couldn't get size of file: %s", strerror(errno));
    }

    *file_length = file_stat.st_size + 1;
    char* file_contents = malloc(*file_length);
    if (NULL == file_contents) {
        close(file_fd);
        g_error("Couldn't allocate memory: %s", strerror(errno));
    }

    ssize_t bytes_read = 1;
    while (bytes_read > 0) {
        bytes_read = read(file_fd, file_contents, *file_length);
    }
    if (0 > bytes_read) {
        close(file_fd);
        g_error("Error reading file: %s", strerror(errno));
    }

    close(file_fd);
    return file_contents;
}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

WebServer* web_server_init(const char* webroot_path,
    StatePublisher* state_publisher)
{
    WebServer* server = malloc(sizeof(WebServer));
    if (NULL == server) {
        return NULL;
    }

    server->stylesheet = priv_read_file(server, webroot_path, STYLESHEET_NAME,
        &server->stylesheet_length);
    if (NULL == server->stylesheet) {
        free(server);
        return NULL;
    }

    server->html = priv_read_file(server, webroot_path, TEMPLATE_NAME,
        &server->html_length);
    if (NULL == server->html) {
        free(server->stylesheet);
        free(server);
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
        free((*server)->html);
        free((*server)->stylesheet);
        free(*server);
        *server = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
