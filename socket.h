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
#include "queue.h"

#define RUDP_SOCKET_ERROR          -1
#define RUDP_SOCKET_SUCCESS        0
#define RUDP_SOCKET_ACCEPT_SLEEP   5

typedef enum { 
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
} rudp_state_t;

typedef struct {
    uint8_t internal;
    int32_t parent_socket_fd;
    rudp_state_t state;

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
} rudp_options_t;

typedef struct {
    int32_t socket_fd;
    struct sockaddr_in local_addr;
    struct sockaddr_in remote_addr;
    
    queue_t* in_buffer;
    queue_t* out_buffer;
    
    queue_t* syn_queue;
    queue_t* accept_queue;
    
    pthread_t listen_thread;
    
    rudp_options_t options;
} rudp_socket_t;

rudp_socket_t* rudp_socket(rudp_options_t* options);
int32_t rudp_close(rudp_socket_t* socket, uint8_t immediately);

int32_t rudp_recv(rudp_socket_t* socket);
int32_t rudp_send(rudp_socket_t* socket);

int32_t rudp_connect(rudp_socket_t* socket, const char* addr, uint16_t port);

int32_t rudp_bind(rudp_socket_t* socket, const char* addr, uint16_t port);

int32_t rudp_listen(rudp_socket_t* socket, uint32_t queue_max);
void* rudp_listen_handler(void* socket);

rudp_socket_t* rudp_accept(rudp_socket_t* socket);

rudp_state_t rudp_state(rudp_socket_t* socket);

void rudp_recv_handler(rudp_socket_t* socket, 
        uint8_t* buffer, uint32_t buffer_size);