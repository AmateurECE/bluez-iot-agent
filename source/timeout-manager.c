///////////////////////////////////////////////////////////////////////////////
// NAME:            timeout-manager.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the TimeoutManager interface
//
// CREATED:         11/17/2021
//
// LAST EDITED:     11/17/2021
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

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>

#include <logger.h>
#include <timeout-manager.h>

///////////////////////////////////////////////////////////////////////////////
// Private Interface
////

static dbus_bool_t add_timeout_function(DBusTimeout* timeout, void* data)
{ return TRUE; }

static void remove_timeout_function(DBusTimeout* timeout, void* data)
{}

///////////////////////////////////////////////////////////////////////////////
// Public Interface
////

TimeoutManager* timeout_manager_init(Logger* logger,
    struct ev_loop* event_loop)
{
    TimeoutManager* manager = malloc(sizeof(TimeoutManager));
    if (NULL == manager) {
        return NULL;
    }

    manager->logger = logger;
    manager->event_loop = event_loop;
    manager->AddTimeout = add_timeout_function;
    manager->RemoveTimeout = remove_timeout_function;
    return manager;
}

void timeout_manager_free(TimeoutManager** manager)
{
    if (NULL != *manager) {
        // TODO: Some way to unset timeout functions in the connection here?
        free(*manager);
        *manager = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
