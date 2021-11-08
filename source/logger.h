///////////////////////////////////////////////////////////////////////////////
// NAME:            logger.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logging interface for the application
//
// CREATED:         11/07/2021
//
// LAST EDITED:     11/08/2021
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

#ifndef LOGGER_H
#define LOGGER_H

#include <stddef.h>

enum LogLevel {
    INFO,
    WARNING,
    ERROR,
};

static const size_t MAX_MESSAGE_SIZE = 256;

// Logging is probably the one place I'll let myself get a little crazy with
// macros. This variadic macro must have a format/argument signature.
#define LOG_MESSAGE(level, logger, message, ...) {                      \
        char error_log_message[MAX_MESSAGE_SIZE];                       \
        snprintf(error_log_message, MAX_MESSAGE_SIZE, message "\n"      \
            __VA_OPT__(,) __VA_ARGS__);                                 \
        logger->handler(level, error_log_message);                      \
    }

#define LOG_ERROR(logger, message, ...) {                               \
        LOG_MESSAGE(ERROR, logger, "%s:%d: " message, __FUNCTION__,     \
            __LINE__ __VA_OPT__(,) __VA_ARGS__);                        \
    }

#define LOG_INFO(logger, message, ...) {                        \
        LOG_MESSAGE(INFO, logger, message, __VA_ARGS__);        \
    }

typedef void UserLogHandler(enum LogLevel level, const char* message);
typedef struct Logger {
    UserLogHandler* handler;
} Logger;

void logger_initialize(Logger* logger, UserLogHandler* handler);

#endif // LOGGER_H

///////////////////////////////////////////////////////////////////////////////
