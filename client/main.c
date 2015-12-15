
#include <stdio.h>
#include "rudp.h"

#define HOST    "127.0.0.1"
#define PORT    1337
#define FILENAME "./librudp.so"

int main(int argc, char **argv)
{
    socket_t* socket = 0;    
    FILE *file_fd = fopen(FILENAME, "rb");
    
    if (!file_fd) {
        goto cleanup;
    }
    
    fseek(file_fd, 0, SEEK_END);
    uint32_t file_size = ftell(file_fd);
    fseek(file_fd, 0, SEEK_SET);        
    
    if (!(socket = rudp_socket(0))) {
        goto cleanup;
    }

    if (rudp_connect(socket, HOST, PORT) < 0) {    
        goto cleanup;
    }     
    
    printf("Connected\n");
            
    uint32_t send_buffer_max_size = 
        socket->options->peer->max_segment_size - BASE_PACKET_LENGTH;
        
    send_buffer_max_size *= 10;
    
    uint8_t* send_buffer = calloc(send_buffer_max_size, sizeof(uint8_t));
    snprintf((char*) send_buffer, socket->options->peer->max_segment_size, 
            "PUT:%s:%d", FILENAME, file_size);
    
    uint32_t send_buffer_size = strlen((char*) send_buffer) + 1;
    
    if (rudp_send(socket, send_buffer, /*send_buffer_size*/ send_buffer_max_size) < 0) {        
        goto cleanup;
    }        
    
    uint32_t sent_buffer_size = 0;
    
    //while(sent_buffer_size < file_size && 
    //        (send_buffer_size = fread(send_buffer, sizeof(uint8_t), 
    //        send_buffer_max_size, file_fd))) {
    //            
    //    if (rudp_send(socket, send_buffer, send_buffer_size) < 0) {           
    //        goto cleanup;
    //    }

    //    sent_buffer_size += send_buffer_size;
        //printf("Send: %d\n", send_buffer_size);
    //}    
    
    uint32_t recv_buffer_size = socket->options->conn->max_segment_size;
    uint8_t* recv_buffer = (uint8_t*) 
            calloc(recv_buffer_size, sizeof(uint8_t));
    
    //if (rudp_recv(socket, recv_buffer, recv_buffer_size) >= 0) {        
    //    printf("%s\n", recv_buffer_size, recv_buffer);
    //} else {    
    //    //sleep(50)    ;
    //    printf("error\n");        
    //    goto cleanup;
    //}    

    sleep(600);
cleanup:    
    if (socket) {
        rudp_close(socket, false);    
    }
    
    if (file_fd) {
        fclose(file_fd);
    }
    
    return 0;
}
