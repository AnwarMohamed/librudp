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
#include "socket.h"

struct rudp_packet_header_t {
    uint8_t flags;
    uint8_t header_length;
    uint16_t checksum;
    uint32_t sequence;
    uint32_t acknowledge;    
};

struct rudp_syn_packet_header_t {
    uint8_t version;    
    uint8_t option_flags;

    uint16_t max_segment_size;
    uint16_t timeout_retransmission;
    uint16_t timeout_cum_ack;
    uint16_t timeout_trans_state;
    uint16_t timeout_null;

    uint8_t max_out_segments;
    uint8_t max_retransmissions;
    uint8_t max_cum_ack;
    uint8_t max_out_sequences;
    uint8_t max_auto_reset;

    uint32_t identifier;
};

struct rudp_syn_packet_t {
    struct rudp_packet_header_t* header;
    struct rudp_syn_packet_header_t* aux_header;
};

enum rudp_packet_type_t {
    PACKET_TYPE_SYN=0,
    PACKET_TYPE_SYN_ACK,
    PACKET_TYPE_ACK,
    PACKET_TYPE_DATA,
    PACKET_TYPE_NULL,
    PACKET_TYPE_RESET,
    PACKET_TYPE_EACK,
    PACKET_TYPE_TCS,
    PACKET_TYPE_UNKNOWN,
};

enum rudp_packet_flag_t {
    PACKET_FLAG_SYN = 1<<7,
    PACKET_FLAG_ACK = 1<<6,
    PACKET_FLAG_RST = 1<<5,
    PACKET_FLAG_NUL = 1<<4,
    PACKET_FLAG_EACK = 1<<3,
    PACKET_FLAG_TCS = 1<<2,
    PACKET_FLAG_CHK = 1<<1,
    PACKET_FLAG_DATA = 1<<0,
};

struct rudp_packet_t {
    enum rudp_packet_type_t type;
    
    void* buffer;
    int32_t buffer_size;
    
    uint64_t transmission_time;
    uint64_t creatation_time;
    
    uint16_t source_port;
    uint32_t source_addr;
    
    uint16_t destination_port;
    uint32_t destination_addr;    
};

typedef struct rudp_packet_t rudp_packet_t;
typedef struct rudp_syn_packet_t rudp_syn_packet_t;
typedef struct rudp_packet_header_t rudp_packet_header_t;
typedef struct rudp_syn_packet_header_t rudp_syn_packet_header_t;
typedef rudp_packet_header_t rudp_rst_packet_t;
typedef rudp_rst_packet_t rudp_nul_packet_t;
typedef rudp_nul_packet_t rudp_ack_packet_t;
typedef enum rudp_packet_flag_t rudp_packet_flag_t;
typedef enum rudp_packet_type_t rudp_packet_type_t;

#define PACKET_HEADER_LENGTH sizeof(rudp_packet_header_t)
#define SYN_PACKET_HEADER_LENGTH sizeof(rudp_syn_packet_header_t)
#define SYN_PACKET_LENGTH PACKET_HEADER_LENGTH + \
        SYN_PACKET_HEADER_LENGTH

rudp_packet_t* rudp_packet(rudp_packet_type_t type, 
        rudp_socket_t* socket, uint8_t* buffer, uint32_t buffer_size);

rudp_packet_t* rudp_packet_from_buffer(rudp_packet_type_t type, 
        rudp_socket_t* socket, uint8_t* buffer, uint32_t buffer_size);
        
int32_t rudp_packet_free(rudp_packet_t* packet);

int32_t rudp_packet_add_checksum(rudp_packet_t* packet);
bool rudp_packet_check_checksum(uint8_t* buffer, uint32_t buffer_size);
uint32_t rudp_packet_checksum(uint8_t* buffer, uint32_t buffer_size);

int32_t rudp_packet_set_header(rudp_packet_t* packet, uint8_t* buffer,
        uint32_t buffer_size);

int32_t rudp_packet_set_syn_header(rudp_socket_t* socket, rudp_packet_t* packet,
        uint8_t* buffer, uint32_t buffer_size);