/*
*
*  Copyright (C) 2015  Anwar Mohamed <anwarelmakrahy[at]gmail.com>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to Anwar Mohamed
*  anwarelmakrahy[at]gmail.com
*
*/

#pragma once
#include "rudp.h"
#include "utils.h"
#include "channel.h"
#include "packet.h"


socket_t* socket_linux(socket_t* socket);

void* socket_connect_handler(void* socket);
void* socket_listen_handler(void* socket);

socket_state_t socket_state(socket_t* socket);

int32_t socket_recv_handler(socket_t* socket, 
        uint8_t* buffer, uint32_t buffer_size);

socket_options_t* socket_options();
int32_t socket_options_get(socket_t* socket, socket_options_t* options);
void socket_options_free(socket_options_t* options);

void socket_timer_handler(union sigval sigval);