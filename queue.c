#include "queue.h"

queue_t* 
queue_init()
{
    return llist_init();
}

void 
queue_free(queue_t* queue, uint8_t data)
{
    llist_free(queue, data);
}

queue_node_t* 
queue_enqueue(queue_t* queue, void* data)
{
    return llist_insert_tail(queue, data);
}

queue_node_t* 
queue_dequeue(queue_t* queue)
{
    return llist_remove_head(queue);
}

uint8_t
queue_empty(queue_t* queue)
{
    return llist_empty(queue);
}

uint32_t 
queue_size(queue_t* queue)
{
    return llist_size(queue);
}