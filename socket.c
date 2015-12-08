#include "socket.h"

static void rudp_timer_handler(
        int signo, 
        siginfo_t *si, 
        void *uc) 
{    
    rudp_channel_timer_handler(si->si_value.sival_ptr);
}

rudp_socket_t* rudp_socket(
        rudp_options_t* options)
{
    rudp_socket_t* rudp_socket_ = 
            (rudp_socket_t*) calloc (1, sizeof(rudp_socket_t));
    
    if (!rudp_socket_) { 
        return NULL; 
    }
    
    if (options) {
        rudp_socket_->options = *options;
    } else {
        rudp_options_t* options_ = rudp_options();        
        rudp_socket_->options = *options_;
        sem_init(&rudp_socket_->options.state_lock, 0, 0);
        free(options_);
    }
    
    if (!rudp_socket_->options.internal) {        
        if (!(rudp_socket_ = rudp_linux_socket(rudp_socket_))) {
            rudp_close(rudp_socket_, false);
            return NULL;            
        }
    }
    
    if (!(rudp_socket_->in_buffer = queue_init())) {
        rudp_close(rudp_socket_, false);
        return NULL;
    }
        
        
    if (!(rudp_socket_->out_buffer = queue_init())) {
        rudp_close(rudp_socket_, false);
        return NULL;
    }

    if (!(rudp_socket_->ready_queue = queue_init())) {
        rudp_close(rudp_socket_, false);
        return NULL;        
    }        
    
    struct sigaction sa; 
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = rudp_timer_handler;
    
    if(sigaction(RUDP_SOCKET_SIGNAL, &sa, NULL) < 0) {
        rudp_close(rudp_socket_, false);
        return NULL;
    }        
    
    return rudp_socket_;
}

rudp_socket_t* rudp_linux_socket(
        rudp_socket_t* rudp_socket_)
{
    int32_t flags, reuseaddr_on = 1;        
    rudp_socket_->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
        
    if (rudp_socket_->socket_fd < 0) {
        return NULL;
    }
            
    if (setsockopt(rudp_socket_->socket_fd, SOL_SOCKET, SO_REUSEADDR, 
            (const void *)&reuseaddr_on , sizeof(int32_t)) < 0) {
        rudp_close(rudp_socket_, 1);
        return NULL;
    }      
    
    if ((flags = fcntl(rudp_socket_->socket_fd, F_GETFL)) < 0) {
        rudp_close(rudp_socket_, 1);
        return NULL;
    }
    
    return rudp_socket_;
}

int32_t rudp_close(
        rudp_socket_t* socket,
        bool immediately) 
{
    if (socket) {           

        if (!socket->options.internal) {
            sem_destroy(&socket->options.state_lock);
            if (socket->socket_fd)
                close(socket->socket_fd);
        }                     

        if (socket->channel)
            rudp_channel_close(socket);
        
        socket->options.state = STATE_CLOSED;            
                
        queue_free(socket->in_buffer, !immediately);
        queue_free(socket->out_buffer, !immediately);
                
        queue_free(socket->ready_queue, !immediately);

        if (socket->thread) {
            if (immediately)
                pthread_kill(socket->thread, SIGINT);
            else
                pthread_join(socket->thread, NULL);
        }
        
        if (socket->temp_buffer)
            free(socket->temp_buffer);        
        
        free(socket);        
    }
    
    return RUDP_SOCKET_SUCCESS;
}

int32_t rudp_recv(
        rudp_socket_t* socket,
        uint8_t* buffer, 
        uint32_t buffer_size)
{
    if (socket->options.state != STATE_ESTABLISHED)
        return RUDP_SOCKET_ERROR;
        
    return rudp_channel_recv(socket, buffer, buffer_size);
}

int32_t rudp_send(
        rudp_socket_t* socket,
        uint8_t* buffer, 
        uint32_t buffer_size)
{
    if (socket->options.state != STATE_ESTABLISHED)
        return RUDP_SOCKET_ERROR;
        
    return rudp_channel_send(socket, buffer, buffer_size);
}

