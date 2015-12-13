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

packet_t* packet(packet_type_t type, socket_t* socket);
        
int32_t packet_free(packet_t* packet);

int32_t packet_checksum_add(packet_t* packet);
bool packet_checksum_check(uint8_t* buffer, uint32_t buffer_size);
uint16_t packet_checksum(uint8_t* buffer, uint32_t buffer_size);

int32_t packet_header_set(socket_t* socket, packet_t* packet);

int32_t packet_syn_header_set(socket_t* socket, 
        packet_t* packet);
        
packet_type_t packet_type_check(uint8_t* buffer, 
        uint32_t buffer_size);

packet_t* packet_buffered(socket_t* socket, 
        uint8_t* buffer, uint32_t buffer_size);
        
int32_t packet_data_set(packet_t* packet, 
        uint8_t* buffer, uint32_t buffer_size);
        
void packet_timeout(utimer_t* timer);
void packet_print(packet_t* packet);        