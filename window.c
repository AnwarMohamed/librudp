#include "window.h"

window_t* window(
        uint8_t max_size)
{
    window_t* new_window = (window_t*) calloc(1, sizeof(window_t));
    
    new_window->buffer = queue();
    new_window->max_size = max_size;
    
    return new_window;
}

void window_max_size_set(
        window_t* window,
        uint8_t max_size) 
{
    if (window) {
        window->max_size = max_size;
    }
}

void window_out_enqueue(
        window_t* window,
        packet_t* packet) 
{
    queue_enqueue(window->buffer, packet);    
    window_expand(window);
}

void window_expand(window_t* window) {
    
    if (!window->buffer->size) {
        window->size = 0;
        window->head = 0;
        window->tail = 0;
    } else {
        
        window->size = MIN(window->max_size, window->buffer->size);
        window->head = window->buffer->head;
        window->tail = window_get(window, window->size-1);
        
        packet_t* packet = (packet_t*) window->head->data;
        queue_node_t* node = window->head;        
        
        while(node != window->tail) {
            packet = (packet_t*) node->data;
            
            if (!packet->transmission_time) {
                channel_send_packet(packet);
            }

            node = node->next;
        }
        
        if (node == window->tail) {
            if (!packet->transmission_time) {
                channel_send_packet(packet);
            }
        }
    }       
}

bool packet_ack_valid(
        packet_t* packet,
        packet_t* ack_packet)
{
    return ack_packet->header->acknowledge == packet->header->sequence + 
            (packet->data_buffer_size ? packet->data_buffer_size: 1);
}

queue_node_t* window_get(
        window_t* window,
        uint32_t index)
{
    if (!window) {
        return 0;        
    }
    
    if (!index) {
        return window->buffer->head;
    }
    
    queue_node_t* node = window->buffer->head;
    uint32_t node_count = 0;
    
    while (node && node->next && (node_count+1) < index) {
        node = node->next;
        node_count++;
    }
    
    if (node_count < index) {
        return 0;
    } else {
        return node;
    }
}

void window_slide(
        window_t* window)
{
    queue_node_t* node = queue_dequeue(window->buffer);  
    packet_t* packet = (packet_t*) node->data;        
    
    packet_free(packet);
    free(node);        
    
    window->size--;
    window_expand(window);    
}

bool window_ack_set(
        window_t* window,
        packet_t* ack_packet)
{
    if (!window || !ack_packet ||
        !window->size || !window->buffer->size) {
        return false;
    }
    
    queue_node_t* node = window->buffer->head;
    uint8_t node_count = 0;
    packet_t* packet;
    
    while(node && node_count < window->size) {
        
        packet = (packet_t*) node->data;
        
        if (packet->needs_ack && packet_ack_valid(packet, ack_packet)) {
            packet->needs_ack = false;
            
            if (window->buffer->head == node) {
                window_slide(window);
            }
            
            return true;
        }
        
        node_count++;
        node = node->next;
    }   

    return false;
}

bool window_ack_check(
        window_t* window,
        packet_t* ack_packet) 
{
    if (!window || !ack_packet) {
        return false;
    }
    
    if (!window->size || !window->buffer->size) {
        return false;
    }
    
    queue_node_t* temp_node = window->buffer->head;
    uint8_t temp_count = 0;
    packet_t* packet;
    
    while(temp_node && temp_count < window->size) {
        
        packet = (packet_t*) temp_node->data;
        
        if (packet->needs_ack && packet_ack_valid(packet, ack_packet)) {
            return true;
        }
        
        temp_count++;
        temp_node = temp_node->next;
    }
    
    return false;
}

int32_t window_free(
        window_t* window)
{
    if (window) {
        if (window->buffer) {
            queue_node_t* node;
            
            while (queue_size(window->buffer)) {
                node = queue_dequeue(window->buffer);

                packet_free(node->data);            
                free(node);
            }

            queue_free(window->buffer, true);
        }
        
        free(window);
    }
}