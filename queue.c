#include "queue.h"

queue_t* queue()
{
    return linkedlist();
}

void queue_free(
        queue_t* queue, 
        bool  free_data)
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
        queue_t* queue)
{    
    return linkedlist_remove_head(queue);
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