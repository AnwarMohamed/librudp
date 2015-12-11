#include "packet.h"


rudp_packet_t* rudp_packet(
        rudp_packet_type_t type, 
        rudp_socket_t* socket)
{        
    debug_print("rudp_packet()\n");        
    
    if (!socket) { 
        goto failed; 
    }
    
    if (type == PACKET_TYPE_UNKNOWN) {
        goto failed;
    }
    
    rudp_packet_t* new_packet = calloc(1, sizeof(rudp_packet_t));
    
    new_packet->creatation_time = rudp_timestamp();
    new_packet->type = type;
    
    if (type == PACKET_TYPE_SYN || type == PACKET_TYPE_SYN_ACK) { 
        new_packet->buffer_size = SYN_PACKET_LENGTH;
    } else {
        new_packet->buffer_size = BASE_PACKET_LENGTH;
    }
    
    new_packet->buffer = calloc(new_packet->buffer_size, sizeof(uint8_t));        

    if (rudp_packet_set_header(socket, new_packet) < 0) { 
        goto failed; 
    }

    if (type == PACKET_TYPE_SYN || type == PACKET_TYPE_SYN_ACK) {            
        if (rudp_packet_set_syn_header(socket, new_packet) < 0) {
            goto failed;
        }                        
    }
    
    switch(type) {
    case PACKET_TYPE_DATA:
    case PACKET_TYPE_NULL:
    case PACKET_TYPE_RESET:
    case PACKET_TYPE_SYN:
    case PACKET_TYPE_TCS:
        new_packet->needs_ack = true;
    }

    debug_print("rudp_packet() succeed\n");
    return new_packet;
    
failed:
    if (new_packet)
        rudp_packet_free(new_packet);    
        
    debug_print("rudp_packet() failed\n");
    return 0;    
}


int32_t rudp_packet_set_header(
        rudp_socket_t* socket,
        rudp_packet_t* packet)
{
    debug_print("rudp_packet_set_header()\n");
    
    rudp_packet_header_t* header = (rudp_packet_header_t*) packet->buffer; 
    header->header_length = BASE_PACKET_LENGTH;
    header->acknowledge = socket->channel->acknowledge;
    header->sequence = socket->channel->sequence;
    
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
    
succeed:
    debug_print("rudp_packet_set_header() succeed\n");
    return RUDP_SOCKET_SUCCESS;    
}

int32_t rudp_packet_set_syn_header(
        rudp_socket_t* socket,
        rudp_packet_t* packet)
{   
    debug_print("rudp_packet_set_syn_header()\n");
        
    if (packet->type == PACKET_TYPE_SYN)
        memcpy(packet->buffer + BASE_PACKET_LENGTH, 
                socket->options->conn, SYN_PACKET_HEADER_LENGTH);
        
    else if (packet->type == PACKET_TYPE_SYN_ACK)
        memcpy(packet->buffer + BASE_PACKET_LENGTH, 
                socket->options->conn, SYN_PACKET_HEADER_LENGTH);                    
                
    else 
        goto failed;
    
    rudp_packet_add_checksum(packet);

    debug_print("rudp_packet_set_syn_header() succeed\n");
    return RUDP_SOCKET_SUCCESS;
    
failed:
    debug_print("rudp_packet_set_syn_header() failed\n");
    return RUDP_SOCKET_ERROR;    
}


int32_t rudp_packet_free(
        rudp_packet_t* packet)
{
    if (packet) {
        if (packet->buffer) {
            free(packet->buffer);
        }
        
        free(packet);
    }
}

int32_t rudp_packet_add_checksum(
        rudp_packet_t* packet)
{
    rudp_packet_header_t* header = (rudp_packet_header_t*) packet->buffer;
    
    switch (packet->type) {
    case PACKET_TYPE_SYN:
    case PACKET_TYPE_SYN_ACK:
        return (header->checksum = rudp_packet_checksum(
                packet->buffer, SYN_PACKET_LENGTH));
    default:
        return (header->checksum = rudp_packet_checksum(
                packet->buffer, BASE_PACKET_LENGTH));  
    }
}

