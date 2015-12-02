#include "socket.h"

rudp_socket_t* 
rudp_socket(rudp_options_t* options)
{
    rudp_socket_t* rudp_socket_ = 
            (rudp_socket_t*) calloc (1, sizeof(rudp_socket_t));
    
    if (!rudp_socket_) { 
        return (rudp_socket_t*) RUDP_SOCKET_ERROR; 
    }
    
    if (options) {
        rudp_socket_->options = *options;        
    } else {
        
    }
    
    if (!rudp_socket_->options.internal) {        
        rudp_socket_->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
            
        if (rudp_socket_->socket_fd < 0) {
            rudp_close(rudp_socket_);
            return (rudp_socket_t*) RUDP_SOCKET_ERROR;
        }
        
        int32_t optval = 1;
        if (setsockopt(rudp_socket_->socket_fd, SOL_SOCKET, SO_REUSEADDR, 
                (const void *)&optval , sizeof(int)) < 0) {
            rudp_close(rudp_socket_);
            return (rudp_socket_t*) RUDP_SOCKET_ERROR;        
        }        
    }
    
    if (!(rudp_socket_->in_buffer = queue_init()))
        return (rudp_socket_t*) RUDP_SOCKET_ERROR;
        
    if (!(rudp_socket_->out_buffer = queue_init()))
        return (rudp_socket_t*) RUDP_SOCKET_ERROR;
    
    return rudp_socket_;
}

int32_t 
rudp_close(rudp_socket_t* socket) 
{
    if (socket) {
        if (socket->socket_fd)
            close(socket->socket_fd);
            
        queue_free(socket->in_buffer, 0);
        queue_free(socket->out_buffer, 0);
        free(socket);        
    }
    
    return RUDP_SOCKET_SUCCESS;
}

int32_t 
rudp_recv(rudp_socket_t* socket)
{
    
}

int32_t 
rudp_send(rudp_socket_t* socket)
{
    
}

int32_t 
rudp_connect(rudp_socket_t* socket, const char* addr, uint16_t port)
{
    
}

int32_t 
rudp_bind(rudp_socket_t* socket,const char* addr, uint16_t port)
{  
    int32_t inet_addr_ = inet_addr(addr);
    
    if (inet_addr_ == -1) {
        return RUDP_SOCKET_ERROR;
    }
    
    socket->local_addr.sin_family = AF_INET;
    socket->local_addr.sin_addr.s_addr = inet_addr_;        
    socket->local_addr.sin_port = htons(port);

    return bind(socket->socket_fd, (struct sockaddr *) &socket->local_addr, 
            sizeof(socket->local_addr));
}

int32_t 
rudp_listen(rudp_socket_t* socket, uint32_t queue_max)
{
    return RUDP_SOCKET_ERROR; 
}

rudp_socket_t* 
rudp_accept(rudp_socket_t* socket)
{
    return (rudp_socket_t*) RUDP_SOCKET_ERROR; 
}

