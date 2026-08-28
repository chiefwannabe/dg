#include "dh/tilemap.h"
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

static void generate_chunk_content(DHChunk *chunk, int chunk_index)
{
    chunk->chunk_index = chunk_index;
    chunk->start_tile_x = chunk_index * DH_CHUNK_WIDTH;
    chunk->active = true;

    /* 1. Default background void for chunk */
    for (int y = 0; y < DH_CHUNK_HEIGHT; y++) {
        for (int x = 0; x < DH_CHUNK_WIDTH; x++) {
            chunk->tiles[y][x].visual_id = 0;
            chunk->tiles[y][x].prop_id = 0;
            chunk->tiles[y][x].flags = DH_COLLISION_NONE;
        }
    }

    /* 2. Base Solid Ground Floor (y = 10 and y = 11) */
    for (int x = 0; x < DH_CHUNK_WIDTH; x++) {
        chunk->tiles[10][x].visual_id = 1; /* Stone floor */
        chunk->tiles[10][x].flags = DH_COLLISION_SOLID;

        chunk->tiles[11][x].visual_id = 21; /* Solid bedrock under floor */
        chunk->tiles[11][x].flags = DH_COLLISION_SOLID;
    }

    /* 3. Solid Dungeon Ceiling (y = 0) */
    for (int x = 0; x < DH_CHUNK_WIDTH; x++) {
        chunk->tiles[0][x].visual_id = 20;
        chunk->tiles[0][x].flags = DH_COLLISION_SOLID;
    }

    /* 4. Leftmost World Wall at Start (Chunk 0, x = 0) */
    if (chunk_index == 0) {
        for (int y = 0; y < DH_CHUNK_HEIGHT; y++) {
            chunk->tiles[y][0].visual_id = 20;
            chunk->tiles[y][0].flags = DH_COLLISION_SOLID;
        }
        /* Spawn archway decoration */
        chunk->tiles[8][2].prop_id = 1; /* Torch */
        chunk->tiles[8][6].prop_id = 1;
    }

    /* 5. Endless Biome / Pattern Variants */
    int pattern = (chunk_index == 0) ? 0 : (abs(chunk_index) % 5);

    switch (pattern) {
        case 0: /* Intro / Open Hall */
            /* Torch props along top wall */
            chunk->tiles[2][5].prop_id = 1;
            chunk->tiles[2][15].prop_id = 1;
            break;

        case 1: /* Elevated Stone Ledge */
            for (int x = 4; x <= 15; x++) {
                chunk->tiles[7][x].visual_id = 2; /* Platform floor */
                chunk->tiles[7][x].flags = DH_COLLISION_SOLID;
            }
            chunk->tiles[6][6].prop_id = 12;  /* Supply crate on ledge */
            chunk->tiles[6][13].prop_id = 18; /* Treasure chest on ledge */
            break;

        case 2: /* Pillar Passage */
            for (int y = 4; y <= 9; y++) {
                chunk->tiles[y][5].visual_id = 20;
                chunk->tiles[y][5].prop_id = 5; /* Stone pillar */
                chunk->tiles[y][5].flags = DH_COLLISION_SOLID;

                chunk->tiles[y][15].visual_id = 20;
                chunk->tiles[y][15].prop_id = 5;
                chunk->tiles[y][15].flags = DH_COLLISION_SOLID;
            }
            break;

        case 3: /* Stepped Platform Ledges */
            /* Lower Step */
            for (int x = 2; x <= 7; x++) {
                chunk->tiles[8][x].visual_id = 2;
                chunk->tiles[8][x].flags = DH_COLLISION_SOLID;
            }
            /* Higher Step */
            for (int x = 11; x <= 17; x++) {
                chunk->tiles[6][x].visual_id = 2;
                chunk->tiles[6][x].flags = DH_COLLISION_SOLID;
            }
            chunk->tiles[5][14].prop_id = 1; /* Torch on high step */
            break;

        case 4: /* Ruined Chamber Ledge & Crates */
            for (int x = 5; x <= 14; x++) {
                chunk->tiles[7][x].visual_id = 3; /* Cracked stone floor */
                chunk->tiles[7][x].flags = DH_COLLISION_SOLID;
            }
            chunk->tiles[6][8].prop_id = 12;
            chunk->tiles[6][9].prop_id = 12;
            break;
    }
}

