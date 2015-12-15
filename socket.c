#include "socket.h"

void socket_timer_handler(
        union sigval sigval) 
{    
    channel_timer_handler(sigval.sival_ptr);    
}

socket_t* rudp_socket(
        socket_options_t* options)
{
    debug_print("rudp_socket()\n");
    
    socket_t* new_socket = 
            (socket_t*) calloc (1, sizeof(socket_t));
    
    if (!new_socket) { 
        goto failed; 
    }
    
    if (options) {
        new_socket->options = options;
    } else {              
        new_socket->options = socket_options();                
    }
    
    if (!new_socket->options) {
        goto failed;
    }
    
    if (!new_socket->options->internal) {        
        if (!(new_socket = socket_linux(new_socket))) {
            goto failed;         
        }
    }
    
    if (!(new_socket->ready_queue = queue())) {
        goto failed;                
    }
    
    /*
    struct sigaction* sa = (struct sigaction*) calloc(1, sizeof(struct sigaction));
    sigemptyset(&sa->sa_mask);
    sa->sa_flags = SA_SIGINFO;
    sa->sa_sigaction = rudp_timer_handler;
    
    if(sigaction(RUDP_SOCKET_SIGNAL, sa, 0) < 0) {
        goto failed;            
    }
     */

    success_print("rudp_socket() succeed\n");
    return new_socket;
    
failed:
    error_print("rudp_socket() failed\n");
    rudp_close(new_socket, false);
    return 0;
}

socket_options_t* socket_options()
{
    socket_options_t* options = 
            (socket_options_t*) calloc(1, sizeof(socket_options_t));
    
    options->conn = (socket_conn_options_t*) 
            calloc(1, sizeof(socket_conn_options_t));                
    
    options->conn->version = 1;    
    options->conn->identifier = rudp_random();
    
    options->conn->max_segment_size = 1024;
    options->conn->max_retransmissions = 2;    
    options->conn->max_cum_ack = 3;
    options->conn->max_out_sequences = 3;
    options->conn->max_auto_reset = 3;
    
    options->conn->timeout_retransmission = 1000;
    options->conn->timeout_cum_ack = 300;
    options->conn->timeout_null = 2000;
    options->conn->timeout_trans_state = 1000;
    
    options->conn->max_window_size = 32;
    options->window_type = WINDOW_TYPE_SR;
    
    options->state = STATE_CLOSED;
    sem_init(&options->state_lock, 0, 0);
    
    options->peer = (socket_peer_options_t*) 
            calloc(1, sizeof(socket_peer_options_t));
        
    return options;
}

socket_t* socket_linux(
        socket_t* new_socket)
{
    debug_print("socket_linux()\n");
    
    int32_t flags, reuseaddr_on = 1;        
    new_socket->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
        
    if (new_socket->socket_fd < 0) {
        goto failed;        
    }
            
    if (setsockopt(new_socket->socket_fd, SOL_SOCKET, SO_REUSEADDR, 
            (const void *)&reuseaddr_on , sizeof(int32_t)) < 0) {
        goto failed;        
    }
    
    if ((flags = fcntl(new_socket->socket_fd, F_GETFL)) < 0) {
        goto failed;    
    }
    
    success_print("socket_linux() succeed\n");
    return new_socket;
    
failed:
    error_print("socket_linux() failed\n");
    rudp_close(new_socket, true);
    return 0;    
}

void socket_options_free(
        socket_options_t* options)
{
    if (options) {            
        if (options->conn) {
            free(options->conn);
            options->conn = 0;                
        }
            
        if (options->peer) {
            free(options->peer); 
            options->peer = 0;
        }
        
        free(options);
    } 
}

void socket_queue_free(queue_t* queue) {
    if (queue) {
        
        queue_node_t* node;
        while (queue_size(queue)) {
            node = queue_dequeue(queue, false);
            
            packet_free(node->data);            
            free(node);
        }
        
        queue_free(queue, true);
    }
}

void socket_hash_free(hash_node_t* hash) {
    if (hash) {
        hash_node_t *current_node, *temp_node;
        HASH_ITER(hh, hash, current_node, temp_node) {
            HASH_DEL(hash, current_node);
            rudp_close(current_node->value, false);
            free(current_node);
        }
    }      
}

