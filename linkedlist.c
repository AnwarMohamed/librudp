#include "linkedlist.h"

linkedlist_t* linkedlist() 
{
    linkedlist_t* list = 
            (linkedlist_t*) calloc(1, sizeof(linkedlist_t));
    sem_init(&list->size_lock, 0, 0);
    pthread_mutex_init(&list->lock, 0);
    return list;
}

void linkedlist_node_free(
        linkedlist_node_t* node)
{
    if (node) {
        pthread_mutex_destroy(&node->lock);
        free(node);
    }
}

linkedlist_node_t* linkedlist_head(
        linkedlist_t* list)
{
    if (!list) {
        return 0;        
    }
    
    linkedlist_node_t* head_node;
    
    pthread_mutex_lock(&list->lock);
    head_node = list->head;
    pthread_mutex_unlock(&list->lock);
    
    return head_node;
}

linkedlist_node_t* linkedlist_tail(
        linkedlist_t* list)
{
    if (!list) {
        return 0;        
    }
    
    linkedlist_node_t* tail_node;
    
    pthread_mutex_lock(&list->lock);
    tail_node = list->tail;
    pthread_mutex_unlock(&list->lock);
    
    return tail_node;
}

linkedlist_node_t* linkedlist_node(
        void* data,
        uint32_t id) 
{
    linkedlist_node_t* llist_node = calloc(1, sizeof(linkedlist_node_t));
    pthread_mutex_init(&llist_node->lock, 0);
    llist_node->data = data;
    llist_node->id = id;
}

linkedlist_node_t* linkedlist_insert_head(
        linkedlist_t* list, 
        void* data,
        uint32_t id)
{
    if (!list)
        return 0;            
    
    linkedlist_node_t* new_node = linkedlist_node(data, id);
    
    pthread_mutex_lock(&list->lock);            
    pthread_mutex_lock(&new_node->lock);
    
    new_node->prev = 0;
    new_node->next = list->head;    
    
    if (list->head) {
        pthread_mutex_lock(&list->head->lock);
        list->head->prev = new_node;        
        pthread_mutex_unlock(&list->head->lock);
    }
    
    if (!list->tail)
        list->tail = new_node;
    
    list->head = new_node;
    
    list->size++;
    sem_post(&list->size_lock);
    
    pthread_mutex_unlock(&new_node->lock);    
    pthread_mutex_unlock(&list->lock);
    return new_node;
}

linkedlist_node_t* linkedlist_insert_tail(
        linkedlist_t* list, 
        void* data,
        uint32_t id)
{
    if (!list)
        return 0;
    
    linkedlist_node_t* new_node = linkedlist_node(data, id);
    
    pthread_mutex_lock(&list->lock);            
    pthread_mutex_lock(&new_node->lock);
    
    new_node->prev = list->tail;
    new_node->next = 0;    
    
    if (list->tail) {
        pthread_mutex_lock(&list->tail->lock);
        list->tail->next = new_node;
        pthread_mutex_unlock(&list->tail->lock);
    }
    
    if (!list->head)
        list->head = new_node;
    
    list->tail = new_node;
    
    list->size++;
    sem_post(&list->size_lock);

    pthread_mutex_unlock(&new_node->lock);    
    pthread_mutex_unlock(&list->lock);    
    return new_node;
}

linkedlist_node_t* linkedlist_remove_head(
        linkedlist_t* list,
        bool blocking)
{
    if (!list) {
        return 0;                       
    } 
    
    pthread_mutex_lock(&list->lock);  
    
    if (blocking) {
        sem_wait(&list->size_lock);
    } else {
        int value; 
        sem_getvalue(&list->size_lock, &value);
        
        if (value) {
            sem_wait(&list->size_lock);
        }
    }
    
    if (!list->head) {
        return 0;
    }
    
    linkedlist_node_t* node = list->head;    
    list->head = node->next;
    
    if (list->head) {
        pthread_mutex_lock(&list->head->lock); 
        
        list->head->prev = 0;
        
        if (!list->head->next)
            list->tail = 0;
        
        pthread_mutex_unlock(&list->head->lock);     
    } else {
        list->tail = 0;
    }
    
    list->size--;            
    
    pthread_mutex_unlock(&list->lock);  
    return node;
}

linkedlist_node_t* linkedlist_remove_tail(
        linkedlist_t* list,
        bool blocking)
{
    if (!list)
        return 0;        

    pthread_mutex_lock(&list->lock);

    if (blocking) {
        sem_wait(&list->size_lock);
    } else {
        int value; 
        sem_getvalue(&list->size_lock, &value);
        
        if (value) {
            sem_wait(&list->size_lock);
        }
    }
    
    if (!list->tail) {
        return 0;
    }
    
    linkedlist_node_t* node = list->tail;    
    list->tail = node->prev;
    
    if (list->tail) {
        
        list->head->next = 0;
        
        if (!list->head->prev)
            list->head = 0;
            
    } else {
        list->head = 0;
    }
    
    list->size--;
    
    pthread_mutex_unlock(&list->lock);  
    return node;  
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
        llist_node = linkedlist_remove_tail(llist, false);
        
        if (free_data && llist_node->data)
            free(llist_node->data);
            
        free(llist_node);
    }
        
    sem_post(&llist->size_lock);
    sem_destroy(&llist->size_lock);
    
    pthread_mutex_destroy(&llist->lock);
    free(llist);
}

linkedlist_node_t* linkedlist_insert_sorted(
        linkedlist_t* list, 
        void* data, 
        uint32_t id)
{
    if (!list)
        return 0;
    
    linkedlist_node_t* new_node = linkedlist_node(data, id);
    
    pthread_mutex_lock(&list->lock);       
    pthread_mutex_lock(&new_node->lock);  
    
    linkedlist_node_t* node = list->head;
    
    while(node && node->id < id)
        node = node->next;
        
    if (!node) {
        /* insert_tail */
        
        new_node->prev = list->tail;
        new_node->next = 0;    
        
        if (list->tail) {
            pthread_mutex_lock(&list->tail->lock);
            list->tail->next = new_node;
            pthread_mutex_unlock(&list->tail->lock);
        }
        
        if (!list->head) {
            list->head = new_node;            
        }
        
        list->tail = new_node;             
        
        /* insert_tail */
    } else {
        
        if (node->prev) {
            node = node->prev;
        }
        
        new_node->next = node->next;
        new_node->prev = node;
        
        if (new_node->next) {
            new_node->next->prev = new_node;
        } else {
            list->tail = new_node;
        }
        
        node->next = new_node;                        
    }
    
    list->size++;
    sem_post(&list->size_lock); 
    
    pthread_mutex_unlock(&new_node->lock);  
    pthread_mutex_unlock(&list->lock);  
    return new_node;  
}