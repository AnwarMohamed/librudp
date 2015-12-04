#include "packet.h"

rudp_packet_t* rudp_packet(
        rudp_packet_type_t type, 
        rudp_socket_t* socket,
        uint8_t* buffer, 
        uint32_t buffer_size)
{
    if (!socket) { goto cleanup; }

    if (buffer) {
        return rudp_packet_from_buffer(
                type, socket, buffer, buffer_size);
    }
    
    if (type == PACKET_TYPE_SYN) {
        rudp_packet_t* packet = calloc(1, sizeof(rudp_packet_t));
        
        packet->buffer_size = SYNPACKET_LENGTH;
        packet->buffer = calloc(SYNPACKET_LENGTH, sizeof(uint8_t));
        packet->type = type;
        
        rudp_packet_set_header(packet, 0, 0);
        rudp_packet_set_syn_header(socket, packet, 0, 0);
    } 
    
    else if (type == PACKET_TYPE_SYN_ACK) {
        
    }
    
cleanup:
    printf("rudp_packet() failed\n");
    return NULL;    
}

void rudp_packet_set_header(
        rudp_packet_t* packet,
        uint8_t* buffer,
        uint32_t buffer_size)
{
    
}

void rudp_packet_set_syn_header(
        rudp_socket_t* socket,
        rudp_packet_t* packet,
        uint8_t* buffer,
        uint32_t buffer_size)
{
    
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

bool rudp_packet_check_checksum(
        rudp_packet_t* packet)
{
    
}