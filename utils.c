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
        rudp_socket_t* socket) 
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