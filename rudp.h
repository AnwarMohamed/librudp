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
#include <sys/param.h>

#include "queue.h"
#include "uthash/src/uthash.h"


#define RUDP_SOCKET_ERROR         -1
#define RUDP_SOCKET_SUCCESS        0
#define RUDP_SOCKET_ACCEPT_SLEEP   5
#define RUDP_SOCKET_SIGNAL  SIGRTMIN+2


enum socket_state_t { 
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

enum socket_type_t {
    TYPE_GENERIC = 0,
    TYPE_SERVER,
    TYPE_CLIENT,
};

struct socket_peer_options_t {
    uint8_t max_window_size;
    uint16_t max_segment_size;
};

struct socket_options_t {
    bool internal;
    struct socket_t* parent;
    
    sem_t state_lock;
    enum socket_state_t state;    
    struct socket_conn_options_t* conn; 
    struct socket_peer_options_t* peer;   
};

struct hash_node_t {
    struct sockaddr_in key;
    struct socket_t* value;
    UT_hash_handle hh;
};

struct socket_conn_options_t {
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

struct socket_t {
    enum socket_type_t type;
    
    int32_t socket_fd;
    struct sockaddr_in local_addr;
    struct sockaddr_in remote_addr;    
    
    struct hash_node_t* waiting_hash;    
    struct hash_node_t* established_hash;
    
    queue_t* ready_queue;
    
    pthread_t thread;
    
    struct socket_options_t *options; 
    struct channel_t *channel;

    uint8_t* temp_buffer;
    uint32_t temp_buffer_size;
};

struct window_t {
    queue_t* buffer;
    uint8_t size;
    uint8_t max_size;
    
    queue_node_t* head;
    queue_node_t* tail;
};

enum window_type_t {
    WINDOWS_TYPE_SNW=0,
    WINDOWS_TYPE_GBN,
    WINDOWS_TYPE_SR,
};

typedef struct window_t window_t;
typedef enum window_type_t window_type_t;

enum timer_type_t {
    TIMER_RETRANS=0,
    TIMER_TRANS_STATE,
    TIMER_CUM_ACK,
    TIMER_NULL,
};

struct utimer_t {
    enum timer_type_t type;
    struct socket_t* socket;
    struct packet_t* packet;
    timer_t timer;
    struct sigevent* event;
    struct itimerspec timer_value;
};

struct channel_t{    
    struct utimer_t* timer_null;
    struct utimer_t* timer_cum_ack;
    struct utimer_t* timer_trans_state;
    
    uint32_t acknowledge;
    uint32_t sequence;
    
    window_t* in_window;
    window_t* out_window;
};

typedef enum socket_state_t socket_state_t;
typedef struct socket_t socket_t;
typedef struct socket_options_t socket_options_t;

typedef enum timer_type_t timer_type_t;

typedef struct hash_node_t hash_node_t;
typedef struct socket_conn_options_t socket_conn_options_t;
typedef struct socket_peer_options_t socket_peer_options_t;

typedef struct socket_t socket_t;
typedef struct socket_options_t socket_options_t;
typedef struct packet_t packet_t;
typedef struct hash_node_t hash_node_t;
typedef struct packet_syn_header_t packet_syn_header_t;

typedef struct channel_t channel_t;
typedef struct utimer_t utimer_t;

struct packet_header_t {
    uint8_t flags;
    uint8_t header_length;
    uint16_t checksum;
    //uint16_t window_size;
    uint32_t sequence;
    uint32_t acknowledge;    
};

struct packet_syn_header_t {
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

struct packet_syn_t {
    struct packet_header_t* header;
    struct packet_syn_header_t* aux_header;
};

enum packet_type_t {
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

enum packet_flag_t {
    PACKET_FLAG_SYN = 1<<7,
    PACKET_FLAG_ACK = 1<<6,
    PACKET_FLAG_RST = 1<<5,
    PACKET_FLAG_NUL = 1<<4,
    PACKET_FLAG_EACK = 1<<3,
    PACKET_FLAG_TCS = 1<<2,
    PACKET_FLAG_CHK = 1<<1,
    PACKET_FLAG_DATA = 1<<0,
};

struct packet_t {
    enum packet_type_t type;
    
    uint8_t* buffer;
    int32_t buffer_size;
    
    uint64_t transmission_time;
    uint64_t creatation_time;
    
    uint16_t source_port;
    uint32_t source_addr;
    
    uint16_t destination_port;
    uint32_t destination_addr;

    struct utimer_t* timer_retrans;
    uint8_t counter_retrans;
    
    bool needs_ack;    
    struct packet_t* ack;
    
    uint8_t* data_buffer;
    uint32_t data_buffer_size;
    
    struct packet_header_t* header;
    struct packet_syn_header_t* syn_header;
    
    socket_t* socket;
};

typedef struct utimer_t utimer_t;
typedef struct packet_t packet_t;
typedef struct packet_syn_t packet_syn_t;
typedef struct packet_header_t packet_header_t;
typedef struct packet_syn_header_t packet_syn_header_t;
typedef packet_header_t packet_rst_t;
typedef packet_rst_t packet_nul_t;
typedef packet_nul_t packet_ack_t;
typedef enum packet_flag_t packet_flag_t;
typedef enum packet_type_t packet_type_t;

#define BASE_PACKET_LENGTH sizeof(packet_header_t)
#define SYN_PACKET_HEADER_LENGTH sizeof(packet_syn_header_t)
#define SYN_PACKET_LENGTH BASE_PACKET_LENGTH + \
        SYN_PACKET_HEADER_LENGTH
        


socket_t* rudp_socket(socket_options_t* options);
int32_t rudp_close(socket_t* socket, bool immediately);
int32_t rudp_connect(socket_t* socket, const char* addr, uint16_t port);
int32_t rudp_recv(socket_t* socket, uint8_t* buffer, uint32_t buffer_size);        
int32_t rudp_send(socket_t* socket, uint8_t* buffer, uint32_t buffer_size);
int32_t rudp_bind(socket_t* socket, const char* addr, uint16_t port);
int32_t rudp_listen(socket_t* socket, uint32_t queue_max);
socket_t* rudp_accept(socket_t* socket);
