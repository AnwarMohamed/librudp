#include "channel.h"


socket_t* channel(socket_t* socket)
{
    debug_print("channel()\n");
    
    socket_t* new_socket = NULL;
    hash_node_t* hash_node = NULL;
    socket_t* hash_socket = NULL;
    
    channel_t* channel = calloc(1, sizeof(channel_t));
    
    channel->timer_trans_state = utimer(socket, 0, TIMER_TRANS_STATE);
    channel->timer_cum_ack = utimer(socket, 0, TIMER_CUM_ACK);
    channel->timer_null = utimer(socket, 0, TIMER_NULL);    
    
    channel->sequence = rudp_sequence(socket);
    
    channel->out_window = window(socket->options->conn->max_window_size);
    channel->in_window = window(socket->options->conn->max_window_size);
    
    if (socket->type == TYPE_SERVER) {                
        socket_options_t* new_socket_options = socket_options();
        new_socket_options->internal = true;
        new_socket_options->parent = socket;                                
        
        new_socket = rudp_socket(new_socket_options);
        new_socket->options->state = STATE_LISTEN;                
        
        memcpy(new_socket->options->conn, socket->options->conn, 
                SYN_PACKET_HEADER_LENGTH);                                                   
                
    } else {
        new_socket = socket;                
    }
    
    new_socket->local_addr = socket->local_addr;
    new_socket->remote_addr = socket->remote_addr;
    new_socket->socket_fd = socket->socket_fd;
    new_socket->channel = channel;        
    
    hash_node = calloc (1, sizeof(hash_node_t));
    hash_node->key = new_socket->remote_addr;
    hash_node->value = new_socket;

    hash_socket = new_socket->options->internal ? 
            new_socket->options->parent :new_socket;
    
    HASH_ADD(hh, hash_socket->waiting_hash, key, 
            sizeof(struct sockaddr_in), hash_node);        
    
    debug_print("channel() succeed\n");
    return new_socket;
}

int32_t channel_free(
        socket_t* socket)
{
    if (socket && socket->channel) {          
        utimer_free(socket->channel->timer_cum_ack);
        utimer_free(socket->channel->timer_null);
        utimer_free(socket->channel->timer_trans_state);
        
        window_free(socket->channel->out_window);
        window_free(socket->channel->in_window);
        
        free(socket->channel);
    }
}


void channel_timer_handler(utimer_t* timer)
{
    debug_print("channel_timer_handler() triggered\n");        
    
    if (timer->type == TIMER_RETRANS) {        
        
        if (timer->packet->needs_ack) {            
            packet_timeout(timer);            
        } else {
            utimer_set(timer, 0);
        }
    } else if (timer->type == TIMER_NULL) {
        packet_t* null_packet = packet(PACKET_TYPE_NULL, timer->socket);
        window_out_enqueue(timer->socket->channel->out_window, null_packet);
    }
    
}



/*
void rudp_channel_deattach(
        rudp_socket_t* socket)
{    
    if (socket->options->state == STATE_LISTEN) { 
        rudp_channel_deattach_node(socket);
        rudp_close(socket, false);
    }
}

void rudp_channel_deattach_node(
        rudp_socket_t* socket)
{
    rudp_hash_node_t* hash_node;
    HASH_FIND(hh, socket->options->parent->waiting_hash, 
        &socket->remote_addr, sizeof(struct sockaddr_in), 
        hash_node);
        
    if (hash_node) {
        HASH_DELETE(hh, socket->options->parent->waiting_hash, hash_node); 
        free(hash_node);
    }

    HASH_FIND(hh, socket->options->parent->established_hash,
        &socket->remote_addr, sizeof(struct sockaddr_in),
        hash_node);

    if (hash_node) {
        HASH_DELETE(hh, socket->options->parent->established_hash, hash_node);
        free(hash_node);
    }
}
*/

int32_t channel_handshake_start(
    socket_t* socket)
{
    debug_print("channel_handshake_start()\n");        
    
    packet_t* syn_packet = packet(PACKET_TYPE_SYN, socket);    
    
    if (!syn_packet) { 
        goto failed; 
    }

    window_out_enqueue(socket->channel->out_window, syn_packet);

    socket->options->state = STATE_SYN_SENT;
    
    debug_print("channel_handshake_start() succeed\n");
    return RUDP_SOCKET_SUCCESS;
    
failed:
    debug_print("channel_handshake_start() failed\n");
    
    packet_free(syn_packet);
    return RUDP_SOCKET_ERROR;    
}

