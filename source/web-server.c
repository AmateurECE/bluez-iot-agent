///////////////////////////////////////////////////////////////////////////////
// NAME:            web-server.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the web server interface
//
// CREATED:         11/20/2021
//
// LAST EDITED:     01/16/2022
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
#include <handlebars.h>

#include <state.h>
#include <web-server.h>

static const char* STYLESHEET_NAME = "style.css";
static const char* TEMPLATE_NAME = "index.html.hbs";

///////////////////////////////////////////////////////////////////////////////
// Private API
////

static const char* get_action_string(enum State state) {
    switch (state) {
    case STATE_CONNECTION_WAIT:
    case STATE_CONNECTED:
        return "Pairing Mode";
    default: return "";
    }
}

static HbsResult context_handler(void* key_handler_data, const char* key,
    const char** value)
{
    WebServer* web_server = (WebServer*)key_handler_data;
    if (!strcmp(key, "action")) {
        *value = get_action_string(state_get(web_server->state_publisher));
        return HBS_OK;
    } else if (!strcmp(key, "styles")) {
        *value = web_server->stylesheet;
        return HBS_OK;
    } else {
        return HBS_ERROR;
    }
}

static void post_request(SoupServer* server, SoupServerMessage* message,
    const char* path, GHashTable* query, gpointer user_data)
{
    WebServer* web_server = (WebServer*)user_data;
    const char* response = "Ok";
    soup_server_message_set_status(message, SOUP_STATUS_OK, NULL);
    soup_server_message_set_response(message, "text/html", SOUP_MEMORY_COPY,
        response, strlen(response));
    g_info("WebServer: GOING TO STATE_PAIRABLE");
    state_set(web_server->state_publisher, STATE_PAIRABLE);
}

static void get_request(SoupServer* server, SoupServerMessage* message,
    const char* path, GHashTable* query, gpointer user_data)
{
    WebServer* web_server = (WebServer*)user_data;
    HbsHandlers handlers = {
        .key_handler = context_handler,
        .key_handler_data = web_server,
    };
    HbsString* response = hbs_template_render(web_server->handlebars,
        &handlers);

    soup_server_message_set_status(message, SOUP_STATUS_OK, NULL);
    soup_server_message_set_response(message, "text/html", SOUP_MEMORY_COPY,
        response->string, response->length);

    hbs_string_free(response);
}

static void handle_connection(SoupServer* server, SoupServerMessage* message,
    const char* path, GHashTable* query, gpointer user_data)
{
    if (strcmp("/", path)) {
        soup_server_message_set_status(message, SOUP_STATUS_NOT_FOUND, NULL);
        g_info("WebServer: GET %s => 404 Not Found", path);
        return;
    }

    if (SOUP_METHOD_GET == soup_server_message_get_method(message)) {
        g_info("WebServer: GET %s => 200 Ok", path);
        get_request(server, message, path, query, user_data);
    } else {
        g_info("WebServer: POST %s => 200 Ok", path);
        post_request(server, message, path, query, user_data);
    }
}

static char* priv_read_file(WebServer* server, const char* webroot,
    const char* filename, size_t* file_length)
{
    const size_t webroot_length = strlen(webroot) + 1;
    const size_t file_path_length = webroot_length + strlen(filename) + 1;
    char* file_path = malloc(file_path_length);
    if (NULL == file_path) {
        g_error("Couldn't allocate memory for path: %s", strerror(errno));
    }

    strncpy(file_path, webroot, webroot_length);
    file_path[webroot_length - 1] = '/';
    strcpy(file_path + webroot_length, filename);

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
    size_t total_bytes_read = 0;
    while (bytes_read > 0) {
        bytes_read = read(file_fd, file_contents + total_bytes_read,
            *file_length - total_bytes_read);
        total_bytes_read += bytes_read;
    }
    if (0 > bytes_read) {
        close(file_fd);
        g_error("Error reading file: %s", strerror(errno));
    }

    close(file_fd);
    file_contents[*file_length - 1] = '\0';
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

    size_t html_length = 0;
    char* html = priv_read_file(server, webroot_path, TEMPLATE_NAME,
        &html_length);
    HbsInputContext* input_context = hbs_input_context_from_string(html);
    server->handlebars = hbs_template_load(input_context);
    hbs_input_context_free(input_context);
    if (NULL == server->handlebars) {
        free(server->stylesheet);
        free(server);
        g_error("Failed to load template!");
    }

    server->handle_connection = handle_connection;
    state_ref(state_publisher);
    server->state_publisher = state_publisher;
    return server;
}

void web_server_free(WebServer** server) {
    if (NULL != *server) {
        state_deref(&(*server)->state_publisher);
        hbs_template_free((*server)->handlebars);
        free((*server)->stylesheet);
        free(*server);
        *server = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
