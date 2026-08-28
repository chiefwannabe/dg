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

    char plat_info[128];
    snprintf(plat_info, sizeof(plat_info), "HP:%d/%d LV:%d XP:%d/%d GOLD:%d",
             game->player.hp, game->player.max_hp, game->player.level,
             game->player.xp, game->player.xp_to_next_level, game->player.gold_count);
    DrawText(plat_info, 110, 4, 10, (Color){255, 220, 80, 255});

    /* Level Up Banner Notification */
    if (game->player.level_up_notify_timer > 0.0f) {
        DrawRectangle(50, 46, 220, 24, (Color){15, 15, 25, 230});
        DrawRectangleLines(50, 46, 220, 24, (Color){255, 220, 80, 255});
        int text_w = MeasureText("LEVEL UP! (+1 HP, +1 DMG)", 10);
        DrawText("LEVEL UP! (+1 HP, +1 DMG)", (renderer->virtual_width - text_w) / 2, 53, 10, (Color){255, 220, 80, 255});
    }

    /* Boss Battle Health Bar HUD Overlay */
    dh_boss_manager_draw_hud(&game->boss_mgr, renderer->virtual_width, renderer->virtual_height);

    /* Inventory & Shop Overlays */
    dh_inventory_draw_ui(&game->inventory, renderer->virtual_width, renderer->virtual_height);
    dh_shop_draw_ui(&game->shop, &game->player, renderer->virtual_width, renderer->virtual_height);

    /* F8 Inventory & Save Debug Overlay */
    if (game->show_inventory_debug) {
        DrawRectangle(10, 24, 200, 48, (Color){ 10, 10, 15, 230 });
        DrawRectangleLines(10, 24, 200, 48, (Color){ 80, 180, 220, 255 });
        char dbg_buf[128];
        snprintf(dbg_buf, sizeof(dbg_buf), "INV Count: %d/24 | WPN: +%d | ARM: +%d",
                 game->inventory.count,
                 dh_inventory_get_total_attack_bonus(&game->inventory),
                 dh_inventory_get_total_hp_bonus(&game->inventory));
        DrawText(dbg_buf, 14, 28, 8, (Color){ 255, 220, 100, 255 });
        snprintf(dbg_buf, sizeof(dbg_buf), "Save Path: %s (STATUS: ACTIVE)", dh_save_get_path());
        DrawText(dbg_buf, 14, 40, 8, (Color){ 100, 220, 120, 255 });
    }

    if (game->state == DH_STATE_PAUSED) {
        DrawRectangle(0, 0, renderer->virtual_width, renderer->virtual_height, (Color){ 0, 0, 0, 140 });
        int font_sz = 20;
        int text_w = MeasureText("PAUSED", font_sz);
        DrawText("PAUSED", (renderer->virtual_width - text_w) / 2, 70, font_sz, (Color){ 255, 220, 100, 255 });
        text_w = MeasureText("Press P or ESC to Resume", 10);
        DrawText("Press P or ESC to Resume", (renderer->virtual_width - text_w) / 2, 98, 10, RAYWHITE);
    }

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

    DrawText("A/D: Move | SPACE: Attack | TAB: Inventory | E: Shop | F2-F8: Debug", 4, 164, 10, (Color){140, 140, 160, 255});
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
