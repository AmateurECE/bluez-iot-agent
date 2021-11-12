///////////////////////////////////////////////////////////////////////////////
// NAME:            agent-server.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     This server connects to D-Bus and configures bluez to
//                  route pairing and authorization requests to itself.
//
// CREATED:         11/07/2021
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

#ifndef AGENT_SERVER_H
#define AGENT_SERVER_H

typedef struct DBusError DBusError;
typedef struct DBusConnection DBusConnection;
typedef struct Logger Logger;
typedef struct AgentServer {
    DBusError* error;
    DBusConnection* connection;
    Logger* logger;
} AgentServer;

AgentServer* agent_server_start(Logger* logger);
void agent_server_stop(AgentServer** server);

#endif // AGENT_SERVER_H

///////////////////////////////////////////////////////////////////////////////
