#include "channel.h"

rudp_socket_t* rudp_channel(rudp_socket_t* socket)
{
    printf("rudp_channel()\n");
    
    rudp_socket_t* new_socket = NULL;
    rudp_channel_t* channel = calloc(1, sizeof(rudp_channel_t));
    
    channel->timer_trans_state = rudp_channel_timer(socket);
    channel->timer_cum_ack = rudp_channel_timer(socket);
    channel->timer_null = rudp_channel_timer(socket);
    //channel->timer_retrans = rudp_channel_timer(socket);       
    
    if (socket->options.state == STATE_LISTEN) {                
        rudp_options_t* internal_options = rudp_options();
        internal_options->internal = true;
        internal_options->parent = socket;                                
        
        new_socket = rudp_socket(internal_options);
        new_socket->options.state = STATE_LISTEN;
        
        memcpy(new_socket->options.conn, socket->options.conn, 
                SYN_PACKET_HEADER_LENGTH);        
                
        rudp_hash_node_t* hash_node = calloc (1, sizeof(rudp_hash_node_t));
        hash_node->key = socket->remote_addr;
        hash_node->value = new_socket;                
        
        HASH_ADD(hh, socket->waiting_hash, key, 
                sizeof(struct sockaddr_in), hash_node);            
        
        rudp_options_free(internal_options);
    } else {
        new_socket = socket;
    }
    
    new_socket->local_addr = socket->local_addr;
    new_socket->remote_addr = socket->remote_addr;
    new_socket->socket_fd = socket->socket_fd;
    new_socket->channel = channel;
    
    printf("rudp_channel() succeed\n");
    return new_socket;
}

void rudp_options_free(
        rudp_options_t* options)
{
    if (options) {
        if (options->conn)
            free(options->conn);
        free(options);
    }
}

void rudp_channel_timer_handler(rudp_channel_timer_t* timer)
{
    printf("rudp_channel_timer_handler() triggered\n");
}

void rudp_channel_timer_close(
        rudp_channel_timer_t* timer)
{
    timer_delete(timer->timer);
    free(timer->event);
    free(timer);
}

rudp_channel_timer_t* rudp_channel_timer(
        rudp_socket_t* socket)
{
    rudp_channel_timer_t* timer = calloc(1, sizeof(rudp_channel_timer_t));
    timer->event = calloc(1, sizeof(struct sigevent));
    timer->event->sigev_notify = SIGEV_SIGNAL;        
    timer->event->sigev_signo = RUDP_SOCKET_SIGNAL;
    timer->event->sigev_value.sival_ptr = timer;

    timer_create(CLOCK_REALTIME, timer->event, &timer->timer);
    
    return timer;
}

void rudp_channel_deattach(
        rudp_socket_t* socket)
{    
    if (socket->options.state == STATE_LISTEN) { 
        rudp_channel_deattach_node(socket);
        rudp_close(socket, false);
    }
}

void rudp_channel_deattach_node(
        rudp_socket_t* socket)
{
    rudp_hash_node_t* hash_node;
    HASH_FIND(hh, socket->options.parent->waiting_hash, 
        &socket->remote_addr, sizeof(struct sockaddr_in), 
        hash_node);
        
    if (hash_node) {
        HASH_DELETE(hh, socket->options.parent->waiting_hash, hash_node); 
        free(hash_node);
    }

    HASH_FIND(hh, socket->options.parent->established_hash,
        &socket->remote_addr, sizeof(struct sockaddr_in),
        hash_node);

    if (hash_node) {
        HASH_DELETE(hh, socket->options.parent->established_hash, hash_node);
        free(hash_node);
    }
}