bool dh_tilemap_init(DHTilemap *tilemap, int tile_size, Texture2D tileset, Texture2D props_tex)
{
    if (tilemap == NULL || tile_size <= 0) {
        return false;
    }

    tilemap->tile_size = tile_size;
    tilemap->tileset = tileset;
    tilemap->props_tex = props_tex;
    tilemap->furthest_generated_chunk = -1;

    for (int i = 0; i < DH_MAX_ACTIVE_CHUNKS; i++) {
        tilemap->active_chunks[i].active = false;
        tilemap->active_chunks[i].chunk_index = -1;
    }

    if (tilemap->tileset.id != 0) {
        SetTextureFilter(tilemap->tileset, TEXTURE_FILTER_POINT);
    }
    if (tilemap->props_tex.id != 0) {
        SetTextureFilter(tilemap->props_tex, TEXTURE_FILTER_POINT);
    }

    /* Pre-generate initial active chunks 0 to DH_MAX_ACTIVE_CHUNKS - 1 */
    for (int i = 0; i < DH_MAX_ACTIVE_CHUNKS; i++) {
        generate_chunk_content(&tilemap->active_chunks[i], i);
    }
    tilemap->furthest_generated_chunk = DH_MAX_ACTIVE_CHUNKS - 1;

    return true;
}

void dh_tilemap_free(DHTilemap *tilemap)
{
    if (tilemap == NULL) return;
    for (int i = 0; i < DH_MAX_ACTIVE_CHUNKS; i++) {
        tilemap->active_chunks[i].active = false;
    }
}

static DHChunk *get_or_create_chunk(DHTilemap *tilemap, int chunk_idx)
{
    if (chunk_idx < 0) return NULL;

    /* 1. Check if already active in buffer */
    for (int i = 0; i < DH_MAX_ACTIVE_CHUNKS; i++) {
        if (tilemap->active_chunks[i].active && tilemap->active_chunks[i].chunk_index == chunk_idx) {
            return &tilemap->active_chunks[i];
        }
    }

    /* 2. Find slot to recycle: prefer inactive, then furthest chunk outside active range */
    int slot_to_use = -1;

    for (int i = 0; i < DH_MAX_ACTIVE_CHUNKS; i++) {
        if (!tilemap->active_chunks[i].active) {
            slot_to_use = i;
            break;
        }
    }

    if (slot_to_use == -1) {
        int max_dist = -1;
        for (int i = 0; i < DH_MAX_ACTIVE_CHUNKS; i++) {
            int dist = abs(tilemap->active_chunks[i].chunk_index - chunk_idx);
            if (dist > max_dist) {
                max_dist = dist;
                slot_to_use = i;
            }
        }
    }

    if (slot_to_use < 0 || slot_to_use >= DH_MAX_ACTIVE_CHUNKS) {
        slot_to_use = 0;
    }

    generate_chunk_content(&tilemap->active_chunks[slot_to_use], chunk_idx);
    if (chunk_idx > tilemap->furthest_generated_chunk) {
        tilemap->furthest_generated_chunk = chunk_idx;
    }
    return &tilemap->active_chunks[slot_to_use];
}

DHTile dh_tilemap_get_tile(DHTilemap *tilemap, int world_tx, int ty)
{
    DHTile empty = { 0, 0, DH_COLLISION_NONE };
    if (tilemap == NULL || ty < 0 || ty >= DH_CHUNK_HEIGHT) {
        empty.flags = DH_COLLISION_SOLID;
        return empty;
    }

    if (world_tx < 0) {
        empty.flags = DH_COLLISION_SOLID; /* Left world border */
        return empty;
    }

    int chunk_idx = world_tx / DH_CHUNK_WIDTH;
    int local_tx = world_tx % DH_CHUNK_WIDTH;

    DHChunk *chunk = get_or_create_chunk(tilemap, chunk_idx);
    if (chunk == NULL) return empty;

    return chunk->tiles[ty][local_tx];
}

void dh_tilemap_set_tile(DHTilemap *tilemap, int world_tx, int ty, uint16_t visual_id, uint16_t prop_id, uint8_t flags)
{
    if (tilemap == NULL || ty < 0 || ty >= DH_CHUNK_HEIGHT || world_tx < 0) return;

    int chunk_idx = world_tx / DH_CHUNK_WIDTH;
    int local_tx = world_tx % DH_CHUNK_WIDTH;

    DHChunk *chunk = get_or_create_chunk(tilemap, chunk_idx);
    if (chunk != NULL) {
        chunk->tiles[ty][local_tx].visual_id = visual_id;
        chunk->tiles[ty][local_tx].prop_id = prop_id;
        chunk->tiles[ty][local_tx].flags = flags;
    }
}

bool dh_tilemap_is_solid(DHTilemap *tilemap, int world_tx, int ty)
{
    if (tilemap == NULL) return true;
    if (world_tx < 0 || ty < 0 || ty >= DH_CHUNK_HEIGHT) return true;

    DHTile t = dh_tilemap_get_tile(tilemap, world_tx, ty);
    return (t.flags & DH_COLLISION_SOLID) != 0;
}

