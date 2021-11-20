///////////////////////////////////////////////////////////////////////////////
// NAME:            bluez-iot-agent.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the application
//
// CREATED:         11/20/2021
//
// LAST EDITED:     11/20/2021
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

#include <argp.h>
#include <stdbool.h>

#include <glib.h>

#include <config.h>

const char* argp_program_name = PROGRAM_NAME " " PROGRAM_VERSION;
const char* argp_program_bug_address = "<ethan.twardy@gmail.com>";

static char doc[] = "Modern pairing wizard for Bluetooth IoT devices on Linux";

static error_t parse_opt(int, char*, struct argp_state*);
static const struct argp_option options[] = {
    { "no-register-name", 'n', NULL, OPTION_ARG_OPTIONAL,
      "Don't attempt to register the service name with D-Bus", 0 },
    { 0 },
};
static struct argp argp = { options, parse_opt, NULL, doc, NULL, NULL, NULL };

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
    bool* register_name = state->input;
    switch (key) {
    case 'n':
        *register_name = false;
        break;
    case ARGP_KEY_END:
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

int main(int argc, char** argv) {
    bool register_name = true;
    argp_parse(&argp, argc, argv, 0, 0, &register_name);

    GMainLoop* main_loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(main_loop);
}

///////////////////////////////////////////////////////////////////////////////
