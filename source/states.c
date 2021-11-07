///////////////////////////////////////////////////////////////////////////////
// NAME:            states.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of actual state behavior.
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

#include <stdio.h>
#include <unistd.h>

#include "state-machine.h"
#include "web-server.h"

int do_state_initializing(int* state __attribute__((unused)),
    void* user_data)
{
    SoundMachineState* machine = (SoundMachineState*)user_data;
    printf("Starting web server\n");
    machine->web_server = web_server_start();
    return SIGNAL_INITIALIZED;
}

// Just for now, wait for 30 seconds, then trigger a shutdown.
int do_state_connection_wait(int* state __attribute__((unused)),
    void* user_data)
{
    int timer = 0;
    while (timer++ < 30) {
        sleep(1);
    }

    return SIGNAL_SHUTDOWN;
}

int do_state_connected(int* state __attribute__((unused)), void* user_data) {
    return SIGNAL_SHUTDOWN;
}

int do_state_pairing(int* state __attribute__((unused)), void* user_data) {
    return SIGNAL_SHUTDOWN;
}

int do_state_shutdown(int* state __attribute__((unused)), void* user_data) {
    SoundMachineState* machine = (SoundMachineState*)user_data;
    printf("Shutting down\n");
    web_server_stop(&machine->web_server);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