/*
int32_t rudp_channel_handshake(
        rudp_socket_t* socket, 
        uint8_t* buffer, 
        uint32_t buffer_size)
{
    rudp_packet_t* packet;
       
failed:
    rudp_channel_deattach(socket);
    return RUDP_SOCKET_ERROR;
}

int32_t rudp_channel_send(
        rudp_socket_t* socket,
        uint8_t* buffer,
        uint32_t buffer_size)
{    
    return rudp_channel_send_raw(socket, buffer, buffer_size);
}
*/

int32_t channel_send_raw(
        socket_t* socket,
        uint8_t* buffer,
        uint32_t buffer_size)
{
    debug_print("channel_send_raw()\n");
    
    if (sendto(socket->socket_fd, buffer, buffer_size, 0, 
            (struct sockaddr *)& socket->remote_addr, 
            sizeof(socket->remote_addr)) < 0) {
        goto failed;
    }
        
    debug_print("channel_send_raw() succeed\n");
    return RUDP_SOCKET_SUCCESS;
    
failed:
    debug_print("channel_send_raw() failed\n");
    return RUDP_SOCKET_ERROR;            
}
 

int32_t channel_send_ack(
        socket_t* socket,
        packet_t* recv_packet)
{
    debug_print("channel_send_ack()\n");    
    
    if (!recv_packet->ack) {
        switch(recv_packet->type) {
        case PACKET_TYPE_SYN:
            recv_packet->ack = packet(PACKET_TYPE_SYN_ACK, socket);
            break;
        case PACKET_TYPE_SYN_ACK:        
        case PACKET_TYPE_DATA:
        case PACKET_TYPE_EACK:
        case PACKET_TYPE_NULL:
            recv_packet->ack = packet(PACKET_TYPE_ACK, socket);
            break;
        case PACKET_TYPE_RESET:
            recv_packet->ack = packet(PACKET_TYPE_RESET_ACK, socket);
            break;
        case PACKET_TYPE_TCS:
            recv_packet->ack = packet(PACKET_TYPE_TCS_ACK, socket);
            break;
        default:
            goto failed; 
        } 
        
        if (!recv_packet->ack) { 
            goto failed; 
        }
    }    
            
    recv_packet->ack->header->sequence = 
            (recv_packet->ack->type == PACKET_TYPE_SYN_ACK ?
            socket->channel->sequence: recv_packet->header->acknowledge);
            
    recv_packet->ack->header->acknowledge = recv_packet->header->sequence + 
            (recv_packet->data_buffer_size ? recv_packet->data_buffer_size: 1);
    
    packet_checksum_add(recv_packet->ack);
    
    if (recv_packet->ack->needs_ack) {
        
        window_out_enqueue(socket->channel->out_window, recv_packet->ack);
    } else {
        channel_send_packet(recv_packet->ack);
        packet_free(recv_packet);
    }

    debug_print("channel_send_ack() succeed\n");
    return RUDP_SOCKET_SUCCESS;
    
failed:
    debug_print("channel_send_ack() failed\n");
    return RUDP_SOCKET_ERROR;
}


int32_t channel_send_packet( 
        packet_t* packet)
{                    
    packet->transmission_time = rudp_timestamp();
    
    packet->destination_addr = 
            ntohl(packet->socket->remote_addr.sin_addr.s_addr);
    packet->destination_port = 
            ntohs(packet->socket->remote_addr.sin_port);
            
    packet->source_addr = 
            ntohl(packet->socket->local_addr.sin_addr.s_addr);
    packet->source_port = 
            ntohs(packet->socket->local_addr.sin_port);
    
    if (packet->needs_ack) {
        utimer_set(packet->timer_retrans, 
                packet->socket->options->conn->timeout_retransmission);
    }
    
    packet_print(packet);
    
    return channel_send_raw(packet->socket, 
            packet->buffer, packet->buffer_size);
}

/*
int32_t rudp_channel_recv(
        rudp_socket_t* socket,
        uint8_t* buffer,
        uint32_t buffer_size)
{
    if (!queue_size(socket->in_buffer))
        return 0;
        
    //queue_dequeue(socket->in_buffer);
}

void rudp_channel_retransmit(
        rudp_socket_t* socket)
{
    
}
*/

