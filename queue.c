#include "queue.h"

queue_t* queue()
{
    return linkedlist();
}

void queue_node_free(
        queue_node_t* node)
{
    linkedlist_node_free(node);
}

void queue_lock(
        queue_t* queue)
{
    if (queue) {
        pthread_mutex_lock(&queue->lock);
    }
}        

void queue_unlock(
        queue_t* queue)
{
    if (queue) {
        pthread_mutex_unlock(&queue->lock);
    }
}    

queue_node_t* queue_head(
        queue_t* queue)
{
    return linkedlist_head(queue);
}

queue_node_t* queue_tail(
        queue_t* queue) 
{
    return linkedlist_tail(queue);
}

void queue_free(
        queue_t* queue, 
        bool free_data)
{
    linkedlist_free(queue, free_data);
}

queue_node_t* queue_enqueue(
        queue_t* queue, 
        void* data)
{    
    return linkedlist_insert_tail(queue, data);
}

queue_node_t* queue_dequeue(
        queue_t* queue,
        bool blocking)
{    
    return linkedlist_remove_head(queue, blocking);
}

uint8_t queue_empty(
        queue_t* queue)
{
    return linkedlist_empty(queue);
}

uint32_t queue_size(
        queue_t* queue)
{
    return linkedlist_size(queue);
}