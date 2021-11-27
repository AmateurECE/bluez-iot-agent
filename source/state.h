///////////////////////////////////////////////////////////////////////////////
// NAME:            state.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic pertaining to application state
//
// CREATED:         11/26/2021
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

#ifndef STATE_H
#define STATE_H

enum State {
    STATE_NONE,
    STATE_SHUTDOWN,
};

typedef struct StatePublisher StatePublisher;

StatePublisher* state_init();
void state_ref(StatePublisher* publisher);
void state_deref(StatePublisher** publisher);
int state_add_observer(StatePublisher* publisher,
    void (*onExit)(enum State, void* user_data),
    void (*onEntry)(enum State, void* user_data), void* user_data);
void state_set(StatePublisher* publisher, enum State);
enum State state_get(StatePublisher* publisher);
void state_do_exit(StatePublisher* publisher);
void state_do_entry(StatePublisher* publisher);

#endif // STATE_H

///////////////////////////////////////////////////////////////////////////////
