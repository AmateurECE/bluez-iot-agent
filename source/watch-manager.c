///////////////////////////////////////////////////////////////////////////////
// NAME:            watch-manager.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the WatchManager
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>

#include <logger.h>
#include <watch-manager.h>

///////////////////////////////////////////////////////////////////////////////
// Private Interface
////

static dbus_bool_t add_watch_function(DBusWatch* watch, void* user_data)
{ return FALSE; }

static void remove_watch_function(DBusWatch* watch, void* user_data)
{}

static void watch_toggled_function(DBusWatch* watch, void* user_data)
{}

///////////////////////////////////////////////////////////////////////////////
// Public Interface
////

WatchManager* watch_manager_init(Logger* logger, DBusConnection* connection,
    DBusError* error, struct ev_loop* event_loop)
{
    WatchManager* manager = malloc(sizeof(WatchManager));
    if (NULL == manager) {
        LOG_ERROR(logger, "Couldn't allocate memory for WatchManager: %s",
            strerror(errno));
    }

    manager->logger = logger;
    manager->connection = connection;
    manager->error = error;
    manager->event_loop = event_loop;

    if (!dbus_connection_set_watch_functions(connection,
            add_watch_function, remove_watch_function,
            watch_toggled_function, manager, NULL)) {
        LOG_ERROR(logger, "out of memory or callback failure");
        free(manager);
        return NULL;
    }

    return manager;
}

void watch_manager_free(WatchManager** manager) {
    if (NULL != *manager) {
        free(*manager);
        *manager = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
