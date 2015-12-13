#include "linkedlist.h"

linkedlist_t* linkedlist() 
{
    return (linkedlist_t*) calloc(1, sizeof(linkedlist_t));        
}

linkedlist_node_t* linkedlist_node(
        void* data) 
{
    linkedlist_node_t* llist_node = calloc(1, sizeof(linkedlist_node_t));
    llist_node->data = data;
}

linkedlist_node_t* linkedlist_insert_head(
        linkedlist_t* llist, 
        void* data)
{
    if (!llist)
        return NULL;
    
    linkedlist_node_t* llist_node = linkedlist_node(data);
    llist_node->prev = NULL;
    llist_node->next = llist->head;    
    
    if (llist->head)
        llist->head->prev = llist_node;
    
    if (!llist->tail)
        llist->tail = llist_node;
    
    llist->head = llist_node;
    llist->size++;
        
    return llist_node;
}

linkedlist_node_t* linkedlist_insert_tail(
        linkedlist_t* llist, 
        void* data)
{
    if (!llist)
        return NULL;
    
    linkedlist_node_t* llist_node = linkedlist_node(data);
    llist_node->prev = llist->tail;
    llist_node->next = NULL;    
    
    if (llist->tail)
        llist->tail->next = llist_node;
    
    if (!llist->head)
        llist->head = llist_node;
    
    llist->tail = llist_node;
    llist->size++;
        
    return llist_node;
}

linkedlist_node_t* linkedlist_remove_head(
        linkedlist_t* llist)
{
    if (!llist)
        return NULL;                
    
    linkedlist_node_t* llist_node = llist->head;    
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

linkedlist_node_t* linkedlist_remove_tail(
        linkedlist_t* llist)
{
    if (!llist)
        return NULL;        
    
    linkedlist_node_t* llist_node = llist->tail;    
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

uint8_t linkedlist_empty(
        linkedlist_t* llist)
{
    return llist ? !llist->size: 0;
}

uint32_t linkedlist_size(
        linkedlist_t* llist)
{
    return llist ? llist->size: 0;
}

void linkedlist_free(
        linkedlist_t* llist, 
        bool free_data)
{
    linkedlist_node_t* llist_node;
    while (linkedlist_size(llist)) {
        llist_node = linkedlist_remove_tail(llist);
        
        if (free_data && llist_node->data)
            free(llist_node->data);
            
        free(llist_node);
    }
        
    free(llist);
}