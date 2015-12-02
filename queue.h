#include <stdint.h>
#include <stdlib.h>
#include "linkedlist.h"

typedef llist_t queue_t;
typedef llist_node_t queue_node_t;

queue_t* queue_init();
void queue_free(queue_t* queue, uint8_t data);

queue_node_t* queue_enqueue(queue_t* queue, void* data);
queue_node_t* queue_dequeue(queue_t* queue);

uint8_t queue_empty(queue_t* queue);
uint32_t queue_size(queue_t* queue);