int32_t rudp_close(
        socket_t* socket,
        bool immediately) 
{
    debug_print("rudp_close()\n");
    
    if (socket) {                                   

        if (socket->thread) {
            //if (immediately)
                pthread_kill(socket->thread, SIGINT);
                socket->thread = 0;
            //else
            //    pthread_join(socket->thread, 0);
        }
        
        if (socket->options) {
            if (!socket->options->internal) {
                if (socket->socket_fd) {
                    close(socket->socket_fd);  
                    socket->socket_fd = 0;
                }
            }
            
            socket->options->state = STATE_CLOSED;
 
            sem_destroy(&socket->options->state_lock);
            memset(&socket->options->state_lock, 0, sizeof(sem_t));
        
            socket_options_free(socket->options);  
            socket->options = 0;
        }                                    
                
        if (socket->ready_queue) {
            socket_queue_free(socket->ready_queue);
            socket->ready_queue = 0;
        }

        //socket_hash_free(socket->established_hash);
        //socket_hash_free(socket->waiting_hash);            
        
        if (socket->channel) {
            channel_free(socket);
            socket->channel = 0;
        }
        
        if (socket->temp_buffer) {
            free(socket->temp_buffer);
            socket->temp_buffer = 0;
            socket->temp_buffer_size = 0;
        }
        
        free(socket);
    }
    
    success_print("rudp_close() succeed\n");
    return RUDP_SOCKET_SUCCESS;
}

int32_t rudp_recv(
        socket_t* socket,
        uint8_t* buffer, 
        uint32_t buffer_size)
{
    if (socket->options->state != STATE_ESTABLISHED)
        return RUDP_SOCKET_ERROR;
        
    return channel_recv(socket, buffer, buffer_size);
}

int32_t rudp_send(
        socket_t* socket,
        uint8_t* buffer, 
        uint32_t buffer_size)
{
    if (socket->options->state != STATE_ESTABLISHED)
        return RUDP_SOCKET_ERROR;
            
    return channel_send(socket, buffer, buffer_size);
}


int32_t rudp_connect(
        socket_t* socket, 
        const char* addr, 
        uint16_t port)
{
    if (socket->type != TYPE_GENERIC && socket->type != TYPE_CLIENT) {
        goto failed;        
    }
    
    if (socket->options->state != STATE_CLOSED) {
        goto failed;                        
    }
        
    socket->type = TYPE_CLIENT;        
        
    socket->remote_addr.sin_family = AF_INET;
    socket->remote_addr.sin_addr.s_addr = inet_addr(addr);        
    socket->remote_addr.sin_port = htons(port);

    if (socket->remote_addr.sin_addr.s_addr == -1) {
        goto failed;        
    }

    if(connect(socket->socket_fd, 
            (struct sockaddr*)&socket->remote_addr, 
            sizeof(socket->remote_addr)) < 0) {
        goto failed;
    }

    
    socklen_t addr_length;
    getsockname(
            socket->socket_fd, 
            (struct sockaddr*)&socket->local_addr,
            &addr_length);

    socket_t* conn_socket = channel(socket);    
    
    if (!conn_socket) { 
        goto failed; 
    }        
    
    if (pthread_create(&socket->thread, 0, 
            socket_connect_handler, (void*) conn_socket)) {
        goto failed;            
    }
    
    sem_wait(&socket->options->state_lock);
    
    if (conn_socket->options->state != STATE_ESTABLISHED) {
        goto failed;                    
    }

    success_print("rudp_connect() succeed\n");
    return RUDP_SOCKET_SUCCESS;  
    
failed:
    error_print("rudp_connect() failed\n");
    socket->options->state = STATE_CLOSED;
    return RUDP_SOCKET_ERROR;           
}

int32_t rudp_bind(
        socket_t* socket,
        const char* addr, 
        uint16_t port)
{  
    debug_print("rudp_bind()\n");
    
    if (!socket || socket->options->state != STATE_CLOSED) {
        goto failed;        
    }
     
    if (socket->type != TYPE_GENERIC && socket->type != TYPE_SERVER) {
        goto failed;       
    }
    
    socket->local_addr.sin_family = AF_INET;
    socket->local_addr.sin_addr.s_addr = inet_addr(addr);        
    socket->local_addr.sin_port = htons(port);

    if (socket->local_addr.sin_addr.s_addr == -1) {
        goto failed;        
    }

    return bind(socket->socket_fd, 
            (struct sockaddr *) &socket->local_addr, 
            sizeof(socket->local_addr));

failed:
    error_print("rudp_bind() failed\n");
    return RUDP_SOCKET_ERROR;
}

int32_t rudp_listen(
        socket_t* socket, 
        uint32_t queue_max)
{
    debug_print("rudp_listen()\n");
    
    if (socket->options->state != STATE_CLOSED)
        goto failed;                

    if (socket->type != TYPE_GENERIC && socket->type != TYPE_SERVER)
        goto failed;
    
    socket->type = TYPE_SERVER;
    socket->options->state = STATE_LISTEN;
    
    if (pthread_create(&socket->thread, 0, 
            socket_listen_handler, (void*) socket)) {
        goto failed;                 
    }
    
    success_print("rudp_listen() succeed\n");
    return RUDP_SOCKET_SUCCESS;
    
failed:
    error_print("rudp_listen() failed\n");
    socket->options->state = STATE_CLOSED;
    return RUDP_SOCKET_ERROR;
}

