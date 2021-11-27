///////////////////////////////////////////////////////////////////////////////
// NAME:            agent-server.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Handle messages from BlueZ about connected devices.
//
// CREATED:         11/20/2021
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

#ifndef AGENT_SERVER_H
#define AGENT_SERVER_H

#include <stdint.h>

typedef char object_path;
typedef char gchar;
typedef struct StatePublisher StatePublisher;
typedef struct _IotAgentAgent1 IotAgentAgent1;
typedef struct _GDBusMethodInvocation GDBusMethodInvocation;

typedef struct AgentServer {
    // Returns an owning string
    gchar* (*RequestPinCode)(IotAgentAgent1* interface,
        GDBusMethodInvocation* invocation, const object_path* device);
    void (*DisplayPinCode)(IotAgentAgent1* interface,
        GDBusMethodInvocation* invocation, const object_path* device,
        const gchar* pincode);
    uint32_t (*RequestPasskey)(IotAgentAgent1* interface,
        GDBusMethodInvocation* invocation, const object_path* device);
    void (*DisplayPasskey)(IotAgentAgent1* interface,
        GDBusMethodInvocation* invocation, const object_path* device,
        uint32_t passkey, uint16_t entered);
    void (*RequestConfirmation)(IotAgentAgent1* interface,
        GDBusMethodInvocation* invocation, const object_path* device,
        uint32_t passkey);
    void (*RequestAuthorization)(IotAgentAgent1* interface,
        GDBusMethodInvocation* invocation, const object_path* device);
    void (*AuthorizeService)(IotAgentAgent1* interface,
        GDBusMethodInvocation* invocation, const object_path* path,
        const gchar* uuid);
    void (*Cancel)(IotAgentAgent1* interface,
        GDBusMethodInvocation* invocation);

    StatePublisher* state_publisher;
} AgentServer;

AgentServer* agent_server_init(StatePublisher* publisher);
void agent_server_free(AgentServer**);

#endif // AGENT_SERVER_H

///////////////////////////////////////////////////////////////////////////////
