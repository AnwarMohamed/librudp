#include <stdint.h>
#include <stdlib.h>

typedef struct llist_node_t llist_node_t;

struct llist_node_t {
    llist_node_t* prev;
    llist_node_t* next;
    void* data;
};

typedef struct {
    uint32_t size;
    llist_node_t* head;
    llist_node_t* tail;
} llist_t;

llist_t* llist_init();
void llist_free(llist_t* llist, uint8_t data);

llist_node_t* llist_node_init(void* data);

llist_node_t* llist_insert_head(llist_t* llist, void* data);
llist_node_t* llist_insert_tail(llist_t* llist, void* data);

llist_node_t* llist_remove_head(llist_t* llist);
llist_node_t* llist_remove_tail(llist_t* llist);

uint8_t llist_empty(llist_t* llist);
uint32_t llist_size(llist_t* llist);