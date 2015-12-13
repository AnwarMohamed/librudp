/*
*
*  Copyright (C) 2015  Anwar Mohamed <anwarelmakrahy[at]gmail.com>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to Anwar Mohamed
*  anwarelmakrahy[at]gmail.com
*
*/

#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <semaphore.h>
#include <stdbool.h>

typedef struct linkedlist_node_t linkedlist_node_t;

struct linkedlist_node_t {
    linkedlist_node_t* prev;
    linkedlist_node_t* next;
    void* data;    
};

typedef struct {
    uint32_t size;    
    linkedlist_node_t* head;
    linkedlist_node_t* tail;    
} linkedlist_t;

linkedlist_t* linkedlist();
void linkedlist_free(linkedlist_t* llist, bool free_data);

linkedlist_node_t* linkedlist_node(void* data);

linkedlist_node_t* linkedlist_insert_head(linkedlist_t* llist, void* data);
linkedlist_node_t* linkedlist_insert_tail(linkedlist_t* llist, void* data);

linkedlist_node_t* linkedlist_remove_head(linkedlist_t* llist);
linkedlist_node_t* linkedlist_remove_tail(linkedlist_t* llist);

uint8_t linkedlist_empty(linkedlist_t* llist);
uint32_t linkedlist_size(linkedlist_t* llist);