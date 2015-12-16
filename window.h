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
#include "rudp.h"
#include "utils.h"
#include "packet.h"

window_t* window(window_type_t type, uint8_t max_size);
void window_type_set(window_t* window, window_type_t type);
int32_t window_free(window_t* window);

queue_node_t* window_get(window_t* window, uint32_t index);
void window_out_enqueue(window_t* window, packet_t* packet);
void window_out_commit(window_t* window);
bool window_out_ack(window_t* window, packet_t* ack_packet);

packet_t* window_in_dequeue(window_t* window, bool blocking);
void window_in_commit(window_t* window);
void window_in_enquque(window_t* window, packet_t* packet);

void window_lock(window_t* window);
void window_unlock(window_t* window);

void window_autocommit_set(window_t* window, bool autocommit);
void window_max_size_set(window_t* window, uint8_t max_size);
bool window_sequenced(window_t* window, queue_node_t* prev_node, queue_node_t* node);