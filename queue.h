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
#include "linkedlist.h"

typedef linkedlist_t queue_t;
typedef linkedlist_node_t queue_node_t;

queue_t* queue();
void queue_free(queue_t* queue, bool free_data);

queue_node_t* queue_enqueue_priority(queue_t* queue, void* data, uint32_t priority);
queue_node_t* queue_enqueue(queue_t* queue, void* data, uint32_t id);
queue_node_t* queue_dequeue(queue_t* queue, bool blocking);

uint8_t queue_empty(queue_t* queue);
uint32_t queue_size(queue_t* queue);

void queue_lock_size(queue_t* queue);
queue_node_t* queue_head(queue_t* queue);