bool dh_tilemap_check_rect_collision(DHTilemap *tilemap, Rectangle box)
{
    if (tilemap == NULL || tilemap->tile_size <= 0) return true;

    int min_tx = (int)floorf(box.x / (float)tilemap->tile_size);
    int max_tx = (int)floorf((box.x + box.width - 0.001f) / (float)tilemap->tile_size);
    int min_ty = (int)floorf(box.y / (float)tilemap->tile_size);
    int max_ty = (int)floorf((box.y + box.height - 0.001f) / (float)tilemap->tile_size);

    for (int ty = min_ty; ty <= max_ty; ty++) {
        for (int tx = min_tx; tx <= max_tx; tx++) {
            if (dh_tilemap_is_solid(tilemap, tx, ty)) {
                return true;
            }
        }
    }
    return false;
}

bool dh_tilemap_world_to_tile(const DHTilemap *tilemap, Vector2 world_pos, int *out_tx, int *out_ty)
{
    if (tilemap == NULL || tilemap->tile_size <= 0) return false;
    int tx = (int)floorf(world_pos.x / (float)tilemap->tile_size);
    int ty = (int)floorf(world_pos.y / (float)tilemap->tile_size);
    if (out_tx) *out_tx = tx;
    if (out_ty) *out_ty = ty;
    return (tx >= 0 && ty >= 0 && ty < DH_CHUNK_HEIGHT);
}

Vector2 dh_tilemap_tile_to_world(const DHTilemap *tilemap, int tx, int ty)
{
    int ts = (tilemap != NULL) ? tilemap->tile_size : 16;
    return (Vector2){ (float)(tx * ts), (float)(ty * ts) };
}

void dh_tilemap_update_active_chunks(DHTilemap *tilemap, float camera_x)
{
    if (tilemap == NULL) return;

    int center_chunk = (int)floorf(camera_x / (float)(DH_CHUNK_WIDTH * tilemap->tile_size));
    if (center_chunk < 0) center_chunk = 0;

    /* Ensure chunks around center_chunk (-2 to +4) are active */
    for (int c = center_chunk - 2; c <= center_chunk + 4; c++) {
        if (c >= 0) {
            get_or_create_chunk(tilemap, c);
        }
    }
}

void dh_tilemap_draw(DHTilemap *tilemap, const DHCamera2D *camera, int virtual_w, int virtual_h, bool show_collision_debug, int *out_visible_tiles)
{
    if (tilemap == NULL || camera == NULL) return;
    (void)virtual_h;

    /* Ensure active window chunks around camera position are loaded */
    dh_tilemap_update_active_chunks(tilemap, camera->position.x);

    float half_vw = (float)virtual_w * 0.5f / camera->zoom;
    float view_left = camera->position.x - half_vw;
    float view_right = camera->position.x + half_vw;

    int min_tx = (int)floorf(view_left / (float)tilemap->tile_size) - 1;
    int max_tx = (int)ceilf(view_right / (float)tilemap->tile_size) + 1;
    if (min_tx < 0) min_tx = 0;

    int visible_count = 0;
    bool has_tileset = (tilemap->tileset.id != 0);
    bool has_props = (tilemap->props_tex.id != 0);
    const int tileset_cols = 19;
    const int props_cols = 10;

    for (int ty = 0; ty < DH_CHUNK_HEIGHT; ty++) {
        for (int tx = min_tx; tx <= max_tx; tx++) {
            visible_count++;
            DHTile t = dh_tilemap_get_tile(tilemap, tx, ty);

            float px = (float)(tx * tilemap->tile_size);
            float py = (float)(ty * tilemap->tile_size);
            float ts = (float)tilemap->tile_size;
            Rectangle dest = { px, py, ts, ts };

            if (t.visual_id > 0 && has_tileset) {
                int tile_idx = t.visual_id - 1;
                int col = tile_idx % tileset_cols;
                int row = tile_idx / tileset_cols;
                Rectangle src = { col * ts, row * ts, ts, ts };
                DrawTexturePro(tilemap->tileset, src, dest, (Vector2){0,0}, 0.0f, WHITE);
            } else {
                if (t.flags & DH_COLLISION_SOLID) {
                    DrawRectangleV((Vector2){px, py}, (Vector2){ts, ts}, (Color){40, 40, 55, 255});
                    DrawRectangleLines((int)px, (int)py, (int)ts, (int)ts, (Color){65, 65, 85, 255});
                }
            }

            if (t.prop_id > 0 && has_props) {
                int prop_idx = t.prop_id - 1;
                int col = prop_idx % props_cols;
                int row = prop_idx / props_cols;
                Rectangle src = { col * ts, row * ts, ts, ts };
                DrawTexturePro(tilemap->props_tex, src, dest, (Vector2){0,0}, 0.0f, WHITE);
            }

            if (show_collision_debug) {
                if (t.flags & DH_COLLISION_SOLID) {
                    DrawRectangleLines((int)px, (int)py, (int)ts, (int)ts, (Color){230, 50, 50, 180});
                }
            }
        }
    }

    if (out_visible_tiles) {
        *out_visible_tiles = visible_count;
    }
}
