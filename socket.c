#include "socket.h"


rudp_socket_t* rudp_socket(
        rudp_options_t* options)
{
    rudp_socket_t* rudp_socket_ = 
            (rudp_socket_t*) calloc (1, sizeof(rudp_socket_t));
    
    if (!rudp_socket_) { 
        return (rudp_socket_t*) RUDP_SOCKET_ERROR; 
    }
    
    if (options) {
        rudp_socket_->options = *options;
    } else {
        rudp_socket_->options.state = STATE_CLOSED;
        rudp_socket_->options.max_segment_size = 1024;
    }
    
    if (!rudp_socket_->options.internal) {        
        int32_t flags, reuseaddr_on = 1;        
        rudp_socket_->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
            
        if (rudp_socket_->socket_fd < 0) {
            rudp_close(rudp_socket_, 1);
            return (rudp_socket_t*) RUDP_SOCKET_ERROR;
        }
                
        if (setsockopt(rudp_socket_->socket_fd, SOL_SOCKET, SO_REUSEADDR, 
                (const void *)&reuseaddr_on , sizeof(int32_t)) < 0) {
            rudp_close(rudp_socket_, 1);
            return (rudp_socket_t*) RUDP_SOCKET_ERROR;        
        }      
        
        if ((flags = fcntl(rudp_socket_->socket_fd, F_GETFL)) < 0) {
            rudp_close(rudp_socket_, 1);
            return (rudp_socket_t*) RUDP_SOCKET_ERROR;  
        }

        /*
        flags |= O_NONBLOCK;
        if (fcntl(rudp_socket_->socket_fd, F_SETFL, flags) < 0) {
            rudp_close(rudp_socket_);
            return (rudp_socket_t*) RUDP_SOCKET_ERROR;             
        }
        */
    }
    
    if (!(rudp_socket_->in_buffer = queue_init()))
        return (rudp_socket_t*) RUDP_SOCKET_ERROR;
        
    if (!(rudp_socket_->out_buffer = queue_init()))
        return (rudp_socket_t*) RUDP_SOCKET_ERROR;
        
    if (!(rudp_socket_->syn_queue = queue_init()))
        return (rudp_socket_t*) RUDP_SOCKET_ERROR;

    if (!(rudp_socket_->accept_queue = queue_init()))
        return (rudp_socket_t*) RUDP_SOCKET_ERROR;        
    
    return rudp_socket_;
}

int32_t rudp_close(
        rudp_socket_t* socket,
        uint8_t immediately) 
{
    if (socket) {
        
        socket->options.state = STATE_CLOSED;
        
        if (socket->socket_fd)
            close(socket->socket_fd);
            
        queue_free(socket->in_buffer, 0);
        queue_free(socket->out_buffer, 0);
        
        queue_free(socket->syn_queue, 0);
        queue_free(socket->accept_queue, 0);
        
        if (socket->listen_thread)
            pthread_join(socket->listen_thread, NULL);
        
        free(socket);        
    }
    
    return RUDP_SOCKET_SUCCESS;
}

int32_t rudp_recv(
        rudp_socket_t* socket)
{
    if (socket->options.state != STATE_ESTABLISHED)
        return RUDP_SOCKET_ERROR;
}

int32_t rudp_send(
        rudp_socket_t* socket)
{
    if (socket->options.state != STATE_ESTABLISHED)
        return RUDP_SOCKET_ERROR;    
}

int32_t rudp_connect(
        rudp_socket_t* socket, 
        const char* addr, 
        uint16_t port)
{
    if (socket->options.state != STATE_CLOSED)
        return RUDP_SOCKET_ERROR;    
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
    
    if (pthread_create(&socket->listen_thread, NULL, 
            rudp_listen_handler, (void*) socket)) {
        socket->options.state = STATE_CLOSED;
        return RUDP_SOCKET_ERROR; 
    }        
    
    return RUDP_SOCKET_SUCCESS;
}

void* rudp_listen_handler(
        void* socket) {
    rudp_socket_t* rudp_socket = (rudp_socket_t*) socket;     
    
    uint32_t buffer_size, sockaddr_in_len = sizeof(struct sockaddr_in);
    uint32_t buffer_max_size = rudp_socket->options.max_segment_size;
    uint8_t* buffer = (uint8_t*) calloc (buffer_max_size, sizeof(uint8_t));
        
    while(rudp_socket->options.state == STATE_LISTEN) {
        
        memset(buffer, 0, buffer_max_size);        
        
        buffer_size = recvfrom(rudp_socket->socket_fd, 
                buffer, buffer_max_size, 0, 
                (struct sockaddr *) &rudp_socket->remote_addr, 
                &sockaddr_in_len);
        
        if (buffer_size < 0) {
            printf("rudp_listen_handler() failed\n");
            goto cleanup;
        } else if (!buffer_size)
            continue;            
        else
            rudp_recv_handler(rudp_socket, buffer, buffer_size);            
    }
    
cleanup:
    free(buffer);
    pthread_exit(0);
    
    return (void*) RUDP_SOCKET_SUCCESS;
}

void rudp_recv_handler(
        rudp_socket_t* socket,
        uint8_t* buffer,
        uint32_t buffer_size) 
{
        printf("%s:%d: %s", 
                inet_ntoa(socket->remote_addr.sin_addr), 
                ntohs(socket->remote_addr.sin_port), 
                buffer);
}

rudp_socket_t* rudp_accept(
        rudp_socket_t* socket)
{
    if (!socket || socket->options.state != STATE_LISTEN)
        return (rudp_socket_t*) RUDP_SOCKET_ERROR;        
    
    return (rudp_socket_t*) queue_dequeue(
            socket->accept_queue)->data; 
}

rudp_state_t rudp_state(
        rudp_socket_t* socket)
{
    if (!socket)
        return RUDP_SOCKET_ERROR;

    return socket->options.state;
}