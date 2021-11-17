///////////////////////////////////////////////////////////////////////////////
// NAME:            bluez-proxy.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     D-Bus interface to the BlueZ Daemon
//
// CREATED:         11/11/2021
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

#ifndef BLUEZ_PROXY_H
#define BLUEZ_PROXY_H

// Forward declarations
typedef struct DBusConnection DBusConnection;
typedef struct DBusError DBusError;
typedef struct Logger Logger;
typedef struct BluezProxy BluezProxy;

typedef struct BluezProxy {
    // Public interface
    int (*RegisterAgent)(BluezProxy* proxy, const char* object_path,
        const char* capability);
    int (*RequestDefaultAgent)(BluezProxy* proxy, const char* object_path);

    // "Private" data
    DBusConnection* connection;
    DBusError* error;
    Logger* logger;
} BluezProxy;

BluezProxy* bluez_proxy_init(Logger* logger, DBusConnection* connection,
    DBusError* error);
DBusError* bluez_proxy_get_error(BluezProxy* proxy);
void bluez_proxy_free(BluezProxy** proxy);

#endif // BLUEZ_PROXY_H

///////////////////////////////////////////////////////////////////////////////
