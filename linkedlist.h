/*
*
*  Copyright (C) 2015  Anwar Mohamed <anwarelmakrahy[at]gmail.com>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
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

typedef struct llist_node_t llist_node_t;

struct llist_node_t {
    llist_node_t* prev;
    llist_node_t* next;
    void* data;    
};

typedef struct {
    uint32_t size;
    sem_t size_lock;
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