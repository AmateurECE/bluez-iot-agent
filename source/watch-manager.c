///////////////////////////////////////////////////////////////////////////////
// NAME:            watch-manager.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the WatchManager
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
#include <string.h>

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <ev.h>
#include <libseastar/vector.h>
#include <libseastar/error.h>

#include <logger.h>
#include <watch-manager.h>

///////////////////////////////////////////////////////////////////////////////
// Private Interface
////

struct WatchListEntry {
    DBusWatch* watch;
    int fd;
};

static void watch_handle_callback(struct ev_loop* loop, ev_io* watcher,
    int revents)
{
    struct WatchListEntry* entry = (struct WatchListEntry*)watcher->data;
    int flags = 0;
    if (revents & EV_READ) {
        flags |= DBUS_WATCH_READABLE;
    }
    if (revents & EV_WRITE) {
        flags |= DBUS_WATCH_WRITABLE;
    }
    dbus_watch_handle(entry->watch, flags);
}

static dbus_bool_t add_watch_function(DBusWatch* watch, void* user_data)
{
    WatchManager* manager = (WatchManager*)user_data;
    ev_io* watcher = malloc(sizeof(ev_io));
    if (NULL == watcher) {
        return FALSE;
    }

    struct WatchListEntry* entry = malloc(sizeof(struct WatchListEntry));
    if (NULL == entry) {
        free(watcher);
        return FALSE;
    }

    // Add the watcher to the vector.
    watcher->data = entry;
    entry->watch = watch;
    entry->fd = dbus_watch_get_unix_fd(watch);
    IndexResult result = cs_vector_push_back(manager->watch_list, watcher);
    if (!result.ok) {
        if (result.error & SEASTAR_ERRNO_SET) {
            LOG_ERROR(manager->logger,
                "Couldn't add DBusWatch to watch list: %s",
                strerror(result.error ^ SEASTAR_ERRNO_SET));
        } else {
            LOG_ERROR(manager->logger,
                "Couldn't add DBusWatch to watch list: %s",
                cs_strerror(result.error));
        }
        free(entry);
        free(watcher);
        return FALSE;
    }

    ev_init(watcher, watch_handle_callback);
    unsigned int dbus_flags = dbus_watch_get_flags(watch);
    int ev_flags = 0;
    if (dbus_flags & DBUS_WATCH_READABLE) {
        ev_flags |= EV_READ;
    }
    if (dbus_flags & DBUS_WATCH_WRITABLE) {
        ev_flags |= EV_WRITE;
    }
    ev_io_set(watcher, entry->fd, ev_flags);
    ev_io_start(manager->event_loop, watcher);
    return TRUE;
}

static void remove_watch_function(DBusWatch* watch, void* user_data)
{
    WatchManager* manager = (WatchManager*)user_data;
    ev_io* element = NULL;
    Iterator iter = cs_vector_iter(manager->watch_list);
    int watch_fd = dbus_watch_get_unix_fd(watch);
    int index = 0;
    while (NULL != (element = (ev_io*)cs_iter_next(&iter))) {
        struct WatchListEntry* entry = (struct WatchListEntry*)element->data;
        if (entry->fd == watch_fd) {
            PointerResult ret = cs_vector_remove(manager->watch_list, index);
            if (!ret.ok) {
                if (ret.error & SEASTAR_ERRNO_SET) {
                    LOG_ERROR(manager->logger,
                        "Couldn't remove watch from list: %s",
                        strerror(ret.error));
                } else {
                    LOG_ERROR(manager->logger,
                        "Couldn't remove watch from list: %s",
                        cs_strerror(ret.error));
                }
                return;
            }

            ev_io* watcher = ret.value;
            free(watcher->data);
            free(watcher);
        }
        ++index;
    }
    LOG_INFO(manager->logger,
        "D-Bus attempted to remove a watcher that wasn't in the watch list.");
}

///////////////////////////////////////////////////////////////////////////////
// Public Interface
////

WatchManager* watch_manager_init(Logger* logger, struct ev_loop* event_loop) {
    WatchManager* manager = malloc(sizeof(WatchManager));
    if (NULL == manager) {
        LOG_ERROR(logger, "Couldn't allocate memory for WatchManager: %s",
            strerror(errno));
    }

    manager->watch_list = malloc(sizeof(Vector));
    if (NULL == manager->watch_list) {
        LOG_ERROR(logger, "Couldn't allocate memory for watch list: %s",
            strerror(errno));
        free(manager);
        return NULL;
    }

    VoidResult result = cs_vector_init(manager->watch_list);
    if (!result.ok) {
        if (result.error & SEASTAR_ERRNO_SET) {
            LOG_ERROR(logger, "Couldn't initialize vector: %s",
                strerror(result.error ^ SEASTAR_ERRNO_SET));
        } else {
            LOG_ERROR(logger, "Couldn't initialize vector: %s",
                cs_strerror(result.error));
        }
        free(manager->watch_list);
        free(manager);
        return NULL;
    }

    manager->logger = logger;
    manager->event_loop = event_loop;
    manager->AddWatch = add_watch_function;
    manager->RemoveWatch = remove_watch_function;
    return manager;
}

void watch_manager_free(WatchManager** manager) {
    if (NULL != *manager) {
        cs_vector_free((*manager)->watch_list);
        free((*manager)->watch_list);
        free(*manager);
        *manager = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
