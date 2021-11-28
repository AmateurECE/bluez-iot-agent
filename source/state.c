///////////////////////////////////////////////////////////////////////////////
// NAME:            state.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     State-related behavior
//
// CREATED:         11/27/2021
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

#include <stdlib.h>
#include <string.h>

#include <state.h>

typedef struct StateObserver {
    void* user_data;
    void (*onExit)(enum State, void* user_data);
    void (*onEntry)(enum State, void* user_data);
} StateObserver;

static const int PENDING_ENTRY = 1 << 0;
static const int PENDING_EXIT  = 1 << 1;

static const size_t MAXIMUM_OBSERVERS = 8;
typedef struct StatePublisher {
    int ref_count;
    enum State current_state;
    int pending_change;
    size_t num_observers;
    StateObserver* observers;
} StatePublisher;

///////////////////////////////////////////////////////////////////////////////
// Public API
////

StatePublisher* state_init() {
    StatePublisher* publisher = malloc(sizeof(StatePublisher));
    if (NULL == publisher) {
        return NULL;
    }

    memset(publisher, 0, sizeof(StatePublisher));
    publisher->observers = calloc(MAXIMUM_OBSERVERS, sizeof(StateObserver));
    if (NULL == publisher->observers) {
        free(publisher);
        return NULL;
    }

    memset(publisher->observers, 0, sizeof(StateObserver));
    return publisher;
}

void state_ref(StatePublisher* publisher) {
    ++publisher->ref_count;
}

void state_deref(StatePublisher** publisher) {
    if (NULL == *publisher) {
        return;
    }

    if ((*publisher)->ref_count > 0) {
        --(*publisher)->ref_count;
        *publisher = NULL;
        return;
    }

    free((*publisher)->observers);
    free(*publisher);
    *publisher = NULL;
}

int state_add_observer(StatePublisher* publisher,
    void (*onExit)(enum State, void* user_data),
    void (*onEntry)(enum State, void* user_data), void* user_data)
{
    if (publisher->num_observers >= MAXIMUM_OBSERVERS) {
        return 1;
    }

    publisher->observers[publisher->num_observers].user_data = user_data;
    publisher->observers[publisher->num_observers].onExit = onExit;
    publisher->observers[publisher->num_observers].onEntry = onEntry;
    ++publisher->num_observers;
    return 0;
}

void state_set(StatePublisher* publisher, enum State state) {
    publisher->current_state = state;
    publisher->pending_change = PENDING_ENTRY | PENDING_EXIT;
}

enum State state_get(StatePublisher* publisher) {
    return publisher->current_state;
}

void state_do_exit(StatePublisher* publisher) {
    if (!(publisher->pending_change & PENDING_EXIT)) {
        return;
    }

    publisher->pending_change ^= PENDING_EXIT;
    for (size_t i = 0; i < publisher->num_observers; ++i) {
        publisher->observers[i].onExit(publisher->current_state,
            publisher->observers[i].user_data);
    }
}

void state_do_entry(StatePublisher* publisher) {
    if (!(publisher->pending_change & PENDING_ENTRY)) {
        return;
    }

    publisher->pending_change ^= PENDING_ENTRY;
    for (size_t i = 0; i < publisher->num_observers; ++i) {
        publisher->observers[i].onEntry(publisher->current_state,
            publisher->observers[i].user_data);
    }
}

///////////////////////////////////////////////////////////////////////////////
