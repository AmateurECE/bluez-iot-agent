///////////////////////////////////////////////////////////////////////////////
// NAME:            bluez-soundsystem-agent.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the program
//
// CREATED:         11/05/2021
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


#include <errno.h>
#include <stdio.h>

#include "state-machine.h"

int main(int argc, char** argv) {
    SoundStateMachine* state_machine = state_machine_initialize();

    int result = state_machine_execute(state_machine);
    state_machine_finish(&state_machine);
    return result;
}

///////////////////////////////////////////////////////////////////////////////
