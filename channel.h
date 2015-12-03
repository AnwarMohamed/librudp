/*
*
*  Copyright (C) 2015  Anwar Mohamed <anwarelmakrahy[at]gmail.com>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
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
#include "socket.h"

rudp_socket_t* rudp_channel_new(rudp_socket_t* socket);

int32_t rudp_channel_send(rudp_socket_t* socket, 
        uint8_t* buffer, uint32_t buffer_size);
        
int32_t rudp_channel_recv(rudp_socket_t* socket, 
        uint8_t* buffer, uint32_t buffer_size);

int32_t rudp_channel_close(rudp_socket_t* socket);
int32_t rudp_channel_free(rudp_socket_t* socket);