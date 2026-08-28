#ifndef DH_TILEMAP_H
#define DH_TILEMAP_H

#include "dh/camera.h"
#include <raylib.h>
#include <stdint.h>
#include <stdbool.h>

#define DH_CHUNK_WIDTH 20   /* 20 tiles * 16px = 320px (1 virtual screen width) */
#define DH_CHUNK_HEIGHT 12  /* 12 tiles * 16px = 192px */
#define DH_MAX_ACTIVE_CHUNKS 8

typedef enum {
    DH_COLLISION_NONE = 0,
    DH_COLLISION_SOLID = (1 << 0),
    DH_COLLISION_HAZARD = (1 << 1),
    DH_COLLISION_TRIGGER = (1 << 2)
} DHTileCollisionFlags;

typedef struct {
    uint16_t visual_id;  /* Base visual tile ID in Tileset.png (0 = void) */
    uint16_t prop_id;    /* Props/decoration overlay tile ID in Objects.png (0 = none) */
    uint8_t flags;       /* Collision flags (DH_COLLISION_SOLID, etc.) */
} DHTile;

typedef struct {
    int chunk_index;     /* 0, 1, 2, 3... */
    int start_tile_x;    /* chunk_index * DH_CHUNK_WIDTH */
    bool active;
    DHTile tiles[DH_CHUNK_HEIGHT][DH_CHUNK_WIDTH];
} DHChunk;

typedef struct {
    int tile_size;                             /* 16 pixels */
    DHChunk active_chunks[DH_MAX_ACTIVE_CHUNKS];/* Ring buffer of loaded chunks */
    int furthest_generated_chunk;              /* Highest chunk_index generated so far */
    Texture2D tileset;                         /* Base tileset texture */
    Texture2D props_tex;                       /* Props texture */
} DHTilemap;

bool dh_tilemap_init(DHTilemap *tilemap, int tile_size, Texture2D tileset, Texture2D props_tex);
void dh_tilemap_free(DHTilemap *tilemap);

DHTile dh_tilemap_get_tile(DHTilemap *tilemap, int world_tx, int ty);
void dh_tilemap_set_tile(DHTilemap *tilemap, int world_tx, int ty, uint16_t visual_id, uint16_t prop_id, uint8_t flags);

bool dh_tilemap_is_solid(DHTilemap *tilemap, int world_tx, int ty);
bool dh_tilemap_check_rect_collision(DHTilemap *tilemap, Rectangle box);

bool dh_tilemap_world_to_tile(const DHTilemap *tilemap, Vector2 world_pos, int *out_tx, int *out_ty);
Vector2 dh_tilemap_tile_to_world(const DHTilemap *tilemap, int tx, int ty);

void dh_tilemap_update_active_chunks(DHTilemap *tilemap, float camera_x);
void dh_tilemap_draw(DHTilemap *tilemap, const DHCamera2D *camera, int virtual_w, int virtual_h, bool show_collision_debug, int *out_visible_tiles);

#endif /* DH_TILEMAP_H */
