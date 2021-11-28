///////////////////////////////////////////////////////////////////////////////
// NAME:            bluez-client.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Client for interacting with BlueZ server
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

#ifndef BLUEZ_CLIENT_H
#define BLUEZ_CLIENT_H

typedef struct BluezClient BluezClient;
typedef struct StatePublisher StatePublisher;
typedef struct _GDBusConnection GDBusConnection;

BluezClient* bluez_client_init(StatePublisher* state_publisher,
    GDBusConnection* connection);
void bluez_client_setup_agent(BluezClient* bluez_client,
    const char* object_path, const char* capability);
void bluez_client_free(BluezClient** client);

#endif // BLUEZ_CLIENT_H

///////////////////////////////////////////////////////////////////////////////
