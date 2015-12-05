#include "packet.h"

rudp_packet_t* rudp_packet(
        rudp_packet_type_t type, 
        rudp_socket_t* socket,
        uint8_t* buffer, 
        uint32_t buffer_size)
{        
    rudp_packet_t* packet = NULL;
    
    if (!socket) { goto cleanup; }
    
    if (type != PACKET_TYPE_UNKNOWN) {
        packet = calloc(1, sizeof(rudp_packet_t));
        
        packet->buffer_size = PACKET_HEADER_LENGTH;
        packet->buffer = calloc(packet->buffer_size, sizeof(uint8_t));
        packet->type = type;
        
        if (rudp_packet_set_header(packet, buffer, buffer_size) < 0) {
            goto cleanup;
        }    
    
        if (type == PACKET_TYPE_SYN || type == PACKET_TYPE_SYN_ACK) {
            packet->buffer_size = SYN_PACKET_LENGTH;
            packet->buffer = realloc(packet->buffer, SYN_PACKET_LENGTH);                
            
            if (rudp_packet_set_syn_header(
                    socket, packet, buffer, buffer_size) < 0) {
                goto cleanup;
            }
            
            return packet;
        }
    }    
    
cleanup:
    if (packet) {
        if (packet->buffer)
            free(packet->buffer);
        free(packet);
    }
    
    printf("rudp_packet() failed\n");
    return NULL;    
}

rudp_packet_type_t rudp_packet_check_type(
        uint8_t* buffer,
        uint32_t buffer_size)
{
    if (!buffer || buffer_size < PACKET_HEADER_LENGTH)
        return PACKET_TYPE_UNKNOWN;
            
    if (*buffer == PACKET_FLAG_SYN)
        return PACKET_TYPE_SYN;        
    else if (*buffer == (PACKET_FLAG_ACK | PACKET_FLAG_SYN))
        return PACKET_TYPE_SYN_ACK;
    else if (*buffer == PACKET_FLAG_ACK)
        return PACKET_TYPE_ACK;
    else if (*buffer == (PACKET_FLAG_ACK | PACKET_FLAG_DATA))
        return PACKET_TYPE_DATA;
    else if (*buffer == (PACKET_FLAG_ACK | PACKET_FLAG_EACK))
        return PACKET_TYPE_EACK;
    else if (*buffer == (PACKET_FLAG_ACK | PACKET_FLAG_NUL))
        return PACKET_TYPE_NULL;
    else if (*buffer == PACKET_FLAG_RST)
        return PACKET_TYPE_RESET;
    else if (*buffer == PACKET_FLAG_TCS)
        return PACKET_TYPE_TCS;
    else       
        return PACKET_TYPE_UNKNOWN;
}

int32_t rudp_packet_set_header(
        rudp_packet_t* packet,
        uint8_t* buffer,
        uint32_t buffer_size)
{
    rudp_packet_header_t* header = packet->buffer; 

    if (buffer) {
        if (buffer_size < PACKET_HEADER_LENGTH || 
                packet->type != rudp_packet_check_type(buffer, buffer_size) ||
                !rudp_packet_check_checksum(buffer, PACKET_HEADER_LENGTH)) {
            
            printf("rudp_packet_set_header() failed\n");
            return RUDP_SOCKET_ERROR;
        }
        
        memcpy(packet->buffer, buffer, PACKET_HEADER_LENGTH);
                
        goto finish;
    }
   
    header->header_length = PACKET_HEADER_LENGTH;
    
    switch(packet->type) {
    case PACKET_TYPE_ACK:
        header->flags = PACKET_FLAG_ACK;
        break;        
    case PACKET_TYPE_SYN:
        header->flags = PACKET_FLAG_SYN;
        break;        
    case PACKET_TYPE_SYN_ACK:
        header->flags = PACKET_FLAG_ACK | PACKET_FLAG_SYN;
        break;        
    case PACKET_TYPE_DATA:
        header->flags = PACKET_FLAG_ACK | PACKET_FLAG_DATA;
        break;        
    case PACKET_TYPE_EACK:
        header->flags = PACKET_FLAG_ACK | PACKET_FLAG_EACK;
        break;        
    case PACKET_TYPE_NULL:
        header->flags = PACKET_FLAG_ACK | PACKET_FLAG_NUL;
        break;        
    case PACKET_TYPE_RESET:
        header->flags = PACKET_FLAG_RST;
        break;        
    case PACKET_TYPE_TCS:
        header->flags = PACKET_FLAG_TCS;
        break;
    }
    
    rudp_packet_add_checksum(packet);
    
finish:
    printf("rudp_packet_set_header() succeed\n");
    return RUDP_SOCKET_SUCCESS;    
}

int32_t rudp_packet_set_syn_header(
        rudp_socket_t* socket,
        rudp_packet_t* packet,
        uint8_t* buffer,
        uint32_t buffer_size)
{
    rudp_syn_packet_header_t* header = 
            packet->buffer + PACKET_HEADER_LENGTH;
    
    if (buffer) {
        if (buffer_size < SYN_PACKET_LENGTH ||
                rudp_packet_check_checksum(buffer, SYN_PACKET_LENGTH)) {
                    
            printf("rudp_packet_set_syn_header() failed\n");
            return RUDP_SOCKET_ERROR;
        }
        
        memcpy(packet->buffer + PACKET_HEADER_LENGTH,
                buffer, SYN_PACKET_HEADER_LENGTH);
                
        goto finish;
    }
    
    header->max_auto_reset = socket->options.max_auto_reset;
    header->max_cum_ack = socket->options.max_cum_ack;
    header->max_out_segments = socket->options.max_out_segments;
    header->max_out_sequences = socket->options.max_out_sequences;
    header->max_retransmissions = socket->options.max_retransmissions;
    header->max_segment_size = socket->options.max_segment_size;
    
    header->option_flags = socket->options.flags;
    header->version = socket->options.version;
    
    header->timeout_cum_ack = socket->options.timeout_cum_ack;
    header->timeout_null = socket->options.timeout_null;
    header->timeout_retransmission = socket->options.timeout_retransmission;
    header->timeout_trans_state = socket->options.timeout_trans_state;
    
    rudp_packet_add_checksum(packet);

finish:
    printf("rudp_packet_set_syn_header() succeed\n");
    return RUDP_SOCKET_SUCCESS;
}

rudp_packet_t* rudp_packet_from_buffer(
        rudp_packet_type_t type, 
        rudp_socket_t* socket,
        uint8_t* buffer, 
        uint32_t buffer_size)
{

}

int32_t rudp_packet_free(
        rudp_packet_t* packet)
{
    
}

int32_t rudp_packet_add_checksum(
        rudp_packet_t* packet)
{
    
}

uint32_t rudp_packet_checksum(
        uint8_t* buffer, 
        uint32_t buffer_size)
{
    
}

bool rudp_packet_check_checksum(
        uint8_t* buffer, 
        uint32_t buffer_size)
{
    
}

void rudp_packet_timeout(
        rudp_channel_timer_t* timer,
        uint32_t interval)
{
    timer->timer_value.it_value.tv_sec = 0;
    timer->timer_value.it_value.tv_nsec = interval * 1000;
    timer->timer_value.it_interval.tv_sec = 0;
    timer->timer_value.it_interval.tv_nsec = 0;

    if (timer_settime(timer->timer, 0, &timer->timer_value, NULL) != 0) {
        printf("rudp_packet_timeout() failed\n");
    }
}