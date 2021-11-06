///////////////////////////////////////////////////////////////////////////////
// NAME:            bluez-soundsystem-agent.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the program
//
// CREATED:         11/05/2021
//
// LAST EDITED:     11/06/2021
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

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <stdio.h>
#include <string.h>

static const int PORT = 8888;

enum MHD_Result answer_to_connection(void* cls,
    struct MHD_Connection* connection, const char* url, const char* method,
    const char* version, const char* upload_data, size_t* upload_data_size,
    void** con_cls)
{
    const char* page = "<html><body>Hello, browser!</body></html>";
    struct MHD_Response* response = MHD_create_response_from_buffer(
        strlen(page), (void*)page, MHD_RESPMEM_PERSISTENT);

    int result = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return result;
}

int main() {
    struct MHD_Daemon* daemon = MHD_start_daemon(
        MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL,
        &answer_to_connection, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        return 1;
    }

    getchar();
    MHD_stop_daemon(daemon);
}

///////////////////////////////////////////////////////////////////////////////
