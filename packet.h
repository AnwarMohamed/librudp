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


rudp_packet_t* rudp_packet(rudp_packet_type_t type, rudp_socket_t* socket);

rudp_packet_t* rudp_packet_from_buffer(rudp_packet_type_t type, 
        rudp_socket_t* socket, uint8_t* buffer, uint32_t buffer_size);
        
int32_t rudp_packet_free(rudp_packet_t* packet);

int32_t rudp_packet_add_checksum(rudp_packet_t* packet);
bool rudp_packet_check_checksum(uint8_t* buffer, uint32_t buffer_size);
uint16_t rudp_packet_checksum(uint8_t* buffer, uint32_t buffer_size);

int32_t rudp_packet_set_header(rudp_socket_t* socket, rudp_packet_t* packet);

int32_t rudp_packet_set_syn_header(rudp_socket_t* socket, 
        rudp_packet_t* packet);
        
rudp_packet_type_t rudp_packet_check_type(uint8_t* buffer, 
        uint32_t buffer_size);


void rudp_packet_timeout(rudp_channel_timer_t* timer, uint32_t interval);

rudp_packet_t* rudp_buffered_packet(rudp_socket_t* socket, 
        uint8_t* buffer, uint32_t buffer_size);