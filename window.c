#include "window.h"

window_t* window(
        window_type_t type,
        uint8_t max_size)
{
    debug_print("window()\n");
    
    window_t* new_window = (window_t*) calloc(1, sizeof(window_t));    
    
    new_window->buffer = queue();            
    new_window->type = type;
    new_window->max_size = max_size;
    new_window->autocommit = true;
    
    pthread_mutex_init(&new_window->lock, 0);
    sem_init(&new_window->available, 0, 0);
    
    if (type == WINDOW_TYPE_SNW) {
        new_window->max_size = 1;
    }
    
    success_print("window() succeed\n");
    return new_window;
}

void window_max_size_set(
        window_t* window,
        uint8_t max_size) 
{
    if (window) {
        window_lock(window);
        window->max_size = max_size;
        window_unlock(window);
    }
}

void window_in_enquque(
        window_t* window,
        packet_t* packet)
{
    debug_print("window_in_enquque()\n");
    
    queue_node_t* new_node = queue_enqueue_priority(
            window->buffer, packet, packet->header->sequence);    
    
    //channel_send_ack(packet->socket, packet);
    
    if (window->autocommit) {
        window_in_commit(window);        
    }
    
    if (window->size) {     
        //utimer_set(packet->socket->channel->timer_cum_ack,
        //        packet->socket->options->conn->timeout_cum_ack);    
    }
    
    success_print("window_in_enquque() succeed\n");
}

void window_in_commit(
        window_t* window) 
{
    debug_print("window_in_commit()\n");
    
    window_lock(window);
    
    if (!window->buffer->size) {
        window->size = 0;
        window->head = 0;
        window->tail = 0;
    } else {
        
        if (!window->head) {
            window->head = queue_head(window->buffer);            
        }                            
        
        packet_t* packet;
        
        while (window->head->next) {
            packet = window->head->data;
            
            //printf("%08X == %08X, %08X == %08X\n",
            //packet->header->sequence, packet->socket->channel->acknowledge,
            //packet->header->acknowledge, packet->socket->channel->sequence +1);
            
            if (packet->header->sequence == packet->socket->channel->acknowledge &&
                packet->header->acknowledge == packet->socket->channel->sequence /*+
                        (packet->socket->options->state == STATE_ESTABLISHED? 1:0)*/) {
                
                packet->socket->channel->acknowledge += packet->data_buffer_size;
                    
                //packet->socket->channel->acknowledge = 
                //    packet->header->sequence + packet->data_buffer_size;
                //packet->socket->channel->sequence = packet->header->acknowledge;                 

                channel_send_ack(packet->socket, packet);
       
                window->head = window->head->next;
            } else {
                goto cleanup;
            }                    
        }
        
        if (window->head) {
            packet = window->head->data;

            //printf("%08X == %08X, %08X == %08X\n",
            //packet->header->sequence, packet->socket->channel->acknowledge,
            //packet->header->acknowledge, packet->socket->channel->sequence +1);
            
            if (packet->header->sequence == packet->socket->channel->acknowledge &&
                packet->header->acknowledge == packet->socket->channel->sequence /*+ 
                        (packet->socket->options->state == STATE_ESTABLISHED? 1:0)*/) {
                
                packet->socket->channel->acknowledge += packet->data_buffer_size;
                //packet->socket->channel->sequence = packet->header->acknowledge;                 

                channel_send_ack(packet->socket, packet);
                sem_post(&window->available);
       
                window->head = window->head->next;
            } else {
                
            }  
        }
    }
/*        
        queue_node_t* node = window->head;
        packet_t* packet;
        
        uint32_t current_size = 0;
        while (node && current_size++ < window->size) {
            window->tail = node;
            packet = node->data;
            
            if (!packet->ack || !packet->ack->transmission_time &&
                    window_sequenced(window, node->prev, node)) {
                        
                printf("seq:%08X == %08X, ack:%08X == %08X\n",
                        packet->socket->channel->sequence,
                        packet->header->sequence,
                        packet->socket->channel->acknowledge,
                        packet->header->acknowledge);
                        
                channel_send_ack(packet->socket, packet);
            }
            
            node = node->next;
        }        
    }

    window_unlock(window);        
    success_print("window_in_commit() succeed\n"); 
    
    //socket->channel->acknowledge = 
    //        packet->header->sequence + packet->data_buffer_size;
    //socket->channel->sequence = packet->header->acknowledge; 

    //if (channel_send_ack(socket, packet) < 0)
    //    goto failed;
     * */
     
cleanup:     
    window_unlock(window);        
    success_print("window_in_commit() succeed\n");      
}

bool window_sequenced(
        window_t* window,
        queue_node_t* prev_node,
        queue_node_t* node)        
{
    if (!window || !node) {
        return false;
    }
    
    packet_t* packet = node->data;
    packet_t* prev_packet;
    
    if (prev_node) {    
        prev_packet = prev_node->data;

        if (packet->header->sequence != 
                prev_packet->header->sequence + 
                prev_packet->data_buffer_size) {
            return false;
        }

    } else {    
        if (packet->header->acknowledge != 
                packet->socket->channel->sequence) {
            return false;
        }    
    }
    
    return true;
}

packet_t* window_in_dequeue(
        window_t* window,
        bool blocking)
{
    sem_wait(&window->available);    
    queue_node_t* node = queue_dequeue(window->buffer, blocking);
    
    if (node) {
        return node->data;
    } else {
        return 0;
    }
}         

