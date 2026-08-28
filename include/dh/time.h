#ifndef DH_TIME_H
#define DH_TIME_H

#include <stdint.h>

typedef struct {
    float delta_time;
    double total_time;
    uint64_t frame_count;
    int fps;
} DHTime;

void dh_time_init(DHTime *time);
void dh_time_update(DHTime *time);

#endif /* DH_TIME_H */
