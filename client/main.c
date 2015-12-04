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

/*
    if (!rudp_connect(socket, argv[1], atoi(argv[2]))) {
        printf("rudp_connect() succeed\n");
    } else {
        printf("rudp_connect() failed\n");
        goto cleanup;        
    }
    
    rudp_options_t socket_options;
    if (!rudp_options_get(socket, &socket_options)) {
        printf("rudp_options_get() succeed\n");
    } else {
        printf("rudp_options_get() failed\n");
        goto cleanup;
    }    
    
    uint8_t* send_buffer = (uint8_t*) "HELLO FROM ANWAR";
    uint32_t send_buffer_size = strlen((char*) send_buffer) + 1;
    
    if (rudp_send(socket, send_buffer, send_buffer_size) > 0) {
        printf("rudp_send() succeed\n");
    } else {
        printf("rudp_send() failed\n");
        goto cleanup;
    }
    
    uint32_t recv_buffer_size = socket_options.max_segment_size;
    uint8_t* recv_buffer = (uint8_t*) 
            calloc(recv_buffer_size, sizeof(uint8_t));
    
    if (rudp_recv(socket, recv_buffer, recv_buffer_size) >= 0) {
        printf("rudp_recv() succeed: %d:%s\n", 
                recv_buffer_size, recv_buffer);
    } else {
        printf("rudp_recv() failed\n");
        goto cleanup;
    }    
*/

cleanup:    
    rudp_close(socket, 0);
    printf("rudp_close() succeed\n");
    
    return 0;
}