int32_t rudp_channel_handshake(
        rudp_socket_t* socket, 
        uint8_t* buffer, 
        uint32_t buffer_size)
{
    rudp_packet_t* packet;
    
    /*
    if (socket->options.state == STATE_LISTEN) {
        
        if (rudp_packet_check_type(buffer, buffer_size) 
                != PACKET_TYPE_SYN) { goto failed; }
                
        packet = rudp_packet(PACKET_TYPE_SYN, socket, buffer, buffer_size);
        
        if (!packet) { goto failed; }

        rudp_channel_negotiate(
                socket, &socket->options,
                ((rudp_syn_packet_t*) (packet->buffer))->aux_header);

        rudp_packet_free(packet);
        
        packet = rudp_packet(PACKET_TYPE_SYN_ACK, socket, 0, 0);
        return rudp_channel_send_packet(socket, packet);
     }     
     
     else if (socket->options.state == STATE_SYN_SENT) {        
        if (rudp_packet_check_type(buffer, buffer_size) != PACKET_TYPE_SYN) {
            goto failed;
        }         
     }

     else if (socket->options.state == STATE_SYN_RECEIVED) {        
        if (rudp_packet_check_type(buffer, buffer_size) != PACKET_TYPE_SYN) {
            goto failed;
        }         
     }
     */
     
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


int32_t rudp_channel_send_raw(
        rudp_socket_t* socket,
        uint8_t* buffer,
        uint32_t buffer_size)
{
    printf("rudp_channel_send_raw()\n");
    
    if (sendto(socket->socket_fd, buffer, buffer_size, 0, 
            (struct sockaddr *)& socket->remote_addr, 
            sizeof(socket->remote_addr)) < 0)
        goto failed;
        
succeed:
    printf("rudp_channel_send_raw() succeed\n");
    return RUDP_SOCKET_SUCCESS;
failed:
    printf("rudp_channel_send_raw() failed\n");
    return RUDP_SOCKET_ERROR;            
}

int32_t rudp_channel_send_ack(
        rudp_socket_t* socket, 
        rudp_packet_t* packet)
{
    printf("rudp_channel_send_ack()\n");
    
    if (!packet->ack) {
        switch(packet->type) {
        case PACKET_TYPE_SYN:
            packet->ack = rudp_packet(PACKET_TYPE_SYN_ACK, socket);
            break;
        case PACKET_TYPE_DATA:            
        case PACKET_TYPE_EACK:            
        case PACKET_TYPE_NULL:
            packet->ack = rudp_packet(PACKET_TYPE_ACK, socket);
            break;
        case PACKET_TYPE_RESET:
            packet->ack = rudp_packet(PACKET_TYPE_RESET_ACK, socket);
            break;
        case PACKET_TYPE_TCS:
            packet->ack = rudp_packet(PACKET_TYPE_TCS_ACK, socket);
            break;
        default:
            goto failed;
        }
        
        if (!packet->ack) { goto failed; }
        
        if (rudp_channel_send_raw(socket, 
                packet->ack->buffer, packet->ack->buffer_size) < 0)
            goto failed;
    }

succeed:
    printf("rudp_channel_send_ack() succeed\n");
    return rudp_channel_send_raw(socket, 
            packet->ack->buffer, packet->ack->buffer_size);
failed:
    printf("rudp_channel_send_ack() failed\n");
    return RUDP_SOCKET_ERROR;
}

int32_t rudp_channel_send_packet(
        rudp_socket_t* socket, 
        rudp_packet_t* packet)
{
    packet->transmission_time = rudp_timestamp();    
    queue_enqueue(socket->out_buffer, packet);
    
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
        
    //queue_dequeue(socket->in_buffer);
}

void rudp_channel_retransmit(
        rudp_socket_t* socket)
{
    
}

int32_t rudp_channel_recv_eack(
        rudp_socket_t* socket,
        rudp_packet_t* packet)
{
}

int32_t rudp_channel_recv_reset_ack(
        rudp_socket_t* socket,
        rudp_packet_t* packet)
{
}

int32_t rudp_channel_recv_reset(
        rudp_socket_t* socket,
        rudp_packet_t* packet)
{
}

int32_t rudp_channel_recv_data(
        rudp_socket_t* socket,
        rudp_packet_t* packet)
{
}

int32_t rudp_channel_recv_null(
        rudp_socket_t* socket,
        rudp_packet_t* packet)
{
}

int32_t rudp_channel_recv_syn(
        rudp_socket_t* socket,
        rudp_packet_t* packet)
{
    printf("rudp_channel_recv_syn()\n");
    
    if (socket->options.state != STATE_LISTEN)
        goto failed;

    rudp_syn_packet_header_t* syn_header =  packet->buffer;    
    socket->options.conn->identifier = syn_header->identifier;
    socket->options.conn->option_flags = syn_header->option_flags;
    
    socket->options.conn->max_auto_reset = 
            MIN(socket->options.conn->max_auto_reset, 
            syn_header->max_auto_reset);
    
    socket->options.conn->max_cum_ack = 
            MIN(socket->options.conn->max_cum_ack, 
            syn_header->max_cum_ack);    
 
    socket->options.conn->max_out_segments = 
            MIN(socket->options.conn->max_out_segments, 
            syn_header->max_out_segments);    
 
    socket->options.conn->max_retransmissions = 
            MIN(socket->options.conn->max_retransmissions, 
            syn_header->max_retransmissions);    
 
    socket->options.conn->max_out_sequences = 
            MIN(socket->options.conn->max_out_sequences, 
            syn_header->max_out_sequences);   

    socket->options.conn->timeout_cum_ack = 
            MIN(socket->options.conn->timeout_cum_ack, 
            syn_header->timeout_cum_ack);   

    socket->options.conn->timeout_null = 
            MIN(socket->options.conn->timeout_null, 
            syn_header->timeout_null);  
            
    socket->options.conn->timeout_retransmission = 
            MIN(socket->options.conn->timeout_retransmission, 
            syn_header->timeout_retransmission);             

    socket->options.conn->timeout_trans_state = 
            MIN(socket->options.conn->timeout_trans_state, 
            syn_header->timeout_trans_state);                
    
    if (rudp_channel_send_ack(socket, packet) < 0)
        goto failed;

    socket->options.state = STATE_SYN_RECEIVED;
    
succeed:   
    printf("rudp_channel_recv_syn() succeed\n");
    return RUDP_SOCKET_SUCCESS; 
failed:    
    printf("rudp_channel_recv_syn() failed\n");
    return RUDP_SOCKET_ERROR;
}

int32_t rudp_channel_recv_syn_ack(
        rudp_socket_t* socket,
        rudp_packet_t* packet)
{
}

int32_t rudp_channel_recv_ack(
        rudp_socket_t* socket,
        rudp_packet_t* packet)
{
}

int32_t rudp_channel_recv_tcs(
        rudp_socket_t* socket,
        rudp_packet_t* packet)
{
}

int32_t rudp_channel_recv_tcs_ack(
        rudp_socket_t* socket,
        rudp_packet_t* packet)
{
}

int32_t rudp_channel_recv_raw(
        rudp_socket_t* socket,
        uint8_t* buffer,
        uint32_t buffer_size)
{
    printf("rudp_channel_recv_raw()\n");
    
    rudp_packet_t* packet = rudp_buffered_packet(socket, buffer, buffer_size);    
    
    if (!packet) { goto failed; }
    
    switch(packet->type) {
    case PACKET_TYPE_ACK:
        if (rudp_channel_recv_ack(socket, packet) < 0)
            goto failed;    
        break;
    case PACKET_TYPE_DATA:
        if (rudp_channel_recv_data(socket, packet) < 0)
            goto failed;    
        break;
    case PACKET_TYPE_EACK:
        if (rudp_channel_recv_eack(socket, packet) < 0)
            goto failed;    
        break;
    case PACKET_TYPE_NULL:
        if (rudp_channel_recv_null(socket, packet) < 0)
            goto failed;    
        break;
    case PACKET_TYPE_RESET:
        if (rudp_channel_recv_reset(socket, packet) < 0)
            goto failed;    
        break;
    case PACKET_TYPE_RESET_ACK:
        if (rudp_channel_recv_reset_ack(socket, packet) < 0)
            goto failed;    
        break;
    case PACKET_TYPE_SYN:
        if (rudp_channel_recv_syn(socket, packet) < 0)
            goto failed;
        break;
    case PACKET_TYPE_SYN_ACK:
        if (rudp_channel_recv_syn_ack(socket, packet) < 0)
            goto failed;    
        break;
    case PACKET_TYPE_TCS:
        if (rudp_channel_recv_tcs(socket, packet) < 0)
            goto failed;    
        break;
    case PACKET_TYPE_TCS_ACK:
        if (rudp_channel_recv_tcs_ack(socket, packet) < 0)
            goto failed;    
        break;
    default:        
        goto failed;        
    }    

succeed:   
    printf("rudp_channel_recv_raw() succeed\n");
    return RUDP_SOCKET_SUCCESS; 
failed:
    rudp_channel_retransmit(socket);
    
    printf("rudp_channel_recv_raw() failed\n");
    return RUDP_SOCKET_ERROR;    
}

void rudp_channel_negotiate(
        rudp_socket_t* socket,
        rudp_options_t* server,
        rudp_syn_packet_header_t* client)
{
    /*
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
    */    
}

int32_t rudp_channel_close(
        rudp_socket_t* socket)
{
    if (socket && socket->channel) {
        
        //rudp_channel_timer_close(socket->channel->timer_retrans);
        rudp_channel_timer_close(socket->channel->timer_cum_ack);
        rudp_channel_timer_close(socket->channel->timer_null);
        rudp_channel_timer_close(socket->channel->timer_trans_state);   
         
        free(socket->channel);
    }
}

rudp_hash_node_t* rudp_channel_waiting(
        rudp_socket_t* socket) 
{
    rudp_hash_node_t* hash_node;
    HASH_FIND(hh, socket->waiting_hash, &socket->remote_addr, 
            sizeof(struct sockaddr_in), hash_node);
    return hash_node;
}

rudp_hash_node_t* rudp_channel_established(
        rudp_socket_t* socket) 
{
    rudp_hash_node_t* hash_node;
    HASH_FIND(hh, socket->established_hash, &socket->remote_addr, 
            sizeof(struct sockaddr_in), hash_node);
    return hash_node;
}