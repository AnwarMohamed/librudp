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

window_t* window(uint8_t max_size);
int32_t window_free(window_t* window);

queue_node_t* window_get(window_t* window, uint32_t index);
void window_out_enqueue(window_t* window, packet_t* packet);
void window_expand(window_t* window);