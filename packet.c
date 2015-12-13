#include "packet.h"


packet_t* packet(
        packet_type_t type, 
        socket_t* socket)
{        
    debug_print("packet()\n");        
    
    if (!socket) { 
        goto failed; 
    }
    
    if (type == PACKET_TYPE_UNKNOWN) {
        goto failed;
    }
    
    packet_t* new_packet = calloc(1, sizeof(packet_t));
    
    new_packet->timer_retrans = utimer(socket, new_packet, TIMER_RETRANS);
    new_packet->creatation_time = rudp_timestamp();
    new_packet->type = type;
    new_packet->socket = socket;
    
    if (type == PACKET_TYPE_SYN || type == PACKET_TYPE_SYN_ACK) { 
        new_packet->buffer_size = SYN_PACKET_LENGTH;
    } else {
        new_packet->buffer_size = BASE_PACKET_LENGTH;
    }
    
    new_packet->buffer = calloc(new_packet->buffer_size, sizeof(uint8_t));        

    if (packet_header_set(socket, new_packet) < 0) { 
        goto failed; 
    }

    if (type == PACKET_TYPE_SYN || type == PACKET_TYPE_SYN_ACK) {            
        if (packet_syn_header_set(socket, new_packet) < 0) {
            goto failed;
        }                        
    }
    
    switch(type) {
    case PACKET_TYPE_DATA:
    case PACKET_TYPE_NULL:
    case PACKET_TYPE_RESET:
    case PACKET_TYPE_SYN:
    case PACKET_TYPE_SYN_ACK:
    case PACKET_TYPE_TCS:
        new_packet->needs_ack = true;
    }

    debug_print("packet() succeed\n");
    return new_packet;
    
failed:
    if (new_packet)
        packet_free(new_packet);    
        
    debug_print("packet() failed\n");
    return 0;    
}

void packet_print(packet_t* packet) {
    printf(
        "%s"
        "%d.%d.%d.%d:%d\t==> %d.%d.%d.%d:%d\t"
        "seq: 0x%08x, ack: 0x%08x" 
        "%s \n",                       
        
        packet->counter_retrans ? ANSI_COLOR_RED: ANSI_COLOR_GREEN,

        packet->source_addr >> 24 & 0xFF,
        packet->source_addr >> 16 & 0xFF,
        packet->source_addr >> 8 & 0xFF,
        packet->source_addr & 0xFF,        
        packet->source_port, 
     
        packet->destination_addr >> 24 & 0xFF, 
        packet->destination_addr >> 16 & 0xFF,
        packet->destination_addr >> 8 & 0xFF,
        packet->destination_addr & 0xFF,
        packet->destination_port,                     

        packet->header->sequence, packet->header->acknowledge,
        
        ANSI_COLOR_RESET);
}

void packet_timeout(utimer_t* timer) {
    if (timer->packet->counter_retrans >= 
        timer->socket->options->conn->max_retransmissions) {
                
        timer->socket->options->state = STATE_CLOSED;
        sem_post(&timer->socket->options->state_lock);
                
        utimer_set(timer, 0);
    } else {
        timer->packet->counter_retrans++;
        channel_send_packet(timer->packet);
    }    
}

int32_t packet_data_set(
        packet_t* packet, 
        uint8_t* buffer, 
        uint32_t buffer_size) 
{
    debug_print("packet_data_set()\n");
    
    if (!packet || packet->type != PACKET_TYPE_DATA) {
        goto failed;
    }
    
    packet->buffer_size = BASE_PACKET_LENGTH + buffer_size;
    packet->buffer = realloc(packet->buffer, packet->buffer_size);
    
    packet->data_buffer_size = buffer_size;
    packet->data_buffer = packet->buffer + BASE_PACKET_LENGTH; 
    
    memcpy(packet->data_buffer, buffer, buffer_size);
    
    packet_checksum_add(packet);
    
    debug_print("packet_data_set() succeed\n");
    return RUDP_SOCKET_SUCCESS;
    
failed:
    debug_print("packet_data_set() failed\n");
    return RUDP_SOCKET_ERROR;      
}

