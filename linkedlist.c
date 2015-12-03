#include "linkedlist.h"

llist_t* llist_init() 
{
    llist_t* llist_new = (llist_t*) calloc(1, sizeof(llist_t));
    sem_init(&llist_new->size_lock, 0, 0);
    return llist_new;
}

llist_node_t* llist_node_init(
        void* data) 
{
    llist_node_t* llist_node = calloc(1, sizeof(llist_node_t));
    llist_node->data = data;
}

llist_node_t* llist_insert_head(
        llist_t* llist, 
        void* data)
{
    if (!llist)
        return NULL;
    
    llist_node_t* llist_node = llist_node_init(data);
    llist_node->prev = NULL;
    llist_node->next = llist->head;    
    
    if (llist->head)
        llist->head->prev = llist_node;
    
    if (!llist->tail)
        llist->tail = llist_node;
    
    llist->head = llist_node;
    llist->size++;
    
    sem_post(&llist->size_lock);
    return llist_node;
}

llist_node_t* llist_insert_tail(
        llist_t* llist, 
        void* data)
{
    if (!llist)
        return NULL;
    
    llist_node_t* llist_node = llist_node_init(data);
    llist_node->prev = llist->tail;
    llist_node->next = NULL;    
    
    if (llist->tail)
        llist->tail->next = llist_node;
    
    if (!llist->head)
        llist->head = llist_node;
    
    llist->tail = llist_node;
    llist->size++;
    
    sem_post(&llist->size_lock);
    return llist_node;
}

llist_node_t* llist_remove_head(
        llist_t* llist)
{
    if (!llist)
        return NULL;        
    
    sem_wait(&llist->size_lock);
    
    llist_node_t* llist_node = llist->head;    
    llist->head = llist_node->next;
    
    if (llist->head) {
        llist->head->prev = NULL;
        
        if (!llist->head->next)
            llist->tail = NULL;
            
    } else {
        llist->tail = NULL;
    }
    
    llist->size--;
    
    return llist_node;
}

llist_node_t* llist_remove_tail(
        llist_t* llist)
{
    if (!llist)
        return NULL;
        
    sem_wait(&llist->size_lock);        
    
    llist_node_t* llist_node = llist->tail;    
    llist->tail = llist_node->prev;
    
    if (llist->tail) {
        llist->head->next = NULL;
        
        if (!llist->head->prev)
            llist->head = NULL;
            
    } else {
        llist->head = NULL;
    }
    
    llist->size--;
    
    return llist_node;  
}

uint8_t llist_empty(
        llist_t* llist)
{
    return llist ? !llist->size: 0;
}

uint32_t llist_size(
        llist_t* llist)
{
    return llist ? llist->size: 0;
}

void llist_free(
        llist_t* llist, 
        uint8_t data)
{
    llist_node_t* llist_node;
    while (llist_size(llist)) {
        llist_node = llist_remove_tail(llist);
        
        if (data && llist_node->data)
            free(llist_node->data);
            
        free(llist_node);
    }
    
    sem_destroy(&llist->size_lock);
    free(llist);
}