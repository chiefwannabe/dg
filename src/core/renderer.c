#include "dh/renderer.h"
#include "dh/game.h"
#include "dh/logging.h"
#include "dh/platform.h"
#include "dh/coords.h"

#include <raylib.h>
#include <stddef.h>
#include <math.h>
#include <stdio.h>

bool dh_renderer_init(DHRenderer *renderer, const DHConfig *config)
{
    if (renderer == NULL || config == NULL) {
        return false;
    }

    renderer->virtual_width = config->virtual_width;
    renderer->virtual_height = config->virtual_height;
    renderer->fullscreen = config->fullscreen;

    InitWindow(config->window_width, config->window_height, config->title);
    SetTargetFPS(config->target_fps);

    if (config->vsync) {
        SetWindowState(FLAG_VSYNC_HINT);
    }

    renderer->target = LoadRenderTexture(renderer->virtual_width, renderer->virtual_height);
    SetTextureFilter(renderer->target.texture, TEXTURE_FILTER_POINT);

    float window_w = (float)GetScreenWidth();
    float window_h = (float)GetScreenHeight();

    renderer->scale = fminf(window_w / (float)renderer->virtual_width,
                            window_h / (float)renderer->virtual_height);
    if (renderer->scale < 1.0f) renderer->scale = 1.0f;

    renderer->offset_x = (window_w - ((float)renderer->virtual_width * renderer->scale)) * 0.5f;
    renderer->offset_y = (window_h - ((float)renderer->virtual_height * renderer->scale)) * 0.5f;

    renderer->dest_rect = (Rectangle){
        renderer->offset_x,
        renderer->offset_y,
        (float)renderer->virtual_width * renderer->scale,
        (float)renderer->virtual_height * renderer->scale
    };

    dh_log_info("Pixel-Perfect Renderer initialized: Base Res %dx%d, Filter: Nearest",
                renderer->virtual_width, renderer->virtual_height);

    return true;
}

void dh_renderer_begin_offscreen(DHRenderer *renderer)
{
    if (renderer == NULL) return;

    float window_w = (float)GetScreenWidth();
    float window_h = (float)GetScreenHeight();

    renderer->scale = fminf(window_w / (float)renderer->virtual_width,
                            window_h / (float)renderer->virtual_height);
    if (renderer->scale < 1.0f) renderer->scale = 1.0f;

    renderer->offset_x = (window_w - ((float)renderer->virtual_width * renderer->scale)) * 0.5f;
    renderer->offset_y = (window_h - ((float)renderer->virtual_height * renderer->scale)) * 0.5f;

    renderer->dest_rect = (Rectangle){
        renderer->offset_x,
        renderer->offset_y,
        (float)renderer->virtual_width * renderer->scale,
        (float)renderer->virtual_height * renderer->scale
    };

    BeginDrawing();
    BeginTextureMode(renderer->target);
    ClearBackground((Color){14, 14, 18, 255});
}

void dh_renderer_end_offscreen(DHRenderer *renderer)
{
    (void)renderer;
    EndTextureMode();
}

void dh_renderer_draw_framebuffer(const DHRenderer *renderer)
{
    if (renderer == NULL) return;

    ClearBackground(BLACK);

    Rectangle src_rect = { 0.0f, 0.0f, (float)renderer->virtual_width, -(float)renderer->virtual_height };
    DrawTexturePro(renderer->target.texture, src_rect, renderer->dest_rect, (Vector2){0, 0}, 0.0f, WHITE);
}

void dh_renderer_draw_foundation_ui(const DHRenderer *renderer, const struct DHGame *game, int visible_tiles)
{
    if (renderer == NULL || game == NULL) return;

    DrawRectangle(0, 0, renderer->virtual_width, 18, (Color){10, 10, 14, 220});
    DrawText("DUNGEON HUNTER", 4, 4, 10, RAYWHITE);

    char plat_info[64];
    snprintf(plat_info, sizeof(plat_info), "[%s | 320x180 Virtual]", dh_platform_get_name());
    DrawText(plat_info, 150, 4, 10, (Color){100, 200, 255, 255});

    DrawRectangle(0, 122, renderer->virtual_width, 58, (Color){12, 12, 18, 230});
    DrawLine(0, 122, renderer->virtual_width, 122, (Color){60, 60, 80, 255});

    char buf[128];
    snprintf(buf, sizeof(buf), "SCR:(%.0f,%.0f) VIR:(%.0f,%.0f) WLD:(%.0f,%.0f)",
             game->input.raw_mouse_screen.x, game->input.raw_mouse_screen.y,
             game->input.virtual_mouse.x, game->input.virtual_mouse.y,
             game->input.world_mouse.x, game->input.world_mouse.y);
    DrawText(buf, 4, 126, 10, (Color){255, 220, 100, 255});

    snprintf(buf, sizeof(buf), "CAM Pos:(%.0f,%.0f) Tgt:(%.0f,%.0f) Tiles Vis:%d",
             game->camera.position.x, game->camera.position.y,
             game->camera.target.x, game->camera.target.y, visible_tiles);
    DrawText(buf, 4, 138, 10, (Color){120, 220, 140, 255});

    int htx = 0, hty = 0;
    bool valid_tile = dh_tilemap_world_to_tile(&game->world.tilemap, game->input.world_mouse, &htx, &hty);
    if (valid_tile) {
        DHTile t = dh_tilemap_get_tile((DHTilemap *)&game->world.tilemap, htx, hty);
        snprintf(buf, sizeof(buf), "Hover Tile:(%d,%d) ID:%d Flags:%s",
                 htx, hty, t.visual_id, (t.flags & DH_COLLISION_SOLID) ? "SOLID" : "EMPTY");
    } else {
        snprintf(buf, sizeof(buf), "Hover Tile: OUT OF BOUNDS");
    }
    DrawText(buf, 4, 150, 10, (Color){200, 200, 220, 255});

    DrawText("A/D: Move | F2: Anim Demo | F3: Collision Debug", 4, 164, 10, (Color){140, 140, 160, 255});
}

void dh_renderer_toggle_fullscreen(DHRenderer *renderer)
{
    if (renderer == NULL) return;
    renderer->fullscreen = !renderer->fullscreen;
    ToggleFullscreen();
}

void dh_renderer_shutdown(DHRenderer *renderer)
{
    if (renderer == NULL) return;

    UnloadRenderTexture(renderer->target);
    CloseWindow();
    dh_log_info("Renderer shutdown complete");
}
