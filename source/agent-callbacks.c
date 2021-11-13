///////////////////////////////////////////////////////////////////////////////
// NAME:            agent-callbacks.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Callbacks used with the AgentServer
//
// CREATED:         11/12/2021
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

#include <stdio.h>

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>

#include "agent-callbacks.h"

dbus_bool_t agent_add_watch_function(DBusWatch* watch __attribute__((unused)),
    void* data __attribute__((unused)))
{
    return TRUE;
}

void agent_watch_toggled_function(DBusWatch* watch __attribute__((unused)),
    void* data __attribute__((unused)))
{}

void agent_remove_watch_function(DBusWatch* watch __attribute__((unused)),
    void* data __attribute__((unused)))
{}

dbus_bool_t agent_add_timeout_function(
    DBusTimeout* timeout __attribute__((unused)),
    void* data __attribute__((unused)))
{
    return TRUE;
}

void agent_remove_timeout_function(
    DBusTimeout* timeout __attribute__((unused)),
    void* data __attribute__((unused)))
{}

void agent_timeout_toggled_function(
    DBusTimeout* timeout __attribute__((unused)),
    void* data __attribute__((unused)))
{}

void agent_object_path_unregister_function(
    DBusConnection* connection __attribute__((unused)),
    void* user_data __attribute__((unused)))
{}

DBusHandlerResult agent_object_path_message_function(
    DBusConnection* connection __attribute__((unused)),
    DBusMessage* message __attribute__((unused)),
    void* user_data __attribute__((unused)))
{
    printf("Tested!\n");
    return DBUS_HANDLER_RESULT_HANDLED;
}

///////////////////////////////////////////////////////////////////////////////
