#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "rudp.h"

#define HOST    "127.0.0.1"
#define PORT    1337
#define BACKLOG 100

socket_t* server_socket = NULL;

void handle_signal(int signal) {
    //rudp_close(server_socket, true);
    exit(0);
}

int main(int argc, char **argv)
{        
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = &handle_signal;
    sa.sa_flags = SA_RESTART;
    
    sigfillset(&sa.sa_mask);
    
    if (sigaction(SIGINT, &sa, 0) < 0) {
        goto cleanup;
    }
    
    if (!(server_socket = rudp_socket(0))) {
        goto cleanup;
    }

    if (rudp_bind(server_socket, HOST, PORT) < 0) {
        goto cleanup;
    }
    
    if (rudp_listen(server_socket, BACKLOG) < 0) {
        goto cleanup;
    }
/*    
    rudp_socket_t* client_socket = NULL;;
        
    client_socket = rudp_accept(server_socket);
    
    if (!client_socket) {
        //printf("rudp_accept() succeed\n");
    } else {
        //printf("rudp_accept() failed\n");
        goto cleanup;               
    }    
*/
cleanup:    
    rudp_close(server_socket, 0);    
    return 0;
}