uint16_t rudp_packet_checksum(
        uint8_t* buffer, 
        uint32_t buffer_size)
{
    uint16_t* addr_ = (uint16_t*) buffer;
    uint16_t count_ = buffer_size;
    
    uint16_t checksum_bkup = addr_[1];   
    addr_[1] = 0x0;

    register uint32_t sum = 0x0;

    while(count_ > 1)  {
        sum += * addr_++;
        count_ -= 2;
    }
    
    if(count_ > 0)
        sum += * (uint8_t *) addr_;

    while (sum >>16)
        sum = (sum & 0xffff) + (sum >> 16);

    addr_ = (uint16_t*) buffer;
    addr_[1] = checksum_bkup;        
    
    return (uint16_t) ~sum;    
}

bool rudp_packet_check_checksum(
        uint8_t* buffer, 
        uint32_t buffer_size)
{      
    return ((uint16_t*) buffer)[1] == 
            rudp_packet_checksum(buffer, buffer_size);
}

/*
void rudp_packet_timeout(
        rudp_channel_timer_t* timer,
        uint32_t interval)
{
    timer->timer_value.it_value.tv_sec = 0;
    timer->timer_value.it_value.tv_nsec = interval * 1000;
    timer->timer_value.it_interval.tv_sec = 0;
    timer->timer_value.it_interval.tv_nsec = 0;

    if (timer_settime(timer->timer, 0, &timer->timer_value, 0) != 0) {
        debug_print("rudp_packet_timeout() failed\n");
    }
}
*/

rudp_packet_t* rudp_buffered_packet(
        rudp_socket_t* socket, 
        uint8_t* buffer, 
        uint32_t buffer_size)
{    
    debug_print("rudp_buffered_packet()\n");
    
    rudp_packet_type_t type = rudp_packet_check_type(buffer, buffer_size);
    
    if (type == PACKET_TYPE_UNKNOWN) { 
        goto failed; 
    }
    
    rudp_packet_t* packet = rudp_packet(type, socket);
    
    if (!packet) { 
        goto failed; 
    }
    
    rudp_packet_header_t *header = (rudp_packet_header_t*) buffer,
        *packet_header = (rudp_packet_header_t*) packet->buffer;
    
    rudp_syn_packet_header_t *packet_syn_header  = 
            (rudp_syn_packet_header_t*) (packet->buffer + BASE_PACKET_LENGTH);
    
    memcpy(packet_header, header, BASE_PACKET_LENGTH);
    
    if (type == PACKET_TYPE_SYN || type == PACKET_TYPE_SYN_ACK) {
                
        if (packet_syn_header->version != socket->options->conn->version ||
            packet_syn_header->identifier == 0) {
            goto failed;            
        }
        
        memcpy((uint8_t*) packet_header + BASE_PACKET_LENGTH, 
                (uint8_t*) header + BASE_PACKET_LENGTH, 
                SYN_PACKET_HEADER_LENGTH);
    }
    
    else if (type == PACKET_TYPE_DATA) {
        packet->buffer_size = buffer_size;
        packet->buffer = realloc(packet->buffer, packet->buffer_size);
        
        memcpy(packet_header + SYN_PACKET_LENGTH,
                header + SYN_PACKET_LENGTH,
                packet->buffer_size - SYN_PACKET_LENGTH);
    }
    
    if (!rudp_packet_check_checksum(buffer, buffer_size)) {
        goto failed;        
    }
       
    debug_print("rudp_buffered_packet() succeed\n");
    return packet;
    
failed:
    if (packet) 
        rudp_packet_free(packet);
        
    debug_print("rudp_buffered_packet() failed\n");        
    return 0;
}

rudp_packet_type_t rudp_packet_check_type(
        uint8_t* buffer,
        uint32_t buffer_size)
{
    if (!buffer || buffer_size < BASE_PACKET_LENGTH)
        return PACKET_TYPE_UNKNOWN;
            
    if (*buffer == PACKET_FLAG_SYN) {
        if (buffer_size == SYN_PACKET_LENGTH)
            return PACKET_TYPE_SYN;
        else
            return PACKET_TYPE_UNKNOWN;
    }
    else if (*buffer == (PACKET_FLAG_ACK | PACKET_FLAG_SYN)) {
        if (buffer_size == SYN_PACKET_LENGTH)
            return PACKET_TYPE_SYN_ACK;
        else
            return PACKET_TYPE_UNKNOWN;
    }        
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

