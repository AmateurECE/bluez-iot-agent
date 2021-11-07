///////////////////////////////////////////////////////////////////////////////
// NAME:            state-machine.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the state machine.
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
#include <stdlib.h>
#include <string.h>

#include <fsautomata/mealy.h>

#include "state-machine.h"

static const StateTransition initializing_tt[] = {
    { SIGNAL_INITIALIZED, 0, STATE_CONNECTION_WAIT },
    { SIGNAL_SHUTDOWN, 0, STATE_SHUTDOWN },
    { /* sentinel */ },
};

static const StateTransition connection_wait_tt[] = {
    { SIGNAL_CONNECTED, 0, STATE_CONNECTED },
    { SIGNAL_PAIRING, 0, STATE_PAIRING },
    { SIGNAL_SHUTDOWN, 0, STATE_SHUTDOWN },
    { /* sentinel */ },
};

static const StateTransition connected_tt[] = {
    { SIGNAL_PAIRING, 0, STATE_PAIRING },
    { SIGNAL_DISCONNECT, 0, STATE_CONNECTION_WAIT },
    { SIGNAL_SHUTDOWN, 0, STATE_SHUTDOWN },
    { /* sentinel */ },
};

static const StateTransition pairing_tt[] = {
    { SIGNAL_CONNECTED, 0, STATE_CONNECTED },
    // TODO: Should be a way to go from PAIRING back to CONNECTION_WAIT
    { SIGNAL_SHUTDOWN, 0, STATE_SHUTDOWN },
    { /* sentinel */ },
};

static const StateTransition final_tt[] = {
    { /* sentinel */ },
};

static const MealyState state_table[] = {
    [STATE_INITIALIZING]={
        STATE_INITIALIZING, NULL, NULL, do_state_initializing, initializing_tt
    },
    [STATE_CONNECTION_WAIT]={
        STATE_CONNECTION_WAIT, NULL, NULL, do_state_connection_wait,
        connection_wait_tt
    },
    [STATE_CONNECTED]={
        STATE_CONNECTED, NULL, NULL, do_state_connected, connected_tt
    },
    [STATE_PAIRING]={
        STATE_PAIRING, NULL, NULL, do_state_pairing, pairing_tt
    },
    [STATE_SHUTDOWN]={
        STATE_SHUTDOWN, NULL, NULL, do_state_shutdown, final_tt
    },
    { /* sentinel */ },
};

static const int final_states[] = {
    STATE_SHUTDOWN,
    0, // sentinel
};

SoundStateMachine* state_machine_initialize() {
    SoundStateMachine* state_machine = malloc(sizeof(SoundStateMachine));
    if (NULL == state_machine) {
        return NULL;
    }

    SoundMachineState* user_data = malloc(sizeof(SoundMachineState));
    if (NULL == user_data) {
        free(state_machine);
        return NULL;
    }

    MealyFsm mealy_machine = {
        .states = state_table,
        .initial_state = STATE_INITIALIZING,
        .final_states = final_states,
        .user_data = user_data,
    };
    mealy_fsm_initialize(&mealy_machine);

    void* malloc_machine = malloc(sizeof(MealyFsm));
    if (NULL == malloc_machine) {
        free(state_machine);
        free(user_data);
        return NULL;
    }
    memcpy(malloc_machine, &mealy_machine, sizeof(MealyFsm));

    state_machine->state_machine = malloc_machine;
    return state_machine;
}

int state_machine_execute(SoundStateMachine* machine) {
    enum FsmEvent fsm_event = FSM_EVENT_NONE;
    MealyFsm* state_machine = machine->state_machine;
    while (1) {
        fsm_event = mealy_fsm_poll(state_machine);
        switch (fsm_event) {
        case FSM_EVENT_NONE:
        case FSM_EVENT_STATE_CHANGE:
            break;
        case FSM_EVENT_ACCEPTED:
            return 0;
        case FSM_EVENT_FAULT:
            fprintf(stderr, "FSM Fault: %s\n",
                fsm_strerror(state_machine->fault));
            return state_machine->fault;
        }
    }
}

void state_machine_finish(SoundStateMachine** state_machine) {
    if (NULL != *state_machine) {
        free(*state_machine);
        *state_machine = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
