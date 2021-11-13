///////////////////////////////////////////////////////////////////////////////
// NAME:            states.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of actual state behavior.
//
// CREATED:         11/07/2021
//
// LAST EDITED:     11/12/2021
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
#include <string.h>
#include <sys/epoll.h>
#include <time.h>
#include <unistd.h>

#include "agent-server.h"
#include "state-machine.h"
#include "web-server.h"

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

int do_state_initializing(int* state, void* user_data) {
    SoundMachineState* machine = (SoundMachineState*)user_data;
    memset(machine, 0, sizeof(SoundMachineState));

    logger_initialize(&machine->logger, default_log_handler);
    machine->web_server = web_server_start(&machine->logger);
    if (NULL == machine->web_server) {
        return SIGNAL_SHUTDOWN;
    }

    machine->agent_server = agent_server_start(&machine->logger);
    if (NULL == machine->agent_server) {
        return SIGNAL_SHUTDOWN;
    }

    return SIGNAL_INITIALIZED;
}

// Just for now, wait for 30 seconds, then trigger a shutdown.
int do_state_connection_wait(int* state, void* user_data) {
    SoundMachineState* machine = (SoundMachineState*)user_data;
    static const int MAX_EVENTS = 10;
    int epoll_fd = epoll_create1(0);
    if (-1 == epoll_fd) {
        LOG_ERROR(&machine->logger, "Could not open an epoll fd: %s",
            strerror(errno));
        return SIGNAL_SHUTDOWN;
    }

    struct epoll_event event = {0};
    event.events = EPOLLIN;
    int web_fd = web_server_get_epoll_fd(machine->web_server);
    event.data.fd = web_fd;
    if (-1 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD, web_fd, &event)) {
        LOG_ERROR(&machine->logger, "Couldn't register web server socket: %s",
            strerror(errno));
        return SIGNAL_SHUTDOWN;
    }

    // Main loop
    struct epoll_event events[MAX_EVENTS];
    while (1) {
        int number_of_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (-1 == number_of_fds) {
            LOG_ERROR(&machine->logger, "In epoll_wait: %s", strerror(errno));
        }

        for (int i = 0; i < number_of_fds; ++i) {
            if (events[i].data.fd == web_fd) {
                int result = web_server_dispatch(machine->web_server,
                    events[i].events);
                if (0 != result) {
                    return SIGNAL_SHUTDOWN;
                }
            }
        }
    }

    return SIGNAL_SHUTDOWN;
}

int do_state_connected(int* state, void* user_data) {
    return SIGNAL_SHUTDOWN;
}

int do_state_pairing(int* state, void* user_data) {
    return SIGNAL_SHUTDOWN;
}

int do_state_shutdown(int* state, void* user_data) {
    SoundMachineState* machine = (SoundMachineState*)user_data;
    printf("Shutting down\n");
    web_server_stop(&machine->web_server);
    agent_server_stop(&machine->agent_server);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
