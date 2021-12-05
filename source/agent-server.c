///////////////////////////////////////////////////////////////////////////////
// NAME:            agent-server.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of agent-server  handlers
//
// CREATED:         11/20/2021
//
// LAST EDITED:     12/04/2021
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
#include <stdlib.h>

#include <agent-server.h>
#include <bluez-agent.h>
#include <state.h>

///////////////////////////////////////////////////////////////////////////////
// Private API
////

static char* request_pin_code(IotAgentAgent1* interface,
    GDBusMethodInvocation* invocation, const object_path* device)
{ return NULL; }

static void display_pin_code(IotAgentAgent1* interface,
    GDBusMethodInvocation* invocation, const object_path* device,
    const gchar* pincode)
{}

static uint32_t request_passkey(IotAgentAgent1* interface,
    GDBusMethodInvocation* invocation, const object_path* device)
{ return 0; }

static void display_passkey(IotAgentAgent1* interface,
    GDBusMethodInvocation* invocation, const object_path* device,
    uint32_t passkey, uint16_t entered)
{}

static void request_confirmation(IotAgentAgent1* interface,
    GDBusMethodInvocation* invocation, const object_path* device,
    uint32_t passkey)
{}

static void request_authorization(IotAgentAgent1* interface,
    GDBusMethodInvocation* invocation, const object_path* device)
{}

static void authorize_service(IotAgentAgent1* interface,
    GDBusMethodInvocation* invocation, const object_path* path,
    const gchar* uuid)
{
    g_info("%s called", __FUNCTION__);
    iot_agent_agent1_complete_authorize_service(interface, invocation);
}

static void cancel(IotAgentAgent1* interface,
    GDBusMethodInvocation* invocation)
{
    g_info("%s called", __FUNCTION__);
    iot_agent_agent1_complete_cancel(interface, invocation);
}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

AgentServer* agent_server_init(StatePublisher* state_publisher) {
    AgentServer* server = malloc(sizeof(AgentServer));
    if (NULL == server) {
        return NULL;
    }

    server->RequestPinCode = request_pin_code;
    server->DisplayPinCode = display_pin_code;
    server->RequestPasskey = request_passkey;
    server->DisplayPasskey = display_passkey;
    server->RequestConfirmation = request_confirmation;
    server->RequestAuthorization = request_authorization;
    server->AuthorizeService = authorize_service;
    server->Cancel = cancel;

    state_ref(state_publisher);
    server->state_publisher = state_publisher;
    return server;
}

void agent_server_free(AgentServer** server) {
    if (NULL != *server) {
        state_deref(&(*server)->state_publisher);
        free(*server);
        *server = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
