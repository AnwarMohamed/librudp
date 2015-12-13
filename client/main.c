
#include <stdio.h>
#include "rudp.h"

#define HOST    "127.0.0.1"
#define PORT    1337

int main(int argc, char **argv)
{
    socket_t* socket = 0;
    
    if (!(socket = rudp_socket(0))) {
        goto cleanup;
    }

    if (rudp_connect(socket, HOST, PORT) < 0) {
        goto cleanup;
    }  

 /*   
    rudp_options_t socket_options;
    if (!rudp_options_get(socket, &socket_options)) {
        //printf("rudp_options_get() succeed\n");
    } else {
        //printf("rudp_options_get() failed\n");
        goto cleanup;
    }    
    
    uint8_t* send_buffer = (uint8_t*) "HELLO FROM ANWAR";
    uint32_t send_buffer_size = strlen((char*) send_buffer) + 1;
    
    if (rudp_send(socket, send_buffer, send_buffer_size) > 0) {
        //printf("rudp_send() succeed\n");
    } else {
        //printf("rudp_send() failed\n");
        goto cleanup;
    }
    
    uint32_t recv_buffer_size = socket_options.conn->max_segment_size;
    uint8_t* recv_buffer = (uint8_t*) 
            calloc(recv_buffer_size, sizeof(uint8_t));
    
    if (rudp_recv(socket, recv_buffer, recv_buffer_size) >= 0) {
        //printf("rudp_recv() succeed: %d:%s\n", 
        //        recv_buffer_size, recv_buffer);
    } else {
        //printf("rudp_recv() failed\n");
        goto cleanup;
    }    
*/
cleanup:    
    rudp_close(socket, false);
    printf("rudp_close() succeed\n");
    
    return 0;
}
