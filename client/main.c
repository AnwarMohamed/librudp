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

    if (!rudp_connect(socket, argv[1], atoi(argv[2]))) {
        printf("rudp_connect() succeed\n");
    } else {
        printf("rudp_connect() failed\n");
        goto cleanup;        
    }
    
cleanup:    
    rudp_close(socket);
    printf("rudp_close() succeed\n");
    
    return 0;
}