void* socket_listen_handler(
        void* socket) 
{
    int32_t buffer_size;
    socket_t* server_socket = (socket_t*) socket;         
    uint32_t sockaddr_in_len = sizeof(struct sockaddr_in);
    
    server_socket->temp_buffer_size = 
            server_socket->options->conn->max_segment_size;
            
    server_socket->temp_buffer = (uint8_t*) calloc (
            server_socket->temp_buffer_size, sizeof(uint8_t));    
        
    while(server_socket->options->state == STATE_LISTEN) {
        
        memset(server_socket->temp_buffer, 0, server_socket->temp_buffer_size);        
        
        buffer_size = recvfrom(server_socket->socket_fd, 
                server_socket->temp_buffer, server_socket->temp_buffer_size, 0, 
                (struct sockaddr *) &server_socket->remote_addr, 
                &sockaddr_in_len);
        
        if (buffer_size < 0) { goto failed; }         
        else if (!buffer_size) { continue; } 
        
        else {
            socket_recv_handler(server_socket, 
                    server_socket->temp_buffer, buffer_size);
        }
    }
    
    success_print("socket_listen_handler() succeed\n");
    //pthread_exit(0);    
    return 0;
    
failed:
    error_print("socket_listen_handler() failed\n");
    //pthread_exit(0);        
    return 0;
    
}

int32_t socket_recv_handler(
        socket_t* socket,
        uint8_t* buffer,
        uint32_t buffer_size) 
{
    debug_print("socket_recv_handler()\n");
    
    hash_node_t* hash_node;
    socket_t* new_socket;
    
    if ((hash_node = channel_waiting(socket)))
        return channel_recv_raw(
                hash_node->value, buffer, buffer_size);
    
    if ((hash_node = channel_established(socket)))
        return channel_recv_raw(
                hash_node->value, buffer, buffer_size);

    if (!(new_socket = channel(socket))) {
        goto failed;
    }
    
    if (channel_recv_raw(new_socket, buffer, buffer_size) < 0) {
        goto failed;
    }
    
    success_print("socket_recv_handler() succeed\n");
    return RUDP_SOCKET_SUCCESS;
    
failed:
    error_print("socket_recv_handler() failed\n");    
    return RUDP_SOCKET_ERROR;      
}

void* socket_connect_handler(
        void* socket) 
{
    int32_t buffer_size;
    socket_t* client_socket = (socket_t*) socket;         
    uint32_t sockaddr_in_len = sizeof(struct sockaddr_in);

    if (channel_handshake_start(socket) < 0) { 
        goto failed; 
    }       
   
    client_socket->temp_buffer_size = 
            client_socket->options->conn->max_segment_size;
            
    client_socket->temp_buffer = (uint8_t*) calloc (
            client_socket->temp_buffer_size, sizeof(uint8_t));                 
    
    while(client_socket->options->state != STATE_CLOSED) {
        
        memset(client_socket->temp_buffer, 0, client_socket->temp_buffer_size);        
        
        buffer_size = recv(client_socket->socket_fd, 
                client_socket->temp_buffer, client_socket->temp_buffer_size, 0);
        
        if (buffer_size < 0) { goto failed; }     
        else if (!buffer_size) { continue; } 
        
        else {
            socket_recv_handler(client_socket, 
                    client_socket->temp_buffer, buffer_size);
        }
    } 

    
failed:
    client_socket->options->state = STATE_CLOSED;
    sem_post(&client_socket->options->state_lock); 
    
    rudp_close(client_socket, false);
    
    error_print("socket_connect_handler() failed\n");
    return (void*) RUDP_SOCKET_ERROR;
}

socket_t* rudp_accept(
        socket_t* socket)
{
    if (!socket || socket->options->state != STATE_LISTEN)
        return 0;        
    
    sem_wait(&socket->options->state_lock);
    
    return (socket_t*) queue_dequeue(socket->ready_queue, false)->data;
}

/*
rudp_state_t rudp_state(
        rudp_socket_t* socket)
{
    if (!socket)
        return RUDP_SOCKET_ERROR;

    return socket->options->state;
}

int32_t rudp_options_get(
        rudp_socket_t* socket, 
        rudp_options_t* options)
{
    if (!socket || !options)
        return RUDP_SOCKET_ERROR; 

    memcpy((void*) &socket->options, (void*) options, 
            sizeof(rudp_options_t));
    
    return RUDP_SOCKET_SUCCESS;
}



*/