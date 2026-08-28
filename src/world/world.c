#include "dh/world.h"
#include "dh/logging.h"
#include <raylib.h>
#include <math.h>
#include <stddef.h>

bool dh_world_init(DHWorld *world, int tile_size, DHAssetManager *assets)
{
    if (world == NULL) return false;

    Texture2D tileset = {0};
    Texture2D props = {0};

    if (assets != NULL) {
        tileset = dh_assets_load_texture(assets, "assets/craftpix-net-436971-free-top-down-roguelike-game-kit-pixel-art/5Tiled_files/Tileset.png");
        props = dh_assets_load_texture(assets, "assets/craftpix-net-436971-free-top-down-roguelike-game-kit-pixel-art/5Tiled_files/Objects.png");
    }

    if (!dh_tilemap_init(&world->tilemap, tile_size, tileset, props)) {
        dh_log_error("Failed to initialize tilemap for endless platformer world");
        return false;
    }

    /* Spawn position in Chunk 0 at start of endless world */
    world->spawn_pos = (Vector2){ 40.0f, 10.0f * 16.0f - 8.0f };

    dh_log_info("Endless Side-Scrolling Platformer World initialized successfully");
    return true;
}

void dh_world_shutdown(DHWorld *world)
{
    if (world == NULL) return;
    dh_tilemap_free(&world->tilemap);
}

void dh_world_draw(DHWorld *world, const DHCamera2D *camera, int virtual_w, int virtual_h, bool show_collision_debug, int *out_visible_tiles)
{
    if (world == NULL || camera == NULL) return;

    /* 1. Deep Dungeon Ambient Atmosphere Clear */
    ClearBackground((Color){ 12, 14, 22, 255 });

    /* 2. Parallax Background Dungeon Wall Arches (moves at 30% camera speed) */
    float bg_parallax_x = camera->position.x * 0.3f;
    int bg_spacing = 64;
    int start_i = (int)floorf((camera->position.x - (float)virtual_w) / (float)bg_spacing);
    int end_i = (int)ceilf((camera->position.x + (float)virtual_w) / (float)bg_spacing);

    for (int i = start_i; i <= end_i; i++) {
        float x = (float)(i * bg_spacing) - fmodf(bg_parallax_x, (float)bg_spacing);
        /* Draw subtle background stone pillar archways */
        DrawRectangle((int)x + 8, 16, 16, 144, (Color){ 20, 24, 38, 255 });
        DrawRectangle((int)x + 6, 12, 20, 8, (Color){ 28, 32, 48, 255 });
        DrawCircle((int)x + 16, 20, 6.0f, (Color){ 10, 12, 20, 255 });
    }

    /* 3. Playable Tilemap & Props Pass */
    dh_tilemap_draw(&world->tilemap, camera, virtual_w, virtual_h, show_collision_debug, out_visible_tiles);
}
