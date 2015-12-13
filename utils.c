#include "utils.h"
#include "socket.h"
#include <openssl/md5.h>

uint64_t rudp_timestamp()
{
    struct timespec ts;
    uint64_t ts_64;
    
    if (clock_gettime(CLOCK_REALTIME, &ts)) {
        return (ts_64 = 0);
    }
    
    ts_64 = ts.tv_sec * 1000000;    
    ts_64 += ts.tv_nsec / 1000;
    
    return ts_64;
}

uint64_t rudp_random()
{
    srand(rudp_timestamp());    
    return rand();
}

uint32_t rudp_sequence(
        socket_t* socket) 
{            
    uint8_t* md5_cypher = malloc(MD5_DIGEST_LENGTH* sizeof(uint8_t));
    uint32_t* md5_plain = malloc(5* sizeof(uint32_t));
        
    md5_plain[0] = ntohl(socket->local_addr.sin_addr.s_addr);
    md5_plain[1] = ntohs(socket->local_addr.sin_port);
    md5_plain[2] = ntohl(socket->remote_addr.sin_addr.s_addr);
    md5_plain[3] = ntohs(socket->remote_addr.sin_port);
    md5_plain[4] = rudp_random();
    
    MD5((uint8_t*)md5_plain, sizeof(md5_plain) * sizeof(uint32_t), md5_cypher);    
    
    uint32_t sequence_no = *(uint32_t*) md5_cypher;
    
    free(md5_cypher);
    free(md5_plain);
    
    return sequence_no;
}

void utimer_free(
        utimer_t* utimer)
{
    if (utimer) {
        timer_delete(utimer->timer);
        free(utimer->event);
        free(utimer);
    }
}

utimer_t* utimer(
        socket_t* socket,
        packet_t* packet,
        timer_type_t type)
{
    utimer_t* new_timer = calloc(1, sizeof(utimer_t));
    new_timer->event = calloc(1, sizeof(struct sigevent));
    new_timer->event->sigev_notify = SIGEV_THREAD;   
    new_timer->event->sigev_notify_function = socket_timer_handler;
    new_timer->event->sigev_notify_attributes = NULL;    
    new_timer->event->sigev_signo = RUDP_SOCKET_SIGNAL;
    new_timer->event->sigev_value.sival_ptr = new_timer;    
    
    new_timer->socket = socket;
    new_timer->packet = packet;
    new_timer->type = type;

    timer_create(CLOCK_REALTIME, new_timer->event, &new_timer->timer);    
    
    return new_timer;
}

void utimer_set(
        utimer_t* utimer,
        uint32_t interval)
{
    utimer->timer_value.it_value.tv_sec = interval/1000;
    utimer->timer_value.it_value.tv_nsec = (interval % 1000) * 1000;
    utimer->timer_value.it_interval.tv_sec = interval/1000;
    utimer->timer_value.it_interval.tv_nsec = (interval % 1000) * 1000;
    
    if (timer_settime(utimer->timer, 0, &utimer->timer_value, 0) < 0) {
        debug_print("rudp_timer_set() failed\n");
    }
}