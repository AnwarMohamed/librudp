#include "queue.h"

queue_t* queue_init()
{
    return llist_init();
}

void queue_free(
        queue_t* queue, 
        bool  free_data)
{
    llist_free(queue, free_data);
}

void queue_lock_size(
        queue_t* queue)
{
    int32_t value; 
    sem_getvalue(&queue->size_lock, &value);
    
    if (!value)
        sem_post(&queue->size_lock);
}

queue_node_t* queue_enqueue(
        queue_t* queue, 
        void* data)
{    
    return llist_insert_tail(queue, data);
}

queue_node_t* queue_dequeue(
        queue_t* queue)
{    
    return llist_remove_head(queue);
}

uint8_t queue_empty(
        queue_t* queue)
{
    return llist_empty(queue);
}

uint32_t queue_size(
        queue_t* queue)
{
    return llist_size(queue);
}