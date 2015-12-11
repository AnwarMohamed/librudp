#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <semaphore.h>
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
#include <signal.h>

#include "queue.h"
#include "uthash/src/uthash.h"


#define RUDP_SOCKET_ERROR         -1
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
};

enum rudp_type_t {
    TYPE_GENERIC = 0,
    TYPE_SERVER,
    TYPE_CLIENT,
};

struct rudp_peer_options_t {
    uint8_t max_window_size;
    uint16_t max_segment_size;
};

struct rudp_options_t {
    bool internal;
    struct rudp_socket_t* parent;
    
    sem_t state_lock;
    enum rudp_state_t state;    
    struct rudp_conn_options_t* conn; 
    struct rudp_peer_options_t* peer;   
};

struct rudp_hash_node_t {
    struct sockaddr_in key;
    struct rudp_socket_t* value;
    UT_hash_handle hh;
};

struct rudp_conn_options_t {
    uint8_t version;    
    uint8_t option_flags;
    
    uint16_t timeout_retransmission;
    uint16_t timeout_cum_ack;
    uint16_t timeout_trans_state;
    uint16_t timeout_null;

    uint16_t max_segment_size;
    uint8_t max_window_size;
    uint8_t max_retransmissions;
    uint8_t max_cum_ack;
    uint8_t max_out_sequences;
    uint8_t max_auto_reset;

    uint32_t identifier;
};

struct rudp_socket_t {
    enum rudp_type_t type;
    
    int32_t socket_fd;
    struct sockaddr_in local_addr;
    struct sockaddr_in remote_addr;
    
    queue_t* in_buffer;
    queue_t* out_buffer;
    
    struct rudp_hash_node_t* waiting_hash;    
    struct rudp_hash_node_t* established_hash;
    
    queue_t* ready_queue;
    
    pthread_t thread;
    
    struct rudp_options_t *options; 
    struct rudp_channel_t *channel;

    uint8_t* temp_buffer;
    uint32_t temp_buffer_size;
};


enum rudp_channel_timer_type_t {
    CHANNEL_TIMER_RETRANS=0,
};

struct rudp_channel_timer_t {
    enum rudp_channel_timer_type_t type;
    struct rudp_socket_t* socket;
    timer_t timer;
    struct sigevent* event;
    struct itimerspec timer_value;
};

struct rudp_channel_t{    
    struct rudp_channel_timer_t* timer_null;
    struct rudp_channel_timer_t* timer_cum_ack;
    struct rudp_channel_timer_t* timer_trans_state;
    
    uint32_t acknowledge;
    uint32_t sequence;
};

typedef enum rudp_state_t rudp_state_t;
typedef struct rudp_socket_t rudp_socket_t;
typedef struct rudp_options_t rudp_options_t;

typedef struct rudp_hash_node_t rudp_hash_node_t;
typedef struct rudp_conn_options_t rudp_conn_options_t;
typedef struct rudp_peer_options_t rudp_peer_options_t;

typedef struct rudp_socket_t rudp_socket_t;
typedef struct rudp_options_t rudp_options_t;
typedef struct rudp_packet_t rudp_packet_t;
typedef struct rudp_hash_node_t rudp_hash_node_t;
typedef struct rudp_syn_packet_header_t rudp_syn_packet_header_t;

typedef struct rudp_channel_t rudp_channel_t;
typedef struct rudp_channel_timer_t rudp_channel_timer_t;

struct rudp_packet_header_t {
    uint8_t flags;
    uint8_t header_length;
    uint16_t checksum;
    uint16_t window_size;
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

    uint8_t max_window_size;
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
    PACKET_TYPE_RESET_ACK,    
    PACKET_TYPE_EACK,
    PACKET_TYPE_TCS,
    PACKET_TYPE_TCS_ACK,
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
    
    uint8_t* buffer;
    int32_t buffer_size;
    
    uint64_t transmission_time;
    uint64_t creatation_time;
    
    uint16_t source_port;
    uint32_t source_addr;
    
    uint16_t destination_port;
    uint32_t destination_addr;

    struct rudp_channel_timer_t* timer_retrans;
    uint8_t counter_retrans;
    
    bool needs_ack;    
    struct rudp_packet_t* ack;
    
    uint8_t* data_buffer;
    uint32_t data_buffer_size;
};

typedef struct rudp_channel_timer_t rudp_channel_timer_t;
typedef struct rudp_packet_t rudp_packet_t;
typedef struct rudp_syn_packet_t rudp_syn_packet_t;
typedef struct rudp_packet_header_t rudp_packet_header_t;
typedef struct rudp_syn_packet_header_t rudp_syn_packet_header_t;
typedef rudp_packet_header_t rudp_rst_packet_t;
typedef rudp_rst_packet_t rudp_nul_packet_t;
typedef rudp_nul_packet_t rudp_ack_packet_t;
typedef enum rudp_packet_flag_t rudp_packet_flag_t;
typedef enum rudp_packet_type_t rudp_packet_type_t;

#define BASE_PACKET_LENGTH sizeof(rudp_packet_header_t)
#define SYN_PACKET_HEADER_LENGTH sizeof(rudp_syn_packet_header_t)
#define SYN_PACKET_LENGTH BASE_PACKET_LENGTH + \
        SYN_PACKET_HEADER_LENGTH
