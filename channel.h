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
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>


typedef struct rudp_socket_t rudp_socket_t;
typedef struct rudp_options_t rudp_options_t;
typedef struct rudp_packet_t rudp_packet_t;
typedef struct rudp_syn_packet_header_t rudp_syn_packet_header_t;


enum rudp_channel_timer_type_t {
    CHANNEL_TIMER_RETRANS=0,
};

typedef struct {
    enum rudp_channel_timer_type_t type;
    struct rudp_socket_t* socket;
    timer_t timer;
    struct sigevent* event;
} rudp_channel_timer_t ;

struct rudp_channel_t{
    rudp_channel_timer_t* timer_retrans;
    rudp_channel_timer_t* timer_null;
    rudp_channel_timer_t* timer_cum_ack;
    rudp_channel_timer_t* timer_trans_state;
};

#include "socket.h"
#include "packet.h"
#include "utils.h"

typedef struct rudp_channel_t rudp_channel_t;

rudp_socket_t* rudp_channel(rudp_socket_t* socket);

int32_t rudp_channel_handshake(rudp_socket_t* socket, 
        uint8_t* buffer, uint32_t buffer_size);

int32_t rudp_channel_send(rudp_socket_t* socket, 
        uint8_t* buffer, uint32_t buffer_size);

int32_t rudp_channel_send_raw(rudp_socket_t* socket, 
        uint8_t* buffer, uint32_t buffer_size);

int32_t rudp_channel_send_packet(rudp_socket_t* socket, 
        rudp_packet_t* packet);        
        
int32_t rudp_channel_recv(rudp_socket_t* socket, 
        uint8_t* buffer, uint32_t buffer_size);

int32_t rudp_channel_recv_raw(rudp_socket_t* socket, 
        uint8_t* buffer, uint32_t buffer_size);

int32_t rudp_channel_close(rudp_socket_t* socket);
int32_t rudp_channel_free(rudp_socket_t* socket);

void rudp_channel_negotiate(rudp_socket_t* socket,
        rudp_options_t* server, rudp_syn_packet_header_t* client);
        
rudp_channel_timer_t* rudp_channel_timer(rudp_socket_t* socket);
void rudp_channel_timer_close(rudp_channel_timer_t* timer);


void rudp_channel_timer_handler(rudp_channel_timer_t* timer);

void rudp_channel_deattach_node(rudp_socket_t* socket);