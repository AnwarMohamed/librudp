#include "channel.h"


socket_t* channel(socket_t* socket)
{
    debug_print("channel()\n");
    
    socket_t* new_socket = NULL;
    hash_node_t* hash_node = NULL;
    socket_t* hash_socket = NULL;
    
    channel_t* channel = calloc(1, sizeof(channel_t));    
    
    if (socket->type == TYPE_SERVER) {                                                            
        new_socket = rudp_socket(0);
        new_socket->type == TYPE_SERVER;
        
        new_socket->options->state = STATE_LISTEN;
        new_socket->options->internal = true;
        new_socket->options->parent = socket;  
        
        memcpy(new_socket->options->conn, socket->options->conn, 
                SYN_PACKET_HEADER_LENGTH);                          
    } else {
        new_socket = socket;                
    }
    
    new_socket->local_addr = socket->local_addr;
    new_socket->remote_addr = socket->remote_addr;
    new_socket->socket_fd = socket->socket_fd;
    new_socket->channel = channel;  
    
    
    channel->timer_trans_state = utimer(new_socket, 0, TIMER_TRANS_STATE);
    channel->timer_cum_ack = utimer(new_socket, 0, TIMER_CUM_ACK);
    channel->timer_null = utimer(new_socket, 0, TIMER_NULL);    
    
    channel->sequence = rudp_sequence(new_socket);
    
    channel->out_window = window(socket->options->window_type, 
            socket->options->conn->max_window_size);
    channel->in_window = window(socket->options->window_type, 
            socket->options->conn->max_window_size);        
    
    hash_node = calloc (1, sizeof(hash_node_t));
    hash_node->key = new_socket->remote_addr;
    hash_node->value = new_socket;

    hash_socket = new_socket->options->internal ? 
            new_socket->options->parent :new_socket;
    
    HASH_ADD(hh, hash_socket->waiting_hash, key, 
            sizeof(struct sockaddr_in), hash_node);        
    
    success_print("channel() succeed\n");
    return new_socket;
}

int32_t channel_free(
        socket_t* socket)
{
    debug_print("channel_free()\n");
    
    if (socket && socket->channel) {

        if (socket->channel->timer_cum_ack) {
            utimer_free(socket->channel->timer_cum_ack);
            socket->channel->timer_cum_ack = 0;                
        }            
        
        if (socket->channel->timer_null) {
            utimer_free(socket->channel->timer_null);
            socket->channel->timer_null = 0;        
        }
        
        if (socket->channel->timer_trans_state) {
            utimer_free(socket->channel->timer_trans_state);
            socket->channel->timer_trans_state = 0;        
        }
        
        if (socket->channel->out_window) {
            window_free(socket->channel->out_window);
            socket->channel->out_window = 0;        
        }
        
        if (socket->channel->in_window) {
            window_free(socket->channel->in_window);
            socket->channel->in_window = 0;        
        }
        
        free(socket->channel);
        socket->channel = 0;      
    }
    
    success_print("channel_free() succeed\n");
    return RUDP_SOCKET_SUCCESS;
}


void channel_timer_handler(utimer_t* timer)
{
    debug_print("channel_timer_handler() :%p\n", timer);        
    
    if (timer) {
        
        socket_t* socket = timer->socket;        
        channel_t* channel = timer->socket->channel;
        
        if (timer->type == TIMER_RETRANS) {                    
            
            if (timer->packet->needs_ack) {            
                packet_timeout(timer);                
            } else {
                utimer_set(timer, 0);
            }                
            
        } else if (timer->type == TIMER_NULL) {
            
            packet_t* null_packet = packet(PACKET_TYPE_NULL, socket, true);
            channel->sequence++;
                        
            window_out_enqueue(channel->out_window, null_packet);            
                    
        } else if (timer->type == TIMER_CUM_ACK) {
            
            
            
            if (!channel->in_window->size) {
                utimer_set(timer, 0);
            } else {
                
                window_lock(channel->in_window);
                
                window_t* window = channel->in_window;
                queue_node_t* head_node = window->head;
                packet_t* packet, * prev_packet;
                
                while (head_node && head_node != window->tail) {                    
                    
                    if (window_sequenced(channel->in_window, 
                        head_node->prev, head_node)) {
                        
                        //printf("sequenced\n");
                        channel_send_ack(socket, head_node->data);
                    }
                    
                    head_node = head_node->next;
                }
                
                window_unlock(channel->in_window);
            }
        }
    }
    
    success_print("channel_timer_handler() succeed\n"); 
}

