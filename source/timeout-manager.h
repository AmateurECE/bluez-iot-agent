///////////////////////////////////////////////////////////////////////////////
// NAME:            timeout-manager.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Interface to manage DBus timeouts
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

#ifndef TIMEOUT_MANAGER_H
#define TIMEOUT_MANAGER_H

typedef struct DBusTimeout DBusTimeout;
typedef struct Logger Logger;
struct ev_loop;
typedef unsigned int dbus_bool_t;

typedef struct TimeoutManager {
    Logger* logger;
    struct ev_loop* event_loop;

    dbus_bool_t (*AddTimeout)(DBusTimeout* timeout, void* data);
    void (*RemoveTimeout)(DBusTimeout* timeout, void* data);
} TimeoutManager;

TimeoutManager* timeout_manager_init(Logger* logger,
    struct ev_loop* event_loop);
void timeout_manager_free(TimeoutManager** manager);

#endif // TIMEOUT_MANAGER_H

///////////////////////////////////////////////////////////////////////////////
