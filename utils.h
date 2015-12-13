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
#include <time.h>
#include "rudp.h"
#include "socket.h"
#include "channel.h"

#ifdef DEBUG_MODE
#define debug_print(fmt, ...) \
    do { printf(fmt, ## __VA_ARGS__); } while (0)
#else
#define debug_print(fmt, ...) \
    do { } while (0)
#endif

#define EXPORT __attribute__((visibility("default")))
#define LOCAL __attribute__((visibility("hidden")))

uint64_t rudp_timestamp();
uint64_t rudp_random();
uint32_t rudp_sequence(socket_t* socket);

#define ANSI_COLOR_RED     "\x1b[1;31m"
#define ANSI_COLOR_GREEN   "\x1b[1;32m"
#define ANSI_COLOR_YELLOW  "\x1b[1;33m"
#define ANSI_COLOR_BLUE    "\x1b[1;34m"
#define ANSI_COLOR_MAGENTA "\x1b[1;35m"
#define ANSI_COLOR_CYAN    "\x1b[1;36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

utimer_t* utimer(socket_t* socket, packet_t* packet, timer_type_t type);
void utimer_set(utimer_t* utimer, uint32_t interval);
void utimer_free(utimer_t* utimer);