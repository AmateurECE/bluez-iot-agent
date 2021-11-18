///////////////////////////////////////////////////////////////////////////////
// NAME:            timeout-manager.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the TimeoutManager interface
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

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <ev.h>
#include <libseastar/vector.h>
#include <libseastar/error.h>

#include <logger.h>
#include <timeout-manager.h>

///////////////////////////////////////////////////////////////////////////////
// Private Interface
////

static void timeout_handle_callback(struct ev_loop* loop, ev_timer* watcher,
    int revents)
{
    DBusTimeout* timeout = (DBusTimeout*)watcher->data;
    dbus_timeout_handle(timeout);
}

static dbus_bool_t add_timeout_function(DBusTimeout* timeout, void* data) {
    TimeoutManager* manager = (TimeoutManager*)data;
    ev_timer* watcher = malloc(sizeof(ev_timer));
    if (NULL == watcher) {
        return FALSE;
    }

    int* timeout_id = malloc(sizeof(int));
    if (NULL == timeout_id) {
        LOG_ERROR(manager->logger, "Couldn't allocate memory for timer id: %s",
            strerror(errno));
        free(watcher);
        return FALSE;
    }

    // Add the watcher to the vector.
    watcher->data = timeout;
    *timeout_id = manager->timeout_id++;
    IndexResult result = cs_vector_push_back(manager->timeout_list, watcher);
    if (!result.ok) {
        if (result.error & SEASTAR_ERRNO_SET) {
            LOG_ERROR(manager->logger,
                "Couldn't add DBusTimeout to timeout list: %s",
                strerror(result.error ^ SEASTAR_ERRNO_SET));
        } else {
            LOG_ERROR(manager->logger,
                "Couldn't add DBusTimeout to timeout list: %s",
                cs_strerror(result.error));
        }
        free(timeout_id);
        free(watcher);
        return FALSE;
    }

    dbus_timeout_set_data(timeout, timeout_id, NULL);
    ev_init(watcher, timeout_handle_callback);
    int dbus_interval = dbus_timeout_get_interval(timeout);
    ev_timer_set(watcher, dbus_interval, dbus_interval);
    ev_timer_start(manager->event_loop, watcher);
    return TRUE;
}

static void remove_timeout_function(DBusTimeout* timeout, void* data)
{
    TimeoutManager* manager = (TimeoutManager*)data;
    ev_timer* element = NULL;
    Iterator iter = cs_vector_iter(manager->timeout_list);
    int* timeout_id = dbus_timeout_get_data(timeout);
    int index = 0;
    while (NULL != (element = (ev_timer*)cs_iter_next(&iter))) {
        DBusTimeout* entry = (DBusTimeout*)element->data;
        int* entry_id = (int*)dbus_timeout_get_data(entry);
        if (*entry_id == *timeout_id) {
            PointerResult ret = cs_vector_remove(manager->timeout_list, index);
            if (!ret.ok) {
                if (ret.error & SEASTAR_ERRNO_SET) {
                    LOG_ERROR(manager->logger,
                        "Couldn't remove timeout from list: %s",
                        strerror(ret.error));
                } else {
                    LOG_ERROR(manager->logger,
                        "Couldn't remove timeout from list: %s",
                        cs_strerror(ret.error));
                }
                return;
            }

            free(entry_id);
            free(element);
            return;
        }
        ++index;
    }
    LOG_INFO(manager->logger,
        "D-Bus attempted to remove a timeout that wasn't in the list.");
}

///////////////////////////////////////////////////////////////////////////////
// Public Interface
////

TimeoutManager* timeout_manager_init(Logger* logger,
    struct ev_loop* event_loop)
{
    TimeoutManager* manager = malloc(sizeof(TimeoutManager));
    if (NULL == manager) {
        return NULL;
    }

    manager->timeout_list = malloc(sizeof(Vector));
    if (NULL == manager->timeout_list) {
        LOG_ERROR(logger, "Couldn't allocate memory for watch list: %s",
            strerror(errno));
        free(manager);
        return NULL;
    }

    VoidResult result = cs_vector_init(manager->timeout_list);
    if (!result.ok) {
        if (result.error & SEASTAR_ERRNO_SET) {
            LOG_ERROR(logger, "Couldn't initialize vector: %s",
                strerror(result.error ^ SEASTAR_ERRNO_SET));
        } else {
            LOG_ERROR(logger, "Couldn't initialize vector: %s",
                cs_strerror(result.error));
        }
        free(manager->timeout_list);
        free(manager);
        return NULL;
    }

    manager->timeout_id = 0;
    manager->logger = logger;
    manager->event_loop = event_loop;
    manager->AddTimeout = add_timeout_function;
    manager->RemoveTimeout = remove_timeout_function;
    return manager;
}

void timeout_manager_free(TimeoutManager** manager)
{
    if (NULL != *manager) {
        cs_vector_free((*manager)->timeout_list);
        free((*manager)->timeout_list);
        free(*manager);
        *manager = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
