#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "socket.h"

#define HOST    "0.0.0.0"
#define PORT    1337
#define BACKLOG 100

rudp_socket_t* server_socket = NULL;

void handle_signal(int signal) {
    rudp_close(server_socket, true);
    exit(0);
}

int main(int argc, char **argv)
{        
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = &handle_signal;
    sa.sa_flags = SA_RESTART;
    
    sigfillset(&sa.sa_mask);
    
    if (sigaction(SIGINT, &sa, NULL) >= 0) {
        printf("sigaction() succeed\n");
    } else {
        printf("sigaction() failed\n");
        return 0;
    }    
    
    if ((server_socket = rudp_socket(NULL))) {
        printf("rudp_socket() succeed\n");
    } else {
        printf("rudp_socket() failed\n");
        return 0;
    }

    if (!rudp_bind(server_socket, HOST, PORT)) {
        printf("rudp_bind() succeed\n");
    } else {
        printf("rudp_bind() failed\n");
        goto cleanup;        
    }
    
    if (!rudp_listen(server_socket, BACKLOG)) {
        printf("rudp_listen() succeed\n");
    } else {
        printf("rudp_listen() failed\n");
        goto cleanup;        
    }
    
    rudp_socket_t* client_socket = NULL;;
        
    client_socket = rudp_accept(server_socket);
    
    if (!client_socket) {
        printf("rudp_accept() succeed\n");
    } else {
        printf("rudp_accept() failed\n");
        goto cleanup;               
    }    

cleanup:    
    rudp_close(server_socket, 0);
    printf("rudp_close() succeed\n");
    
    return 0;
}
