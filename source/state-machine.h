///////////////////////////////////////////////////////////////////////////////
// NAME:            state-machine.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Definitions for the state machine.
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

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

enum States {
    STATE_INVALID,
    STATE_INITIALIZING,
    STATE_CONNECTION_WAIT,
    STATE_CONNECTED,
    STATE_PAIRING,
    STATE_SHUTDOWN,
};

enum Events {
    SIGNAL_NONE,
    SIGNAL_INITIALIZED,
    SIGNAL_CONNECTED,
    SIGNAL_DISCONNECT,
    SIGNAL_PAIRING,
    SIGNAL_SHUTDOWN,
    // This signal doesn't get plugged in anywhere, so that any time it's
    // thrown it will cause a state machine fault.
    SIGNAL_FAULT,
};

// Currently, we don't use the value of the FlipFlop (making this a Moore
// machine).
enum FlipFlop {
    FLIPFLOP_INVALID,
};

int do_state_initializing(int*, void* user_data);
int do_state_connection_wait(int*, void* user_data);
int do_state_connected(int*, void* user_data);
int do_state_pairing(int*, void* user_data);
int do_state_shutdown(int*, void* user_data);

typedef struct WebServer WebServer;
typedef struct SoundMachineState {
    WebServer* web_server;
} SoundMachineState;

typedef struct MealyFsm MealyFsm;
typedef struct SoundStateMachine {
    MealyFsm* state_machine;
} SoundStateMachine;

SoundStateMachine* state_machine_initialize();
int state_machine_execute(SoundStateMachine* state_machine);
void state_machine_finish(SoundStateMachine** state_machine);

const char* state_to_string(enum States);

#endif // STATE_MACHINE_H

///////////////////////////////////////////////////////////////////////////////
