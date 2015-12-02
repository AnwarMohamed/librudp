#include <stdio.h>

#include "socket.h"

int main(int argc, char **argv)
{
    rudp_socket_t* socket = NULL;
    
    if ((socket = rudp_socket(NULL))) {
        printf("rudp_socket() succeed\n");
    } else {
        printf("rudp_socket() failed\n");
        return 0;
    }

    if (!rudp_bind(socket, argv[1], atoi(argv[2]))) {
        printf("rudp_bind() succeed\n");
    } else {
        printf("rudp_bind() failed\n");
        goto cleanup;        
    }
    
    if (!rudp_listen(socket, atoi(argv[3]))) {
        printf("rudp_listen() succeed\n");
    } else {
        printf("rudp_listen() failed\n");
        goto cleanup;        
    }
    
    rudp_socket_t* client;
    while(1) {
        if (!(client = rudp_accept(socket))) {
            printf("rudp_accept() succeed\n");
        } else {
            printf("rudp_accept() failed\n");
            goto cleanup;               
        }
    }
    
cleanup:    
    rudp_close(socket);
    printf("rudp_close() succeed\n");
    
    return 0;
}
