///////////////////////////////////////////////////////////////////////////////
// NAME:            bluez-iot-agent.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the program
//
// CREATED:         11/05/2021
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

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include <ev.h>

#include <web-server.h>
#include <logger.h>

static const int HTTP_PORT = 8888;

static void default_log_handler(enum LogLevel level, const char* message) {
    switch (level) {
    case INFO:
    case WARNING:
        printf("%s", message);
        break;
    case ERROR:
        fprintf(stderr, "%s", message);
        break;
    }
}

int main() {
    // Use the standard event loop
    struct ev_loop* loop = EV_DEFAULT;

    Logger logger;
    logger_initialize(&logger, default_log_handler);
    WebServer* web_server = web_server_start(&logger, HTTP_PORT);
    ev_io* web_watcher = web_server_get_watcher(web_server);
    ev_io_start(loop, web_watcher);
    ev_run(loop, 0);
    web_server_stop(&web_server);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
