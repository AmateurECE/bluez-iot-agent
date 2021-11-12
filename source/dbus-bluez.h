///////////////////////////////////////////////////////////////////////////////
// NAME:            dbus-bluez.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     D-Bus interface to the BlueZ Daemon
//
// CREATED:         11/11/2021
//
// LAST EDITED:     11/11/2021
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

#ifndef DBUS_BLUEZ_H
#define DBUS_BLUEZ_H

#include "agent-server.h"

int bluez_register_agent(AgentServer* server);
int bluez_make_default_agent(AgentServer* server);

#endif // DBUS_BLUEZ_H

///////////////////////////////////////////////////////////////////////////////
