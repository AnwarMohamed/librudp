#include "utils.h"

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