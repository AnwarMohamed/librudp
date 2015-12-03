#include "channel.h"

rudp_socket_t* rudp_channel_new(rudp_socket_t* socket)
{
    if (socket->options.state == STATE_LISTEN) {
        rudp_options_t* internal_options = rudp_options();
        internal_options->internal = true;        
        
        rudp_socket_t* internal_socket = rudp_socket(internal_options);
        internal_socket->local_addr = socket->local_addr;
        internal_socket->remote_addr = socket->remote_addr;
        
        rudp_hash_node_t* hash_node = calloc (1, sizeof(rudp_hash_node_t));
        hash_node->key = internal_socket->remote_addr;
        hash_node->value = internal_socket;
        
        printf("%p\n", socket->syn_hash);
        
        HASH_ADD(hh, socket->syn_hash, key, 
                sizeof(struct sockaddr_in), hash_node);
        
        printf("%p\n", socket->syn_hash);
        
        free(internal_options);        
        return internal_socket;
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