int32_t rudp_connect(
        rudp_socket_t* socket, 
        const char* addr, 
        uint16_t port)
{
    if (socket->options.state != STATE_CLOSED)
        return RUDP_SOCKET_ERROR;

    int32_t inet_addr_ = inet_addr(addr);
    
    if (inet_addr_ == -1) {
        return RUDP_SOCKET_ERROR;
    }
    
    socket->remote_addr.sin_family = AF_INET;
    socket->remote_addr.sin_addr.s_addr = inet_addr_;        
    socket->remote_addr.sin_port = htons(port);

    if(connect(socket->socket_fd, 
            (struct sockaddr*)&socket->remote_addr, 
            sizeof(socket->remote_addr)) < 0) {
        return RUDP_SOCKET_ERROR;
    }

    rudp_socket_t* conn_socket = rudp_channel(socket);
    
    if (!conn_socket) {
        return RUDP_SOCKET_ERROR;
    }
    
    if (pthread_create(&socket->thread, NULL, 
            rudp_connect_handler, (void*) conn_socket)) {
        socket->options.state = STATE_CLOSED;
        return RUDP_SOCKET_ERROR; 
    }    
    
    sem_wait(&socket->options.state_lock);
    
    return conn_socket->options.state == STATE_ESTABLISHED ? 
            RUDP_SOCKET_SUCCESS : RUDP_SOCKET_ERROR;            
}

int32_t rudp_bind(
        rudp_socket_t* socket,
        const char* addr, 
        uint16_t port)
{  
    if (!socket || socket->options.state != STATE_CLOSED)
        return RUDP_SOCKET_ERROR;
        
    int32_t inet_addr_ = inet_addr(addr);
    
    if (inet_addr_ == -1) {
        return RUDP_SOCKET_ERROR;
    }
    
    socket->local_addr.sin_family = AF_INET;
    socket->local_addr.sin_addr.s_addr = inet_addr_;        
    socket->local_addr.sin_port = htons(port);

    return bind(socket->socket_fd, 
            (struct sockaddr *) &socket->local_addr, 
            sizeof(socket->local_addr));
}

int32_t rudp_listen(
        rudp_socket_t* socket, 
        uint32_t queue_max)
{
    if (socket->options.state != STATE_CLOSED)
        return RUDP_SOCKET_ERROR;                
    
    socket->options.state = STATE_LISTEN;
    
    if (pthread_create(&socket->thread, NULL, rudp_listen_handler, 
            (void*) socket)) {
        socket->options.state = STATE_CLOSED;
        return RUDP_SOCKET_ERROR; 
    }        
    
    return RUDP_SOCKET_SUCCESS;
}

void* rudp_listen_handler(
        void* socket) 
{
    rudp_socket_t* server_socket = (rudp_socket_t*) socket;         
    uint32_t buffer_size, sockaddr_in_len = sizeof(struct sockaddr_in);
    
    server_socket->temp_buffer_size = 
            server_socket->options.conn->max_segment_size;
    server_socket->temp_buffer = (uint8_t*) calloc (
            server_socket->temp_buffer_size, sizeof(uint8_t));    
        
    while(server_socket->options.state == STATE_LISTEN) {
        
        memset(server_socket->temp_buffer, 0, server_socket->temp_buffer_size);        
        
        buffer_size = recvfrom(server_socket->socket_fd, 
                server_socket->temp_buffer, server_socket->temp_buffer_size, 0, 
                (struct sockaddr *) &server_socket->remote_addr, 
                &sockaddr_in_len);
        
        if (buffer_size < 0) { goto failed; }         
        else if (!buffer_size) { continue; } 
        
        else {
            rudp_recv_handler(server_socket, 
                    server_socket->temp_buffer, buffer_size);
        }
    }
    
succeed:
    printf("rudp_listen_handler() succeed\n");
    pthread_exit(0);    
    return (void*) RUDP_SOCKET_SUCCESS;
failed:
    printf("rudp_listen_handler() failed\n");
    pthread_exit(0);    
    return (void*) RUDP_SOCKET_ERROR;
}

