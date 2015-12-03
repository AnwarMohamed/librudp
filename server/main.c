#include <stdio.h>
#include <signal.h>
#include "socket.h"

#define HOST    "0.0.0.0"
#define PORT    1337
#define BACKLOG 100


int main(int argc, char **argv)
{
    rudp_socket_t* server_socket = NULL;
    
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
    
    while(1) {
        client_socket = rudp_accept(server_socket);
        
        if (!client_socket) {
            printf("rudp_accept() succeed\n");
        } else {
            printf("rudp_accept() failed\n");
            goto cleanup;               
        }
    }
    
cleanup:    
    rudp_close(server_socket, 0);
    printf("rudp_close() succeed\n");
    
    return 0;
}