void channel_timeout(
        socket_t* socket)
{
    debug_print("channel_timeout()\n");
    
    if (socket) {
        rudp_close(socket, false);
    }
    
    success_print("channel_timeout() succeed\n");
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
*/

void channel_deattach_node(
        socket_t* socket)
{
    hash_node_t* hash_node;
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

int32_t channel_handshake_start(
    socket_t* socket)
{
    debug_print("channel_handshake_start()\n");        
    
    packet_t* syn_packet = packet(PACKET_TYPE_SYN, socket, true);    
    
    if (!syn_packet) { 
        goto failed; 
    }

    window_out_enqueue(socket->channel->out_window, syn_packet);

    socket->type = TYPE_CLIENT;
    socket->options->state = STATE_SYN_SENT; 

    socket->channel->sequence++;
    
    success_print("channel_handshake_start() succeed\n");
    return RUDP_SOCKET_SUCCESS;
    
failed:
    error_print("channel_handshake_start() failed\n");
    
    packet_free(syn_packet);
    return RUDP_SOCKET_ERROR;    
}

int32_t channel_send(
        socket_t* socket,
        uint8_t* buffer,
        uint32_t buffer_size)
{   
    if (!buffer_size) {
        return buffer_size;
    }
    
    uint16_t segment_size = 
            socket->options->peer->max_segment_size - BASE_PACKET_LENGTH;
    uint32_t segment_count = buffer_size / segment_size;    
            
    packet_t* data_packet;
    
    window_autocommit_set(socket->channel->out_window, false);
    
    for (uint32_t i= 0; i<segment_count; i++) {
        data_packet = packet(PACKET_TYPE_DATA, socket, true);
        packet_data_set(data_packet, 
                buffer + i*segment_size,
                segment_size, true);
        
        window_out_enqueue(socket->channel->out_window, data_packet);        
        socket->channel->sequence += data_packet->data_buffer_size;
    }    
    
    if (buffer_size % segment_size) {
        data_packet = packet(PACKET_TYPE_DATA, socket, true);
        packet_data_set(data_packet, 
                buffer + buffer_size - (buffer_size % segment_size),
                buffer_size % segment_size, true);
        
        window_out_enqueue(socket->channel->out_window, data_packet);         
        socket->channel->sequence += data_packet->data_buffer_size;
    }
    
    window_out_commit(socket->channel->out_window);
    window_autocommit_set(socket->channel->out_window, true);
    
    return buffer_size;
}

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
        
    success_print("channel_send_raw() succeed\n");
    return RUDP_SOCKET_SUCCESS;
    
failed:
    error_print("channel_send_raw() failed\n");
    
    if (socket) {
        channel_free(socket);
    }
    
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
            recv_packet->ack = packet(PACKET_TYPE_SYN_ACK, socket, true);
            break;
        case PACKET_TYPE_SYN_ACK:        
        case PACKET_TYPE_DATA:
        case PACKET_TYPE_EACK:
        case PACKET_TYPE_NULL:
            recv_packet->ack = packet(PACKET_TYPE_ACK, socket, true);
            break;
        case PACKET_TYPE_RESET:
            recv_packet->ack = packet(PACKET_TYPE_RESET_ACK, socket, true);
            break;
        case PACKET_TYPE_TCS:
            recv_packet->ack = packet(PACKET_TYPE_TCS_ACK, socket, true);
            break;
        default:
            goto failed; 
        } 
        
        if (!recv_packet->ack) { 
            goto failed; 
        }
    }    
    
    
    if (recv_packet->ack->type == PACKET_TYPE_SYN_ACK) {
        recv_packet->ack->header->sequence =  socket->channel->sequence;
    } else {
        recv_packet->ack->header->sequence = recv_packet->header->acknowledge;
    }
     
    if (recv_packet->type == PACKET_TYPE_DATA) {
        recv_packet->ack->header->acknowledge = 
                recv_packet->header->sequence + recv_packet->data_buffer_size;
       socket->channel->acknowledge = recv_packet->ack->header->acknowledge;
    } else {
        recv_packet->ack->header->acknowledge = recv_packet->header->sequence + 1;
    }

    
    socket->channel->sequence = recv_packet->ack->header->sequence;    
    socket->channel->acknowledge = recv_packet->ack->header->acknowledge;

    packet_checksum_add(recv_packet->ack);
    
    if (recv_packet->ack->needs_ack) {        
        window_out_enqueue(socket->channel->out_window, recv_packet->ack);
    } else {
        if (channel_send_packet(recv_packet->ack) <0)
            goto failed;
        
        if (recv_packet->type != PACKET_TYPE_DATA) {
            packet_free(recv_packet);            
        }
    }

    //if (!socket->channel->in_window->size) {
    //    utimer_set(socket->channel->timer_cum_ack,
    //            socket->options->conn->timeout_cum_ack);
    //} 

    success_print("channel_send_ack() succeed\n");
    return RUDP_SOCKET_SUCCESS;
    