int32_t channel_recv_eack(
        socket_t* socket,
        packet_t* packet)
{
}

int32_t channel_recv_reset_ack(
        socket_t* socket,
        packet_t* packet)
{
}

int32_t channel_recv_reset(
        socket_t* socket,
        packet_t* packet)
{
}

int32_t channel_recv_data(
        socket_t* socket,
        packet_t* packet)
{
}

int32_t channel_recv_null(
        socket_t* socket,
        packet_t* packet)
{
    debug_print("channel_recv_null()\n");    
    
    socket->channel->acknowledge = packet->header->sequence + 1;
    socket->channel->sequence = packet->header->acknowledge; 

    if (channel_send_ack(socket, packet) < 0)
        goto failed;

    debug_print("channel_recv_null() succeed\n");
    return RUDP_SOCKET_SUCCESS; 
    
failed:    
    debug_print("channel_recv_null() failed\n");
    return RUDP_SOCKET_ERROR;       
}

int32_t channel_recv_syn(
        socket_t* socket,
        packet_t* packet)
{
    debug_print("channel_recv_syn()\n");
    
    if (socket->options->state != STATE_LISTEN)
        goto failed;            
        
    socket->channel->acknowledge = packet->header->sequence + 1;
    
    socket->options->conn->identifier = packet->syn_header->identifier;
    socket->options->conn->option_flags = packet->syn_header->option_flags;

    
    socket->options->peer->max_segment_size =
            packet->syn_header->max_segment_size;
    
    socket->options->peer->max_window_size =  
            packet->syn_header->max_window_size;
    
        
    socket->options->conn->timeout_retransmission = 
            MIN(socket->options->conn->timeout_retransmission, 
            packet->syn_header->timeout_retransmission); 
        
    socket->options->conn->max_auto_reset = 
            MIN(socket->options->conn->max_auto_reset, 
            packet->syn_header->max_auto_reset);
    
    socket->options->conn->max_cum_ack = 
            MIN(socket->options->conn->max_cum_ack, 
            packet->syn_header->max_cum_ack);     
 
    socket->options->conn->max_retransmissions = 
            MIN(socket->options->conn->max_retransmissions, 
            packet->syn_header->max_retransmissions);    
 
    socket->options->conn->max_out_sequences = 
            MIN(socket->options->conn->max_out_sequences, 
            packet->syn_header->max_out_sequences);   

    socket->options->conn->timeout_cum_ack = 
            MIN(socket->options->conn->timeout_cum_ack, 
            packet->syn_header->timeout_cum_ack);   

    socket->options->conn->timeout_null = 2*
            MIN(socket->options->conn->timeout_null, 
            packet->syn_header->timeout_null);                      

    socket->options->conn->timeout_trans_state = 
            MIN(socket->options->conn->timeout_trans_state, 
            packet->syn_header->timeout_trans_state);                
        
    if (channel_send_ack(socket, packet) < 0) {
        goto failed;        
    }
    
    socket->options->state = STATE_SYN_RECEIVED;
      
    debug_print("channel_recv_syn() succeed\n");
    return RUDP_SOCKET_SUCCESS; 
    
failed:    
    debug_print("channel_recv_syn() failed\n");
    return RUDP_SOCKET_ERROR;
}


int32_t channel_recv_syn_ack(
        socket_t* socket,
        packet_t* packet)
{
    debug_print("channel_recv_syn_ack()\n");
    
    if (socket->options->state != STATE_SYN_SENT)
        goto failed;

    socket->channel->acknowledge = packet->header->sequence + 1;
    socket->channel->sequence = packet->header->acknowledge;
            
    socket->options->conn->identifier = packet->syn_header->identifier;
    socket->options->conn->option_flags = packet->syn_header->option_flags;
    socket->options->conn->max_window_size = packet->syn_header->max_window_size;
    
    if (packet->syn_header->max_auto_reset > 
                socket->options->conn->max_auto_reset ||
        packet->syn_header->max_cum_ack > 
                socket->options->conn->max_cum_ack ||
        packet->syn_header->max_retransmissions > 
                socket->options->conn->max_retransmissions ||
        packet->syn_header->max_out_sequences > 
                socket->options->conn->max_out_sequences ||
        packet->syn_header->timeout_cum_ack > 
                socket->options->conn->timeout_cum_ack ||
        packet->syn_header->timeout_null > 
                socket->options->conn->timeout_null ||
        packet->syn_header->timeout_retransmission > 
                socket->options->conn->timeout_retransmission ||
        packet->syn_header->timeout_trans_state > 
                socket->options->conn->timeout_trans_state) {
        goto failed;             
              
    }
    
    if (channel_send_ack(socket, packet) < 0)
        goto failed;

    if (socket->type == TYPE_CLIENT) {
        utimer_set(socket->channel->timer_null, 
                socket->options->conn->timeout_null);
    }

    socket->options->state = STATE_ESTABLISHED;
    
  
    debug_print("channel_recv_syn_ack() succeed\n");
    return RUDP_SOCKET_SUCCESS; 
    
failed:    
    debug_print("channel_recv_syn_ack() failed\n");
    return RUDP_SOCKET_ERROR;    
}