int32_t rudp_recv_handler(
        rudp_socket_t* socket,
        uint8_t* buffer,
        uint32_t buffer_size) 
{
    rudp_hash_node_t* hash_node;                        
    
    printf("%s:%d\n", 
            inet_ntoa(socket->remote_addr.sin_addr), 
            ntohs(socket->remote_addr.sin_port));    
    
    if ((hash_node = rudp_channel_waiting(socket)))
        return rudp_channel_recv_raw(
                hash_node->value, buffer, buffer_size);        
    //    return rudp_channel_handshake(
    //            hash_node->value, buffer, buffer_size);
    
    if ((hash_node = rudp_channel_established(socket)))
        return rudp_channel_recv_raw(
                hash_node->value, buffer, buffer_size);

    rudp_socket_t* internal_socket =  rudp_channel(socket);
    
    if (internal_socket) {
        printf("rudp_channel() succeed\n");
        return rudp_channel_recv_raw(
                internal_socket, buffer, buffer_size);
    } else {
        printf("rudp_channel() failed\n");
        return RUDP_SOCKET_ERROR;
    }    
}

int32_t rudp_channel_start_handshake(
    rudp_socket_t* socket)
{
    
}

void* rudp_connect_handler(
        void* socket) 
{
    rudp_socket_t* client_socket = (rudp_socket_t*) socket;         
    uint32_t buffer_size, sockaddr_in_len = sizeof(struct sockaddr_in);

    if (rudp_channel_start_handshake(socket) < 0) { goto failed; } 
   
    client_socket->temp_buffer_size = 
            client_socket->options.conn->max_segment_size;
    client_socket->temp_buffer = (uint8_t*) calloc (
            client_socket->temp_buffer_size, sizeof(uint8_t));                 
    
    while(client_socket->options.state != STATE_CLOSED) {
        
        memset(client_socket->temp_buffer, 0, client_socket->temp_buffer_size);        
        
        buffer_size = recvfrom(client_socket->socket_fd, 
                client_socket->temp_buffer, client_socket->temp_buffer_size, 0, 
                (struct sockaddr *) &client_socket->remote_addr, 
                &sockaddr_in_len);
        
        if (buffer_size < 0) { goto failed; }     
        else if (!buffer_size) { continue; } 
        
        else {
            rudp_recv_handler(client_socket, 
                    client_socket->temp_buffer, buffer_size);
        }
    } 

succeed:
    printf("rudp_connect_handler() succeed\n");
    
    client_socket->options.state = STATE_CLOSED;
    sem_post(&client_socket->options.state_lock);
    pthread_exit(0);    
    return (void*) RUDP_SOCKET_SUCCESS;
failed:
    printf("rudp_connect_handler() failed\n");
    
    client_socket->options.state = STATE_CLOSED;        
    sem_post(&client_socket->options.state_lock);
    pthread_exit(0);
    
    return (void*) RUDP_SOCKET_ERROR;
}

rudp_socket_t* rudp_accept(
        rudp_socket_t* socket)
{
    if (!socket || socket->options.state != STATE_LISTEN)
        return (rudp_socket_t*) RUDP_SOCKET_ERROR;        
    
    sem_wait(&socket->options.state_lock);
    
    return (rudp_socket_t*) queue_dequeue(
            socket->ready_queue)->data; 
}

rudp_state_t rudp_state(
        rudp_socket_t* socket)
{
    if (!socket)
        return RUDP_SOCKET_ERROR;

    return socket->options.state;
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

rudp_options_t* rudp_options()
{
    rudp_options_t* options = 
            (rudp_options_t*) calloc(1, sizeof(rudp_options_t));
    
    options->conn = 
            (rudp_conn_options_t*) calloc(1, sizeof(rudp_conn_options_t));
            
    options->state = STATE_CLOSED;
    
    options->conn->version = 1;    
    options->conn->max_segment_size = 1024;
    
    return options;
}

