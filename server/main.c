#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "rudp.h"

void* client_thread(void* socket);

int main(int argc, char **argv)
{            
    if (argc < 5) {
        printf(
            "Usage: ./rudp-server [port] [SNW=0 GBN=1 SR=2] [window-size] [probability]\n\n");
        exit(0);
    }
    
    socket_t* server_socket;
    socket_t* client_socket;
    pthread_t client_tid;
    
    if (!(server_socket = rudp_socket(0))) {
        goto cleanup;
    }

    server_socket->options->conn->max_window_size = atoi(argv[3]);
    server_socket->options->window_type = atoi(argv[2]);

    if (rudp_bind(server_socket, "127.0.0.1", atoi(argv[1])) < 0) {
        goto cleanup;
    }    
    
    if (rudp_listen(server_socket, 100) < 0) {
        goto cleanup;
    }        

    printf(
        "[" COLOR_MAGENTA_B 
        "Waiting for connections on %s:%d" 
        COLOR_RESET "]\n",
        
        inet_ntoa(server_socket->local_addr.sin_addr),
        ntohs(server_socket->local_addr.sin_port));
    
    while ((client_socket = rudp_accept(server_socket))) {
        printf(
            "[" COLOR_MAGENTA_B 
            "Accepted a new connection from %s:%d" 
            COLOR_RESET "]\n",
            
            inet_ntoa(client_socket->remote_addr.sin_addr),
            ntohs(client_socket->remote_addr.sin_port));        
            
        pthread_create(&client_tid, 0, client_thread, client_socket);
    }    

cleanup:    
    rudp_close(server_socket, 0);    
    return 0;
}

void* client_thread(void* raw_socket)
{
    socket_t* socket = raw_socket;
    FILE *file_fd = 0;
   
    uint32_t recv_buffer_size = socket->options->conn->max_segment_size;
    uint8_t* recv_buffer = (uint8_t*) 
            calloc(recv_buffer_size, sizeof(uint8_t)); 
    
    if (rudp_recv(socket, recv_buffer, recv_buffer_size) < 0) {
        goto cleanup;
    }        
    
    printf(
    "[" COLOR_MAGENTA_B "File request: %s" COLOR_RESET "]\n", recv_buffer);     
    
    file_fd = fopen((char*) recv_buffer, "rb");        
    
    if (!file_fd) {
        printf("error");
        goto cleanup;
    }    
    
    fseek(file_fd, 0, SEEK_END);
    uint32_t file_size = ftell(file_fd);
    fseek(file_fd, 0, SEEK_SET);  
    
    uint32_t send_buffer_max_size = 
            socket->options->peer->max_segment_size - BASE_PACKET_LENGTH;
    uint8_t* send_buffer = calloc(send_buffer_max_size, sizeof(uint8_t));            
    
    if (rudp_send(socket, (uint8_t*) &file_size, sizeof(uint32_t)) < 0) {
        goto cleanup;
    }
    
    
    uint32_t sent_buffer_size = 0;
    uint32_t send_buffer_size;
    
    while(sent_buffer_size < file_size && 
            (send_buffer_size = fread(send_buffer, sizeof(uint8_t), 
            send_buffer_max_size, file_fd))) {
                
        if (rudp_send(socket, send_buffer, send_buffer_size) < 0) {           
            goto cleanup;
        }

        sent_buffer_size += send_buffer_size;
        //printf("Send: %d\n", send_buffer_size);
    } 
    
    //sleep(60);
    
    if (rudp_recv(socket, recv_buffer, recv_buffer_size) < 0) {
        goto cleanup;
    }  
    
cleanup:
    printf(
    "[" COLOR_MAGENTA_B "Closing connection %s:%d" COLOR_RESET "]\n",
    inet_ntoa(socket->remote_addr.sin_addr),
    ntohs(socket->remote_addr.sin_port)); 
    
    if (file_fd) {
        fclose(file_fd);
    }
    
    rudp_close(socket, false);
    pthread_exit(0);
}     