int32_t channel_recv_ack(
        socket_t* socket,
        packet_t* packet)
{
    socket->channel->acknowledge = packet->header->sequence + 1;
    socket->channel->sequence = packet->header->acknowledge;
    
    return RUDP_SOCKET_SUCCESS;
}

int32_t channel_recv_tcs(
        socket_t* socket,
        packet_t* packet)
{
}

int32_t channel_recv_tcs_ack(
        socket_t* socket,
        packet_t* packet)
{
}

int32_t channel_recv_raw(
        socket_t* socket,
        uint8_t* buffer,
        uint32_t buffer_size)
{
    debug_print("channel_recv_raw()\n");
    
    packet_t* packet = packet_buffered(socket, buffer, buffer_size);          
    
    if (!packet) { 
        goto failed; 
    }
    
    packet->source_addr = 
            ntohl(packet->socket->remote_addr.sin_addr.s_addr);
    packet->source_port = 
            ntohs(packet->socket->remote_addr.sin_port);
            
    packet->destination_addr = 
            ntohl(packet->socket->local_addr.sin_addr.s_addr);
    packet->destination_port = 
            ntohs(packet->socket->local_addr.sin_port);    
    
    packet_print(packet);    
    
    switch(packet->type) {
    case PACKET_TYPE_ACK:
        if (!window_ack_set(socket->channel->out_window, packet))
            goto failed;
            
        if (channel_recv_ack(socket, packet) < 0)
            goto failed;
        break;
    case PACKET_TYPE_DATA:
        if (channel_recv_data(socket, packet) < 0)
            goto failed;        
        break;
    case PACKET_TYPE_EACK:
        if (channel_recv_eack(socket, packet) < 0)
            goto failed;    
        break;
    case PACKET_TYPE_NULL:
        if (channel_recv_null(socket, packet) < 0)
            goto failed;    
        break;
    case PACKET_TYPE_RESET:
        if (channel_recv_reset(socket, packet) < 0)
            goto failed;    
        break;
    case PACKET_TYPE_RESET_ACK:
        if (channel_recv_reset_ack(socket, packet) < 0)
            goto failed;    
        break;
    case PACKET_TYPE_SYN:
        if (channel_recv_syn(socket, packet) < 0)
            goto failed;        
        break;
    case PACKET_TYPE_SYN_ACK:
        if (!window_ack_set(socket->channel->out_window, packet))
            goto failed;
            
        if (channel_recv_syn_ack(socket, packet) < 0)
            goto failed;            
        break;
    case PACKET_TYPE_TCS:
        if (channel_recv_tcs(socket, packet) < 0)
            goto failed;
        break;
    case PACKET_TYPE_TCS_ACK:
        if (channel_recv_tcs_ack(socket, packet) < 0)
            goto failed;    
        break;
    default:        
        goto failed;        
    }    


    debug_print("channel_recv_raw() succeed\n");
    return RUDP_SOCKET_SUCCESS; 

failed:
    //rudp_channel_retransmit(socket);
    
    debug_print("channel_recv_raw() failed\n");
    return RUDP_SOCKET_ERROR;    
}

hash_node_t* channel_waiting(
        socket_t* socket) 
{
    hash_node_t* hash_node;
    HASH_FIND(hh, socket->waiting_hash, &socket->remote_addr, 
            sizeof(struct sockaddr_in), hash_node);
    return hash_node;
}

hash_node_t* channel_established(
        socket_t* socket) 
{
    hash_node_t* hash_node;
    HASH_FIND(hh, socket->established_hash, &socket->remote_addr, 
            sizeof(struct sockaddr_in), hash_node);
    return hash_node;
}
