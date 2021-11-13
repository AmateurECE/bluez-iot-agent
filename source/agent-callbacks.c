///////////////////////////////////////////////////////////////////////////////
// NAME:            agent-callbacks.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Callbacks used with the AgentServer
//
// CREATED:         11/12/2021
//
// LAST EDITED:     11/12/2021
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
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>

#include "agent-callbacks.h"
#include "agent-server.h"
#include "logger.h"

///////////////////////////////////////////////////////////////////////////////
// Watch Callbacks
////

dbus_bool_t agent_add_watch_function(DBusWatch* watch, void* user_data) {
    AgentServer* server = (AgentServer*)user_data;
    int watch_fd = dbus_watch_get_unix_fd(watch);

    struct epoll_event event = {0};
    event.data.fd = watch_fd;

    unsigned int flags = dbus_watch_get_flags(watch);
    if (flags & DBUS_WATCH_READABLE) {
        event.events = EPOLLIN;
    }
    if (flags & DBUS_WATCH_WRITABLE) {
        event.events = EPOLLOUT;
    }
    if (-1 == epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, watch_fd, &event)) {
        LOG_ERROR(server->logger, "Couldn't register watch socket: %s",
            strerror(errno));
        return FALSE;
    }
    return TRUE;
}

void agent_watch_toggled_function(DBusWatch* watch, void* user_data)
{
    if (dbus_watch_get_enabled(watch)) {
        agent_add_watch_function(watch, user_data);
    } else {
        agent_remove_watch_function(watch, user_data);
    }
}

void agent_remove_watch_function(DBusWatch* watch, void* user_data)
{
    AgentServer* server = (AgentServer*)user_data;
    if (-1 == epoll_ctl(server->epoll_fd, EPOLL_CTL_DEL,
            dbus_watch_get_unix_fd(watch), NULL)) {
        LOG_ERROR(server->logger, "Couldn't deregister watch socket: %s",
            strerror(errno));
    }
}

///////////////////////////////////////////////////////////////////////////////
// Timeout Callbacks
////

dbus_bool_t agent_add_timeout_function(DBusTimeout* timeout, void* user_data) {
    return TRUE;
}

void agent_remove_timeout_function(DBusTimeout* timeout, void* user_data)
{}

void agent_timeout_toggled_function(DBusTimeout* timeout, void* user_data)
{}

///////////////////////////////////////////////////////////////////////////////
// Object Path Callbacks
////

void agent_object_path_unregister_function(DBusConnection* connection,
    void* user_data)
{}

DBusHandlerResult agent_object_path_message_function(
    DBusConnection* connection, DBusMessage* message, void* user_data)
{
    printf("Tested!\n");
    return DBUS_HANDLER_RESULT_HANDLED;
}

///////////////////////////////////////////////////////////////////////////////
