#include "channel.h"

rudp_socket_t* rudp_channel(rudp_socket_t* socket)
{
    rudp_channel_t* channel = calloc(1, sizeof(rudp_channel_t));
    
    channel->timer_trans_state = rudp_channel_timer(socket);
    channel->timer_cum_ack = rudp_channel_timer(socket);
    channel->timer_null = rudp_channel_timer(socket);
    channel->timer_retrans = rudp_channel_timer(socket);       
    
    if (socket->options.state == STATE_LISTEN) {                
        rudp_options_t* internal_options = rudp_options();
        internal_options->internal = true;
        internal_options->parent = socket;                                
        
        rudp_socket_t* internal_socket = rudp_socket(internal_options);
        internal_socket->options.state = STATE_LISTEN;
        internal_socket->local_addr = socket->local_addr;
        internal_socket->remote_addr = socket->remote_addr;
        internal_socket->socket_fd = socket->socket_fd;
        internal_socket->channel = channel;
        
        rudp_hash_node_t* hash_node = calloc (1, sizeof(rudp_hash_node_t));
        hash_node->key = internal_socket->remote_addr;
        hash_node->value = internal_socket;                
        
        HASH_ADD(hh, socket->syn_hash, key, 
                sizeof(struct sockaddr_in), hash_node);            
        
        free(internal_options);        
        return internal_socket;
    } else {
        

        return socket;
    }
}

void rudp_channel_timer_handler(rudp_channel_timer_t* timer)
{
    printf("rudp_channel_timer_handler() triggered\n");
}

void rudp_channel_timer_close(
        rudp_channel_timer_t* timer)
{
    free(timer->event);
    free(timer);
}

rudp_channel_timer_t* rudp_channel_timer(
        rudp_socket_t* socket)
{
    rudp_channel_timer_t* timer = calloc(1, sizeof(rudp_channel_timer_t));
    timer->event = calloc(1, sizeof(struct sigevent));
    timer->event->sigev_notify = SIGEV_SIGNAL;        
    timer->event->sigev_signo = SIGRTMAX;
    timer->event->sigev_value.sival_ptr = timer;

    timer_create(CLOCK_REALTIME, timer->event, &timer->timer);
    
    return timer;
}

void rudp_channel_remove(
        rudp_socket_t* socket)
{
    if (socket->options.state == STATE_LISTEN) { 
        rudp_hash_node_t* hash_node;        
        HASH_FIND(hh, socket->options.parent->syn_hash, 
                &socket->remote_addr, sizeof(struct sockaddr_in), 
                hash_node);
                
        if (!hash_node) { return; }
        HASH_DELETE(hh, socket->options.parent->syn_hash, hash_node); 

        rudp_close(hash_node->value, false);
        free(hash_node);                   
    }
}

int32_t rudp_channel_handshake(
        rudp_socket_t* socket, 
        uint8_t* buffer, 
        uint32_t buffer_size)
{
    if (socket->options.state == STATE_LISTEN) {   
        rudp_packet_t* packet;
        
        packet = rudp_packet(PACKET_TYPE_SYN, 0, buffer, buffer_size);
        
        if (!packet) {
            rudp_channel_remove(socket);
            
            printf("rudp_channel_recv_raw() failed\n");
            return RUDP_SOCKET_ERROR;
        }
        
        rudp_channel_negotiate(
                socket, 
                &socket->options.parent->options,
                ((rudp_syn_packet_t*) packet->buffer)->aux_header);
        
        rudp_packet_free(packet);
        
        packet = rudp_packet(PACKET_TYPE_SYN_ACK, socket, 0, 0);                
                
        rudp_channel_send_packet(socket, packet);        
    }
}

int32_t rudp_channel_send(
        rudp_socket_t* socket,
        uint8_t* buffer,
        uint32_t buffer_size)
{
    return rudp_channel_send_raw(socket, buffer, buffer_size);
}


int32_t rudp_channel_send_raw(
        rudp_socket_t* socket,
        uint8_t* buffer,
        uint32_t buffer_size)
{
    return sendto(socket->socket_fd, buffer, buffer_size, 0, 
            (struct sockaddr *)& socket->remote_addr, 
            sizeof(socket->remote_addr));
}

int32_t rudp_channel_send_packet(
        rudp_socket_t* socket, 
        rudp_packet_t* packet)
{
    packet->transmission_time = rudp_timestamp();
    return rudp_channel_send_raw(socket, 
            packet->buffer, packet->buffer_size);
}


int32_t rudp_channel_recv(
        rudp_socket_t* socket,
        uint8_t* buffer,
        uint32_t buffer_size)
{
    if (!queue_size(socket->in_buffer))
        return 0;
}

int32_t rudp_channel_recv_raw(
        rudp_socket_t* socket,
        uint8_t* buffer,
        uint32_t buffer_size)
{
    if (socket->options.state == STATE_LISTEN ||
        socket->options.state == STATE_SYN_SENT)
        return rudp_channel_handshake(socket, buffer, buffer_size);        
    
    
    printf("rudp_channel_recv_raw() failed\n");
    return RUDP_SOCKET_SUCCESS;    
}

void rudp_channel_negotiate(
        rudp_socket_t* socket,
        rudp_options_t* server,
        rudp_syn_packet_header_t* client)
{
    socket->options.identifier = client->identifier;
    socket->options.flags = client->option_flags;

    if (client->max_auto_reset > server->max_auto_reset) {
        client->max_auto_reset = server->max_auto_reset;                
    } else {
        socket->options.max_auto_reset = client->max_auto_reset;
    }
             
    if (client->max_cum_ack > server->max_cum_ack) {
        client->max_cum_ack = server->max_cum_ack;                
    } else {
        socket->options.max_cum_ack = client->max_cum_ack;
    }

    if (client->max_out_segments > server->max_out_segments) {
        client->max_out_segments = server->max_out_segments;                
    } else {
        socket->options.max_out_segments = client->max_out_segments;
    }

    if (client->max_retransmissions > server->max_retransmissions) {
        client->max_retransmissions = server->max_retransmissions;                
    } else {
        socket->options.max_retransmissions = client->max_retransmissions;
    }

    if (client->max_out_sequences > server->max_out_sequences) {
        client->max_out_sequences = server->max_out_sequences;                
    } else {
        socket->options.max_out_sequences = client->max_out_sequences;
    }

    if (client->timeout_cum_ack > server->timeout_cum_ack) {
        client->timeout_cum_ack = server->timeout_cum_ack;                
    } else {
        socket->options.timeout_cum_ack = client->timeout_cum_ack;
    }  

    if (client->timeout_null > server->timeout_null) {
        client->timeout_null = server->timeout_null;                
    } else {
        socket->options.timeout_null = client->timeout_null;
    }   

    if (client->timeout_retransmission > server->timeout_retransmission) {
        client->timeout_retransmission = server->timeout_retransmission;                
    } else {
        socket->options.timeout_retransmission = client->timeout_retransmission;
    }      

    if (client->timeout_trans_state > server->timeout_trans_state) {
        client->timeout_trans_state = server->timeout_trans_state;                
    } else {
        socket->options.timeout_trans_state = client->timeout_trans_state;
    }    
}

int32_t rudp_channel_close(
        rudp_socket_t* socket)
{
    if (socket && socket->channel) {
        timer_settime(socket->channel->timer_retrans, 0, NULL, NULL);
        
        free(socket->channel->timer_retrans->event);
        free(socket->channel->timer_retrans);        
        free(socket->channel);
    }
}