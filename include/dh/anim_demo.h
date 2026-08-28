#ifndef DH_ANIM_DEMO_H
#define DH_ANIM_DEMO_H

#include "dh/animation.h"
#include "dh/assets.h"
#include "dh/input.h"
#include <stdbool.h>

typedef struct {
    DHAnimationController controller;
    bool loaded;
    float current_speed;
    bool is_paused;
} DHAnimDemo;

bool dh_anim_demo_init(DHAnimDemo *demo, DHAssetManager *assets);
void dh_anim_demo_update(DHAnimDemo *demo, const DHInput *input, float delta_time);
void dh_anim_demo_draw(const DHAnimDemo *demo, int screen_width, int screen_height);
void dh_anim_demo_shutdown(DHAnimDemo *demo, DHAssetManager *assets);

#endif /* DH_ANIM_DEMO_H */