failed:
    error_print("channel_send_ack() failed\n");
    return RUDP_SOCKET_ERROR;
}


int32_t channel_send_packet( 
        packet_t* packet)
{                    
    socket_t* socket = packet->socket;        
    
    packet->destination_addr = ntohl(socket->remote_addr.sin_addr.s_addr);
    packet->destination_port = ntohs(socket->remote_addr.sin_port);
    packet->source_addr = ntohl(socket->local_addr.sin_addr.s_addr);
    packet->source_port = ntohs(socket->local_addr.sin_port);
    
    if (packet->needs_ack) {
        utimer_set(packet->timer_retrans, 
                socket->options->conn->timeout_retransmission);
    } else {
        if (packet->counter_retrans > 
                socket->options->conn->max_retransmissions) {
            rudp_close(socket, false);
        } else {
            if (packet->transmission_time)
                packet->counter_retrans++;
        }
    }
    
    packet->transmission_time = rudp_timestamp();
    
    if (packet->socket->type == TYPE_CLIENT) {
        utimer_set(socket->channel->timer_null, 
                socket->options->conn->timeout_null);
    }     
    
    packet_print(packet);
    
    return channel_send_raw(socket, packet->buffer, packet->buffer_size);
}


int32_t channel_recv(
        socket_t* socket,
        uint8_t* buffer,
        uint32_t buffer_size)
{
    packet_t* packet = window_in_dequeue(
            socket->channel->in_window, true);
    
    if (!packet) {
        goto failed;
    }
    
    memcpy(buffer, packet->data_buffer, 
            MIN(packet->data_buffer_size, buffer_size));
            
    success_print("channel_recv() succeed\n");
    return MIN(packet->data_buffer_size, buffer_size);    
    
failed:    
    error_print("channel_recv() failed\n");
    return RUDP_SOCKET_ERROR;     
}

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
    debug_print("channel_recv_data()\n");                
    
    window_in_enquque(socket->channel->in_window, packet);    

    //socket->channel->acknowledge = 
    //        packet->header->sequence + packet->data_buffer_size;
    //socket->channel->sequence = packet->header->acknowledge; 

    //if (channel_send_ack(socket, packet) < 0)
    //    return -1;

    success_print("channel_recv_data() succeed\n");
    return RUDP_SOCKET_SUCCESS;   
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

    success_print("channel_recv_null() succeed\n");
    return RUDP_SOCKET_SUCCESS; 
    
failed:    
    error_print("channel_recv_null() failed\n");
    return RUDP_SOCKET_ERROR;       
}

int32_t channel_recv_syn(
        socket_t* socket,
        packet_t* packet)
{
    debug_print("channel_recv_syn()\n");
    
    if (socket->options->state != STATE_LISTEN)
        goto failed;            
        
    socket->channel->acknowledge = packet->header->sequence;
    
    socket->options->conn->identifier = packet->syn_header->identifier;
    socket->options->conn->option_flags = packet->syn_header->option_flags;
    
    socket->options->peer->max_segment_size =
            packet->syn_header->max_segment_size;
    
    socket->options->peer->max_window_size =  
            packet->syn_header->max_window_size;        
    
    window_max_size_set(socket->channel->out_window, 
            packet->syn_header->max_window_size);
    
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

    socket->options->conn->timeout_null = 
            MIN(socket->options->conn->timeout_null, 
            packet->syn_header->timeout_null);                      

    socket->options->conn->timeout_trans_state = 
            MIN(socket->options->conn->timeout_trans_state, 
            packet->syn_header->timeout_trans_state);                
        
    if (channel_send_ack(socket, packet) < 0) {
        goto failed;        
    }
    
    socket->channel->sequence++;
    socket->options->state = STATE_SYN_RECEIVED;
      
    success_print("channel_recv_syn() succeed\n");
    return RUDP_SOCKET_SUCCESS; 
    
failed:    
    error_print("channel_recv_syn() failed\n");
    return RUDP_SOCKET_ERROR;
}