int32_t packet_header_set(
        socket_t* socket,
        packet_t* packet)
{
    debug_print("packet_header_set()\n");
        
    packet->header = (packet_header_t*) packet->buffer;
    packet->header->header_length = BASE_PACKET_LENGTH;
    packet->header->acknowledge = socket->channel->acknowledge;
    packet->header->sequence = socket->channel->sequence;
    
    switch(packet->type) {
    case PACKET_TYPE_ACK:
        packet->header->flags = PACKET_FLAG_ACK;
        break;        
    case PACKET_TYPE_SYN:
        packet->header->flags = PACKET_FLAG_SYN;
        break;        
    case PACKET_TYPE_SYN_ACK:
        packet->header->flags = PACKET_FLAG_ACK | PACKET_FLAG_SYN;
        break;        
    case PACKET_TYPE_DATA:
        packet->header->flags = PACKET_FLAG_ACK | PACKET_FLAG_DATA;
        break;        
    case PACKET_TYPE_EACK:
        packet->header->flags = PACKET_FLAG_ACK | PACKET_FLAG_EACK;
        break;        
    case PACKET_TYPE_NULL:
        packet->header->flags = PACKET_FLAG_ACK | PACKET_FLAG_NUL;
        break;        
    case PACKET_TYPE_RESET:
        packet->header->flags = PACKET_FLAG_RST;
        break;        
    case PACKET_TYPE_TCS:
        packet->header->flags = PACKET_FLAG_TCS;
        break;
    }
    
    packet_checksum_add(packet);    
    
    debug_print("packet_header_set() succeed\n");
    return RUDP_SOCKET_SUCCESS;    
}

int32_t packet_syn_header_set(
        socket_t* socket,
        packet_t* packet)
{   
    debug_print("packet_syn_header_set()\n");        
    
    packet->syn_header = (packet_syn_header_t* ) 
            packet->buffer + BASE_PACKET_LENGTH;
    
    if (packet->type == PACKET_TYPE_SYN) {                
        memcpy(packet->syn_header, socket->options->conn, 
                SYN_PACKET_HEADER_LENGTH);                
    }
        
    else if (packet->type == PACKET_TYPE_SYN_ACK) {
        memcpy(packet->syn_header, socket->options->conn, 
                SYN_PACKET_HEADER_LENGTH);
    }
                
    else
        goto failed;
    
    packet_checksum_add(packet);

    debug_print("packet_syn_header_set() succeed\n");
    return RUDP_SOCKET_SUCCESS;
    
failed:
    debug_print("packet_syn_header_set() failed\n");
    
    packet->syn_header = 0;
    return RUDP_SOCKET_ERROR;    
}


int32_t packet_free(
        packet_t* packet)
{
    if (packet) {
        if (packet->buffer) {
            free(packet->buffer);
        }
                
        utimer_free(packet->timer_retrans);        
        
        if (packet->ack) {
            packet_free(packet->ack);
        }
                    
        free(packet);
    }
}

int32_t packet_checksum_add(
        packet_t* packet)
{
    packet_header_t* header = (packet_header_t*) packet->buffer;
    
    switch (packet->type) {
    case PACKET_TYPE_SYN:
    case PACKET_TYPE_SYN_ACK:
        return (header->checksum = packet_checksum(
                packet->buffer, SYN_PACKET_LENGTH));
    default:
        return (header->checksum = packet_checksum(
                packet->buffer, BASE_PACKET_LENGTH));  
    }
}

uint16_t packet_checksum(
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

bool packet_checksum_check(
        uint8_t* buffer, 
        uint32_t buffer_size)
{      
    return ((uint16_t*) buffer)[1] == 
            packet_checksum(buffer, buffer_size);
}

packet_t* packet_buffered(
        socket_t* socket, 
        uint8_t* buffer, 
        uint32_t buffer_size)
{    
    debug_print("packet_buffered()\n");        
    
    packet_type_t type = packet_type_check(buffer, buffer_size);
    
    if (type == PACKET_TYPE_UNKNOWN) { 
        goto failed; 
    }        
    
    packet_t* new_packet = packet(type, socket);
    
    if (!new_packet) { 
        goto failed; 
    }
        
    packet_header_t *header = (packet_header_t*) buffer;
    
    memcpy(new_packet->header, header, BASE_PACKET_LENGTH);
    
    if (type == PACKET_TYPE_SYN || type == PACKET_TYPE_SYN_ACK) {
                
        if (new_packet->syn_header->version != socket->options->conn->version ||
            new_packet->syn_header->identifier == 0) {
            goto failed;            
        }
        
        memcpy(new_packet->syn_header, 
                (uint8_t*) header + BASE_PACKET_LENGTH, 
                SYN_PACKET_HEADER_LENGTH);
    }
    
    else if (type == PACKET_TYPE_DATA) {
        new_packet->buffer_size = buffer_size;
        new_packet->buffer = realloc(new_packet->buffer, new_packet->buffer_size);
        
        memcpy(new_packet->header + SYN_PACKET_LENGTH,
                header + SYN_PACKET_LENGTH,
                new_packet->buffer_size - SYN_PACKET_LENGTH);
    }
    
    if (!packet_checksum_check(buffer, buffer_size)) {
        goto failed;        
    }
       
    debug_print("packet_buffered() succeed\n");
    return new_packet;
    
failed:
    if (new_packet) 
        packet_free(new_packet);
        
    debug_print("packet_buffered() failed\n");        
    return 0;
}

packet_type_t packet_type_check(
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

