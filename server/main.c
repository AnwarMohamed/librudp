#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "rudp.h"

#define HOST    "127.0.0.1"
#define PORT    1337
#define BACKLOG 100


void* client_thread(void* socket);

int main(int argc, char **argv)
{        

    socket_t* server_socket;
    socket_t* client_socket;
    pthread_t client_tid;
    
    if (!(server_socket = rudp_socket(0))) {
        goto cleanup;
    }

    if (rudp_bind(server_socket, HOST, PORT) < 0) {
        goto cleanup;
    }
    
    if (rudp_listen(server_socket, BACKLOG) < 0) {
        goto cleanup;
    }        
        
    while ((client_socket = rudp_accept(server_socket))) {
        pthread_create(&client_tid, 0, client_thread, client_socket);
    }    

cleanup:    
    rudp_close(server_socket, 0);    
    return 0;
}

void* client_thread(void* socket)
{
   
   pthread_exit(0);
}     