int32_t channel_recv_syn_ack(
        socket_t* socket,
        packet_t* packet)
{
    debug_print("channel_recv_syn_ack()\n");
    
    if (socket->options->state != STATE_SYN_SENT) {        
        goto failed;        
    }

    socket->channel->acknowledge = packet->header->sequence;
            
    socket->options->conn->identifier = packet->syn_header->identifier;
    socket->options->conn->option_flags = packet->syn_header->option_flags;        
    
    socket->options->peer->max_window_size = packet->syn_header->max_window_size;
    socket->options->peer->max_segment_size = packet->syn_header->max_segment_size;
    
    window_max_size_set(socket->channel->out_window, 
            packet->syn_header->max_window_size);    
    
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
    
    if (channel_send_ack(socket, packet) < 0) {
        goto failed;        
    }
    
    socket->options->state = STATE_ESTABLISHED;
    sem_post(&socket->options->state_lock);
    
    if (socket->type == TYPE_CLIENT) {
        utimer_set(socket->channel->timer_null, 
                socket->options->conn->timeout_null);
    }    
  
    success_print("channel_recv_syn_ack() succeed\n");
    return RUDP_SOCKET_SUCCESS; 
    
failed:    
    error_print("channel_recv_syn_ack() failed\n");
    return RUDP_SOCKET_ERROR;    
}


int32_t channel_recv_ack(
        socket_t* socket,
        packet_t* packet)
{
    if (socket->options->state == STATE_SYN_RECEIVED) {
        
        hash_node_t* hash_node;
        HASH_FIND(hh, 
                socket->options->parent->waiting_hash, 
                &socket->remote_addr, 
                sizeof(struct sockaddr_in), 
                hash_node);
        
        HASH_DELETE(hh, 
                socket->options->parent->waiting_hash, 
                hash_node);
                
        queue_enqueue(socket->options->parent->ready_queue, hash_node, 0);            
        
        socket->options->state = STATE_ESTABLISHED;
        
        sem_post(&socket->options->state_lock);        
        sem_post(&socket->options->parent->options->state_lock);
    }
    
    //socket->channel->acknowledge = packet->header->sequence + 1;
    //socket->channel->sequence = packet->header->acknowledge;
    
    //socket->channel->acknowledge++;
    
    return RUDP_SOCKET_SUCCESS;
}

int32_t channel_recv_tcs(
        socket_t* socket,
        packet_t* packet)
{
    return RUDP_SOCKET_SUCCESS;
}

int32_t channel_recv_tcs_ack(
        socket_t* socket,
        packet_t* packet)
{
    return RUDP_SOCKET_SUCCESS;
}

int32_t channel_recv_raw(
        socket_t* socket,
        uint8_t* buffer,
        uint32_t buffer_size)
{
    debug_print("channel_recv_raw()\n");
    
    //for (int i=0; i<buffer_size; i++) {
    //    printf("%02x ", buffer[i]);
    //}
    //printf("\n");
    
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
        if (!window_out_ack(socket->channel->out_window, packet))
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
        if (!window_out_ack(socket->channel->out_window, packet))
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


    success_print("channel_recv_raw() succeed\n");
    return RUDP_SOCKET_SUCCESS; 

failed:
    //rudp_channel_retransmit(socket);
    
    error_print("channel_recv_raw() failed\n");
    return RUDP_SOCKET_ERROR;    
}

hash_node_t* channel_waiting(
        socket_t* socket) 
{
    debug_print("channel_waiting()\n");
    
    hash_node_t* hash_node;
    HASH_FIND(hh, socket->waiting_hash, &socket->remote_addr, 
            sizeof(struct sockaddr_in), hash_node);
    return hash_node;
}

hash_node_t* channel_established(
        socket_t* socket) 
{
    debug_print("channel_established()\n");
    
    hash_node_t* hash_node;
    HASH_FIND(hh, socket->established_hash, &socket->remote_addr, 
            sizeof(struct sockaddr_in), hash_node);
    return hash_node;
}
