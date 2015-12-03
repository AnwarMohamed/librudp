#include "channel.h"

rudp_socket_t* rudp_channel_new(rudp_socket_t* socket)
{
    if (socket->options.internal) {
        
    } else {
        
    }
}

int32_t rudp_channel_handshake(
        rudp_socket_t* socket, 
        uint8_t* buffer, 
        uint32_t buffer_size)
{
    
}

int32_t rudp_channel_send(
        rudp_socket_t* socket,
        uint8_t* buffer,
        uint32_t buffer_size)
{
    
}

int32_t rudp_channel_recv(
        rudp_socket_t* socket,
        uint8_t* buffer,
        uint32_t buffer_size)
{
    
}

int32_t rudp_channel_recv_raw(
        rudp_socket_t* socket,
        uint8_t* buffer,
        uint32_t buffer_size)
{
    
}

int32_t rudp_channel_free(
        rudp_socket_t* socket)
{
    
}

int32_t rudp_channel_close(
        rudp_socket_t* socket)
{
    
}