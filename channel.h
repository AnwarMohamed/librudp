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
#include "socket.h"
#include "packet.h"
#include "window.h"


socket_t* channel(socket_t* socket);

int32_t channel_send(socket_t* socket, 
        uint8_t* buffer, uint32_t buffer_size);

int32_t channel_send_raw(socket_t* socket, 
        uint8_t* buffer, uint32_t buffer_size);

int32_t channel_send_packet(packet_t* packet);        
        
int32_t channel_recv(socket_t* socket, 
        uint8_t* buffer, uint32_t buffer_size);

int32_t channel_recv_raw(socket_t* socket, 
        uint8_t* buffer, uint32_t buffer_size);

int32_t channel_free(socket_t* socket);


void channel_timer_handler(utimer_t* utimer);

void channel_deattach_node(socket_t* socket);

hash_node_t* channel_established(socket_t* socket);
hash_node_t* channel_waiting(socket_t* socket);

void channel_retransmit(socket_t* socket);

int32_t channel_recv_eack(
        socket_t* socket, packet_t* packet);
int32_t channel_recv_reset_ack(
        socket_t* socket, packet_t* packet);
int32_t channel_recv_reset(
        socket_t* socket, packet_t* packet);
int32_t channel_recv_data(
        socket_t* socket, packet_t* packet);
int32_t channel_recv_null(
        socket_t* socket, packet_t* packet);
int32_t channel_recv_syn(
        socket_t* socket, packet_t* packet);
int32_t channel_recv_syn_ack(
        socket_t* socket, packet_t* packet);
int32_t channel_recv_ack(
        socket_t* socket, packet_t* packet);
int32_t channel_recv_tcs(
        socket_t* socket, packet_t* packet);
int32_t channel_recv_tcs_ack(
        socket_t* socket, packet_t* packet);
        
int32_t channel_handshake_start(socket_t* socket);
