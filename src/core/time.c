#include "dh/time.h"
#include <raylib.h>
#include <stddef.h>

void dh_time_init(DHTime *time)
{
    if (time == NULL) {
        return;
    }

    time->delta_time = 0.0f;
    time->total_time = 0.0;
    time->frame_count = 0;
    time->fps = 0;
}

void dh_time_update(DHTime *time)
{
    if (time == NULL) {
        return;
    }

    float dt = GetFrameTime();
    if (dt > 0.1f) {
        dt = 0.1f;
    }

    time->delta_time = dt;
    time->total_time += dt;
    time->frame_count++;
    time->fps = GetFPS();
}
