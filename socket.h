#include <stdint.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

#include "queue.h"

#define RUDP_SOCKET_ERROR   -1
#define RUDP_SOCKET_SUCCESS   0

typedef struct {
    uint8_t internal;
} rudp_options_t;

typedef struct {
    int32_t socket_fd;
    struct sockaddr_in local_addr;
    struct sockaddr_in remote_addr;
    
    queue_t* in_buffer;
    queue_t* out_buffer;
    
    rudp_options_t options;
} rudp_socket_t;

rudp_socket_t* rudp_socket(rudp_options_t* options);
int32_t rudp_close(rudp_socket_t* socket);

int32_t rudp_recv(rudp_socket_t* socket);
int32_t rudp_send(rudp_socket_t* socket);

int32_t rudp_connect(rudp_socket_t* socket, const char* addr, uint16_t port);

int32_t rudp_bind(rudp_socket_t* socket, const char* addr, uint16_t port);
int32_t rudp_listen(rudp_socket_t* socket, uint32_t queue_max);
rudp_socket_t* rudp_accept(rudp_socket_t* socket);
