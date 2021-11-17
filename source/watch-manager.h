///////////////////////////////////////////////////////////////////////////////
// NAME:            watch-manager.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Interface to manage DBusWatch instances with libev
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

#ifndef WATCH_MANAGER_H
#define WATCH_MANAGER_H

// Forward declarations
typedef struct Logger Logger;
struct ev_loop;
typedef unsigned int dbus_bool_t;

typedef struct WatchManager {
    Logger* logger;
    struct ev_loop* event_loop;
    dbus_bool_t (*AddWatch)(DBusWatch* watch, void* user_data);
    void (*RemoveWatch)(DBusWatch* watch, void* user_data);
} WatchManager;

WatchManager* watch_manager_init(Logger* logger, struct ev_loop* event_loop);
void watch_manager_free(WatchManager** manager);

#endif // WATCH_MANAGER_H

///////////////////////////////////////////////////////////////////////////////
