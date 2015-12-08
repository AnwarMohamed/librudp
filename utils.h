#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "socket.h"

typedef struct rudp_socket_t rudp_socket_t;

uint64_t rudp_timestamp();
uint64_t rudp_random();

uint32_t rudp_sequence(rudp_socket_t* socket);