void window_out_enqueue(
        window_t* window,
        packet_t* packet) 
{   
    debug_print("window_out_enqueue()\n");
        
    queue_enqueue(window->buffer, packet, packet->header->sequence);
    
    //printf("q=%d w=%d\n", window->buffer->size, window->size);

    if (window->autocommit) {
        window_out_commit(window);        
    }

    success_print("window_out_enqueue() succeed\n");
}

void window_autocommit_set(
        window_t* window,
        bool autocommit) 
{
    if (window) {
        window->autocommit = autocommit;
    }
}

void window_out_commit(
        window_t* window) 
{            
    debug_print("window_out_commit()\n");
    
    window_lock(window);
    
    if (!window->buffer->size) {
        window->size = 0;
        window->head = 0;
        window->tail = 0;
    } else {
        
        window->size = MIN(window->max_size, window->buffer->size);            
        window->head = queue_head(window->buffer);
        window->tail = window_get(window, window->size-1);                
        
        //printf("head: %p, tail: %p\n", window->head, window->tail);
        
        if (!window->tail || !window->head) {            
            return;
        }
        
        queue_node_t* node = window->head;
        packet_t* packet = (packet_t*) node->data;            
        
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

    window_unlock(window);
    success_print("window_out_commit() succeed\n");     
}

bool packet_ack_valid(
        packet_t* packet,
        packet_t* ack_packet)
{   
    debug_print("packet_ack_valid()\n");
    
    if (packet->type == PACKET_TYPE_SYN && 
            ack_packet->type == PACKET_TYPE_SYN_ACK) {
        if (ack_packet->header->acknowledge == 
            (packet->header->sequence + 
            (packet->data_buffer_size ? packet->data_buffer_size: 1))) {
            goto succeed;
        }
    } else {
    
        if (ack_packet->header->acknowledge == 
            (packet->header->sequence + 
            (packet->data_buffer_size ? packet->data_buffer_size: 1)) &&
            ack_packet->header->sequence == packet->header->acknowledge) {
            goto succeed;
        }
    }
        
    error_print("packet_ack_valid() failed\n");    
    return false;
    
succeed:
    success_print("packet_ack_valid() succeed\n"); 
    return true;    
}

queue_node_t* window_get(
        window_t* window,
        uint32_t index)
{
    debug_print("window_get()\n");
    
    //printf("index: %d\n", index);
    
    if (!window || index >= window->size) {
        return 0;        
    }
    
    if (!index) {
        return window->buffer->head;
    }
    
    //printf("check\n");
    
    queue_node_t* node = queue_head(window->buffer);
    uint32_t node_count = 0;
    
    while (node_count++ < (index-1)) {
        node = node->next;
    }
    
    //while (node && node->next && node_count < (index+1)) {
    //    printf("%d\n", node_count);
    //    
    //    node = node->next;
    //    node_count++;        
    //}
    
    //printf("%d\n", node_count);
    return node;    
}

void window_type_set(
        window_t* window, 
        window_type_t type)
{
    if (window) {
        window->type = type;
    }
}


void window_unlock(
        window_t* window)
{
    if (window) {
        pthread_mutex_unlock(&window->lock);
    }
}

void window_lock(
        window_t* window)
{
    if (window) {
        pthread_mutex_lock(&window->lock);
    }    
}

void window_slide(
        window_t* window)
{    
    debug_print("window_slide()\n");
    
    queue_node_t* node;
    packet_t* packet;
    
    window_lock(window);
    
    while (window->buffer->size > 0) {
        node = queue_head(window->buffer);
        packet = node->data;
        
        if (!packet->needs_ack && packet->transmission_time) {
            node = queue_dequeue(window->buffer, false);            
            
            packet_free(node->data);
            free(node);
            
            if (window->size) {
                window->size--;
            }
            
        } else {
            break;
        }
    }
    
    window_unlock(window);    
    window_out_commit(window);    
        
    success_print("window_slide() succeed\n");
}

bool window_out_ack(
        window_t* window,
        packet_t* ack_packet)
{   
    debug_print("window_ack_set()\n");
    
    if (!window || !ack_packet ||
        !window->size || !window->buffer->size) {
        return false;
    }
        
    queue_node_t* node = window->head;
    uint8_t node_count = 1;
    packet_t* packet;
    
    while(node && node_count <= window->size) {
        
        packet = (packet_t*) node->data;
        
        if (packet->needs_ack && packet_ack_valid(packet, ack_packet)) {
            
            packet->needs_ack = false;
            utimer_set(packet->timer_retrans, 0);
            
            if (queue_head(window->buffer)== node) {
                window_slide(window);                
            }
            
            goto succeed;
        }
        
        node_count++;
        node = node->next;
    }   

    error_print("window_ack_set() failed\n");    
    return false;
    
succeed:    
    success_print("window_ack_set() succeed\n");
    return true;    
}

int32_t window_free(
        window_t* window)
{
    debug_print("window_free()\n");
    
    if (window) {
        pthread_mutex_lock(&window->lock);
        
        if (window->buffer) {
            queue_node_t* node;
            packet_t* packet;
            
            while (queue_size(window->buffer)) {
                node = queue_dequeue(window->buffer, false);
                packet = (packet_t*) node->data;
                            
                packet_free(packet);           
                free(node);
            }

            queue_free(window->buffer, true);            
            window->buffer = 0;
        }
        
        sem_post(&window->available);
        sem_destroy(&window->available);
        
        pthread_mutex_unlock(&window->lock);
        pthread_mutex_destroy(&window->lock);
        free(window);
    }
}