
#include <stdio.h>
#include "rudp.h"

int main(int argc, char **argv)
{
    if (argc < 6) {
        printf(
            "Usage: ./rudp-client [host] [port] [filename] [SNW=0 GBN=1 SR=2] [window-size]\n\n");
        exit(0);
    }
    
    socket_t* socket = 0;    
    FILE *file_fd = fopen(argv[3], "wb");
    
    if (!file_fd) {        
        goto cleanup;
    }
    
    if (!(socket = rudp_socket(0))) {
        goto cleanup;
    }

    socket->options->conn->max_window_size = atoi(argv[5]);
    socket->options->window_type = atoi(argv[4]);

    if (rudp_connect(socket, argv[1], atoi(argv[2])) < 0) {            
        goto cleanup;    
    }             
    
    //sleep(60);
    
    printf("[" COLOR_MAGENTA_B "Connected to %s:%d" COLOR_RESET "]\n",        
            argv[1], atoi(argv[2]));                      
            
    printf("[" COLOR_MAGENTA_B "Requesting file: %s" COLOR_RESET "]\n", argv[3]);
    
    if (rudp_send(socket, (uint8_t*) argv[3], strlen(argv[3])+1) < 0) {        
        goto cleanup;
    }        
    
    uint32_t file_size;
    
    if (rudp_recv(socket, (uint8_t*) &file_size, sizeof(uint32_t)) < 0) {
        goto cleanup;
    }
    
    printf("[" COLOR_MAGENTA_B "Filesize: %d" COLOR_RESET "]\n", file_size);
    
    
    uint32_t recv_buffer_max_size = socket->options->conn->max_segment_size;
    uint8_t* recv_buffer = (uint8_t*) 
            calloc(recv_buffer_max_size, sizeof(uint8_t)); 

    uint32_t recvd_buffer_size = 0;
    uint32_t recv_buffer_size;
    
    while(recvd_buffer_size < file_size) {
        if ((recv_buffer_size = rudp_recv(socket, 
                recv_buffer, recv_buffer_max_size)) < 0) {
            goto cleanup;
        }
        
        fwrite(recv_buffer, sizeof(uint8_t), recv_buffer_size, file_fd);
        
        recvd_buffer_size += recv_buffer_size;
        //printf("Recieved: %d\n", recv_buffer_size);
    }     
  
  
  
    if (rudp_send(socket, (uint8_t*) "DONE", strlen("DONE")+1) < 0) {        
        goto cleanup;
    }  
    
    
    
cleanup:        
    if (file_fd) {
        fclose(file_fd);
    }
    
    if (socket) {
        rudp_close(socket, false);    
    }    
    return 0;
}
