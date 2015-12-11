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


rudp_socket_t* rudp_socket(rudp_options_t* options);
rudp_socket_t* rudp_linux_socket(rudp_socket_t* socket);

int32_t rudp_close(rudp_socket_t* socket, bool immediately);

int32_t rudp_recv(rudp_socket_t* socket,
        uint8_t* buffer, uint32_t buffer_size);
        
int32_t rudp_send(rudp_socket_t* socket, 
        uint8_t* buffer, uint32_t buffer_size);

int32_t rudp_connect(rudp_socket_t* socket, const char* addr, uint16_t port);
void* rudp_connect_handler(void* socket);

int32_t rudp_bind(rudp_socket_t* socket, const char* addr, uint16_t port);

int32_t rudp_listen(rudp_socket_t* socket, uint32_t queue_max);
void* rudp_listen_handler(void* socket);

rudp_socket_t* rudp_accept(rudp_socket_t* socket);

rudp_state_t rudp_state(rudp_socket_t* socket);

int32_t rudp_recv_handler(rudp_socket_t* socket, 
        uint8_t* buffer, uint32_t buffer_size);

rudp_options_t* rudp_options();
int32_t rudp_options_get(rudp_socket_t* socket, rudp_options_t* options);
void rudp_options_free(rudp_options_t* options);
