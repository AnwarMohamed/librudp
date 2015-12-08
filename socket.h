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
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <event.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/ioctl.h>

#include "queue.h"
#include "utils.h"
#include "uthash/src/uthash.h"

#define RUDP_SOCKET_ERROR          -1
#define RUDP_SOCKET_SUCCESS        0
#define RUDP_SOCKET_ACCEPT_SLEEP   5
#define RUDP_SOCKET_SIGNAL  (SIGRTMAX+SIGRTMIN)/2


enum rudp_state_t { 
    STATE_LISTEN = 0,
    STATE_SYN_SENT,
    STATE_SYN_RECEIVED,
    STATE_ESTABLISHED,
    STATE_RST_WAIT_1,
    STATE_RST_WAIT_2,
    STATE_CLOSE_WAIT,
    STATE_CLOSING,
    STATE_LAST_ACK,
    STATE_TIME_WAIT,
    STATE_CLOSED,
} ;

typedef struct rudp_syn_packet_header_t rudp_conn_options_t;

struct rudp_options_t {
    bool internal;
    struct rudp_socket_t* parent;
    
    sem_t state_lock;
    enum rudp_state_t state;    
    rudp_conn_options_t* conn;
    
    /*
    uint8_t version;    
    uint8_t flags;

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
    */ 
};

struct rudp_hash_node_t {
    struct sockaddr_in key;
    struct rudp_socket_t* value;
    UT_hash_handle hh;
};

struct rudp_socket_t {
    int32_t socket_fd;
    struct sockaddr_in local_addr;
    struct sockaddr_in remote_addr;
    
    queue_t* in_buffer;
    queue_t* out_buffer;
    
    struct rudp_hash_node_t* waiting_hash;    
    struct rudp_hash_node_t* established_hash;
    
    queue_t* ready_queue;
    
    pthread_t thread;
    
    struct rudp_options_t options; 
    struct rudp_channel_t *channel;

    uint8_t* temp_buffer;
    uint32_t temp_buffer_size;
};

#include "channel.h"

typedef struct rudp_syn_packet_header_t rudp_conn_options_t;
typedef enum rudp_state_t rudp_state_t;
typedef struct rudp_options_t rudp_options_t;
typedef struct rudp_socket_t rudp_socket_t;
typedef struct rudp_hash_node_t rudp_hash_node_t;

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
