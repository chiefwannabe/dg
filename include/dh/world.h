#ifndef DH_WORLD_H
#define DH_WORLD_H

#include "dh/tilemap.h"
#include "dh/camera.h"
#include "dh/assets.h"
#include <raylib.h>
#include <stdbool.h>

typedef struct {
    DHTilemap tilemap;
    Vector2 spawn_pos;
} DHWorld;

bool dh_world_init(DHWorld *world, int tile_size, DHAssetManager *assets);
void dh_world_shutdown(DHWorld *world);
void dh_world_draw(DHWorld *world, const DHCamera2D *camera, int virtual_w, int virtual_h, bool show_collision_debug, int *out_visible_tiles);

#endif /* DH_WORLD_H */
