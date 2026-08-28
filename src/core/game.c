#include "dh/game.h"
#include "dh/logging.h"
#include <raylib.h>
#include <math.h>
#include <stddef.h>
#include <string.h>

bool dh_game_init(DHGame *game, DHConfig config)
{
    if (game == NULL) {
        return false;
    }

    dh_log_init(DH_LOG_LEVEL_DEBUG);
    dh_log_info("Initializing %s v%s (2D Side-Scrolling Platformer Engine)...", config.title, config.version);

    game->config = config;
    game->state = DH_STATE_BOOT;
    game->running = true;
    game->demo_mode = false;
    game->show_collision_debug = false;
    game->show_enemy_debug = false;

    dh_time_init(&game->time);

    if (!dh_assets_init(&game->assets)) {
        dh_log_error("Failed to initialize asset manager");
        return false;
    }

    if (!dh_renderer_init(&game->renderer, &game->config)) {
        dh_log_error("Failed to initialize renderer");
        dh_assets_shutdown(&game->assets);
        return false;
    }

    /* Initialize Endless Platformer World (16px tile size) */
    if (!dh_world_init(&game->world, 16, &game->assets)) {
        dh_log_error("Failed to initialize endless platformer world");
        dh_renderer_shutdown(&game->renderer);
        dh_assets_shutdown(&game->assets);
        return false;
    }

    /* Initialize platformer player with Swordsman character assets */
    dh_test_player_init(&game->player, game->world.spawn_pos, &game->assets);

    /* Initialize Enemy Manager & Pool */
    if (!dh_enemy_manager_init(&game->enemies, &game->assets)) {
        dh_log_error("Failed to initialize enemy manager");
        dh_test_player_shutdown(&game->player, &game->assets);
        dh_world_shutdown(&game->world);
        dh_renderer_shutdown(&game->renderer);
        dh_assets_shutdown(&game->assets);
        return false;
    }

    /* Initialize Combat System */
    dh_combat_init(&game->combat);

    /* Initialize Loot Pickup Manager */
    dh_loot_init(&game->loot);
    game->show_loot_debug = false;

    /* Initialize Boss System Manager */
    dh_boss_manager_init(&game->boss_mgr, &game->assets);
    game->show_boss_debug = false;

    /* Initialize Inventory & Shop */
    dh_inventory_init(&game->inventory);
    dh_shop_init(&game->shop);
    game->show_inventory_debug = false;

    /* Attempt loading existing session save */
    dh_load_game(&game->player, &game->inventory);

    /* Initialize 2D side-scrolling camera following player horizontally */
    dh_camera_init(&game->camera, (float)game->renderer.virtual_width, (float)game->renderer.virtual_height);
    float init_cam_x = game->player.position.x;
    if (init_cam_x < (float)game->renderer.virtual_width * 0.5f) {
        init_cam_x = (float)game->renderer.virtual_width * 0.5f;
    }
    dh_camera_set_target(&game->camera, (Vector2){ init_cam_x, (float)game->renderer.virtual_height * 0.5f });

    dh_input_init(&game->input);

    /* Initialize optional animation test demo */
    if (!dh_anim_demo_init(&game->anim_demo, &game->assets)) {
        dh_log_error("Failed to initialize animation demo scene");
        dh_enemy_manager_shutdown(&game->enemies, &game->assets);
        dh_test_player_shutdown(&game->player, &game->assets);
        dh_world_shutdown(&game->world);
        dh_renderer_shutdown(&game->renderer);
        dh_assets_shutdown(&game->assets);
        return false;
    }

    /* Initialize Audio System */
    dh_audio_init(&game->audio);

    /* Attempt loading existing session save */
    game->has_save_file = dh_load_game(&game->player, &game->inventory);
    game->menu_selected_idx = game->has_save_file ? 1 : 0;

    game->state = DH_STATE_MENU;
    dh_log_info("Dungeon Hunter v1.0.0 Engine initialized. Initial State: MAIN MENU.");
    return true;
}

void dh_game_update(DHGame *game)
{
    if (game == NULL || !game->running) {
        return;
    }

    if (WindowShouldClose()) {
        game->running = false;
        game->state = DH_STATE_EXIT;
        return;
    }

    dh_time_update(&game->time);

    Camera2D rcam = dh_camera_get_raylib_camera(&game->camera);
    dh_input_update(&game->input,
                    game->renderer.virtual_width,
                    game->renderer.virtual_height,
                    game->renderer.scale,
                    game->renderer.offset_x,
                    game->renderer.offset_y,
                    rcam);

    /* Main Menu Navigation */
    if (game->state == DH_STATE_MENU) {
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            game->menu_selected_idx--;
            if (game->menu_selected_idx < 0) game->menu_selected_idx = 2;
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            game->menu_selected_idx++;
            if (game->menu_selected_idx > 2) game->menu_selected_idx = 0;
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_KP_ENTER)) {
            if (game->menu_selected_idx == 0) {
                /* New Game */
                game->state = DH_STATE_PLAYING;
                dh_log_info("Started NEW GAME.");
            } else if (game->menu_selected_idx == 1) {
                /* Continue Game */
                game->state = DH_STATE_PLAYING;
                dh_log_info("CONTINUED existing game.");
            } else if (game->menu_selected_idx == 2) {
                /* Quit */
                game->running = false;
                game->state = DH_STATE_EXIT;
            }
        }
        return;
    }

    /* Pause Menu Shortcuts */
    if (dh_input_is_pressed(&game->input, DH_INPUT_PAUSE)) {
        if (game->inventory.show_ui || game->shop.show_ui) {
            game->inventory.show_ui = false;
            game->shop.show_ui = false;
        } else if (game->state == DH_STATE_PLAYING) {
            game->state = DH_STATE_PAUSED;
            dh_log_info("Game State: PAUSED");
        } else if (game->state == DH_STATE_PAUSED) {
            game->state = DH_STATE_PLAYING;
            dh_log_info("Game State: RESUMED");
        }
    }

    if (IsKeyPressed(KEY_F11)) {
        dh_renderer_toggle_fullscreen(&game->renderer);
    }

    if (IsKeyPressed(KEY_F2)) {
        game->demo_mode = !game->demo_mode;
        dh_log_info("Toggled Animation Demo Mode: %s", game->demo_mode ? "ON" : "OFF");
    }

    if (IsKeyPressed(KEY_F3)) {
        game->show_collision_debug = !game->show_collision_debug;
        dh_log_info("Toggled Collision Debug Overlay: %s", game->show_collision_debug ? "ON" : "OFF");
    }

    if (IsKeyPressed(KEY_F4)) {
        game->show_enemy_debug = !game->show_enemy_debug;
        dh_log_info("Toggled Enemy Debug Overlay: %s", game->show_enemy_debug ? "ON" : "OFF");
    }

    if (IsKeyPressed(KEY_F5)) {
        game->show_combat_debug = !game->show_combat_debug;
        dh_log_info("Toggled Combat Debug Overlay: %s", game->show_combat_debug ? "ON" : "OFF");
    }

    if (IsKeyPressed(KEY_F6)) {
        game->show_loot_debug = !game->show_loot_debug;
        dh_log_info("Toggled Loot/Progression Debug: %s", game->show_loot_debug ? "ON" : "OFF");
    }

    if (IsKeyPressed(KEY_F7)) {
        game->show_boss_debug = !game->show_boss_debug;
        game->boss_mgr.show_boss_debug = game->show_boss_debug;
        dh_log_info("Toggled Boss Debug Overlay: %s", game->show_boss_debug ? "ON" : "OFF");
    }

    if (IsKeyPressed(KEY_F8)) {
        game->show_inventory_debug = !game->show_inventory_debug;
        dh_log_info("Toggled Inventory/Save Debug Overlay: %s", game->show_inventory_debug ? "ON" : "OFF");
    }

    if (IsKeyPressed(KEY_TAB)) {
        game->inventory.show_ui = !game->inventory.show_ui;
        if (game->inventory.show_ui) game->shop.show_ui = false;
    }

    if (IsKeyPressed(KEY_E)) {
        int milestone = (int)floorf(game->camera.position.x / 1500.0f);
        if (milestone <= 0) milestone = 1;
        float shop_x = (float)milestone * 1500.0f + 120.0f;

        if (fabsf(game->player.position.x - shop_x) <= SHOP_INTERACTION_RADIUS) {
            game->shop.show_ui = !game->shop.show_ui;
            if (game->shop.show_ui) game->inventory.show_ui = false;
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        game->inventory.show_ui = false;
        game->shop.show_ui = false;
    }

    /* Number key selection for Inventory & Shop */
    for (int k = 0; k < 9; k++) {
        if (IsKeyPressed(KEY_ONE + k)) {
            if (game->inventory.show_ui && k < game->inventory.count) {
                game->inventory.selected_idx = k;
            } else if (game->shop.show_ui && k < SHOP_CATALOG_SIZE) {
                game->shop.selected_idx = k;
            }
        }
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
        if (game->inventory.show_ui && game->inventory.count > 0) {
            dh_inventory_equip_toggle(&game->inventory, game->inventory.selected_idx);
            dh_save_game(&game->player, &game->inventory);
        } else if (game->shop.show_ui) {
            if (dh_shop_buy_selected(&game->shop, &game->player, &game->inventory)) {
                dh_save_game(&game->player, &game->inventory);
            }
        }
    }

    /* Simulation updates ONLY run when PLAYING */
    if (game->state == DH_STATE_PLAYING) {
        if (game->demo_mode) {
            dh_anim_demo_update(&game->anim_demo, &game->input, game->time.delta_time);
        } else {
            /* Input for player movement only active when UI popups are closed */
            DHInput input_sub = game->input;
            if (game->inventory.show_ui || game->shop.show_ui) {
                memset(&input_sub, 0, sizeof(DHInput));
            }

            /* Update 2D Platformer Player (Horizontal A/D movement, gravity, floor snapping) */
            dh_test_player_update(&game->player, &input_sub, &game->world.tilemap, game->time.delta_time);

            /* Update Shop System Timers */
            dh_shop_update(&game->shop, &game->player, &game->inventory, game->time.delta_time);

            /* Trigger enemy spawning for active world chunks */
            int center_chunk = (int)floorf(game->camera.position.x / (float)(DH_CHUNK_WIDTH * game->world.tilemap.tile_size));
            for (int c = center_chunk - 1; c <= center_chunk + 4; c++) {
                if (c >= 0) {
                    dh_enemy_manager_on_chunk_generated(&game->enemies, c, game->world.tilemap.tile_size);
                }
            }

            /* Update Enemy Pool (Detection, Pursuit AI, Gravity, Wall/Floor Collision, Animation) */
            dh_enemy_manager_update(&game->enemies, game->player.position, &game->world.tilemap, game->camera.position.x, game->time.delta_time);

            /* Update Boss Spawning & Boss Battle System */
            dh_boss_manager_check_spawning(&game->boss_mgr, game->camera.position.x, (float)game->world.tilemap.tile_size);
            dh_boss_manager_update(&game->boss_mgr, &game->player, &game->world.tilemap, &game->loot, game->time.delta_time);

            /* Update Combat System (Sword Hitbox, Single-Hit Register, Enemy HP, Knockback, XP Award & Loot Spawning) */
            dh_combat_update(&game->combat, &game->player, &game->enemies, &game->loot, game->time.delta_time);

            /* Update Loot Pickups (Bobbing animation, player overlap collision & Gold collection) */
            float p_half_w = game->player.bbox_size.x * 0.5f;
            float p_half_h = game->player.bbox_size.y * 0.5f;
            Rectangle p_box = { game->player.position.x - p_half_w, game->player.position.y - p_half_h, game->player.bbox_size.x, game->player.bbox_size.y };
            dh_loot_update(&game->loot, p_box, &game->player.gold_count, game->time.delta_time);

            /* Side-scrolling camera follows player horizontally */
            float target_cam_x = game->player.position.x;
            float min_cam_x = (float)game->renderer.virtual_width * 0.5f;
            if (target_cam_x < min_cam_x) {
                target_cam_x = min_cam_x;
            }

            dh_camera_set_target(&game->camera, (Vector2){ target_cam_x, (float)game->renderer.virtual_height * 0.5f });
        }

        /* Reset camera shortcut */
        if (dh_input_is_pressed(&game->input, DH_INPUT_RESET_CAM)) {
            float reset_x = game->player.position.x;
            if (reset_x < (float)game->renderer.virtual_width * 0.5f) reset_x = (float)game->renderer.virtual_width * 0.5f;
            dh_camera_set_target(&game->camera, (Vector2){ reset_x, (float)game->renderer.virtual_height * 0.5f });
        }

        dh_camera_update(&game->camera, game->time.delta_time);
    }
}

void dh_game_draw(DHGame *game)
{
    if (game == NULL || !game->running) {
        return;
    }

    if (game->state == DH_STATE_MENU) {
        DrawRectangle(0, 0, game->renderer.virtual_width, game->renderer.virtual_height, (Color){ 10, 10, 16, 255 });
        DrawText("DUNGEON HUNTER", (game->renderer.virtual_width - MeasureText("DUNGEON HUNTER", 20)) / 2, 35, 20, (Color){ 255, 220, 80, 255 });
        DrawText("Endless Side-Scrolling Platformer v1.0.0", (game->renderer.virtual_width - MeasureText("Endless Side-Scrolling Platformer v1.0.0", 8)) / 2, 60, 8, (Color){ 180, 180, 200, 255 });

        const char *opt0 = "NEW GAME";
        const char *opt1 = game->has_save_file ? "CONTINUE" : "[NO SAVE]";
        const char *opt2 = "QUIT";

        Color c0 = (game->menu_selected_idx == 0) ? (Color){ 255, 220, 80, 255 } : RAYWHITE;
        Color c1 = (game->menu_selected_idx == 1) ? (Color){ 255, 220, 80, 255 } : (game->has_save_file ? RAYWHITE : (Color){ 100, 100, 120, 255 });
        Color c2 = (game->menu_selected_idx == 2) ? (Color){ 255, 220, 80, 255 } : RAYWHITE;

        DrawText(opt0, (game->renderer.virtual_width - MeasureText(opt0, 12)) / 2, 90, 12, c0);
        DrawText(opt1, (game->renderer.virtual_width - MeasureText(opt1, 12)) / 2, 110, 12, c1);
        DrawText(opt2, (game->renderer.virtual_width - MeasureText(opt2, 12)) / 2, 130, 12, c2);

        DrawText("A/D: Move | SPACE/ENTER: Select | TAB: Inv | E: Shop", (game->renderer.virtual_width - MeasureText("A/D: Move | SPACE/ENTER: Select | TAB: Inv | E: Shop", 8)) / 2, 160, 8, (Color){ 140, 140, 160, 255 });
        dh_renderer_end_offscreen(&game->renderer);
        dh_renderer_draw_framebuffer(&game->renderer);
        EndDrawing();
        return;
    }

    Camera2D rcam = dh_camera_get_raylib_camera(&game->camera);
    BeginMode2D(rcam);

    /* Draw Endless Side-Scrolling Dungeon Tiles & Props */
    dh_world_draw(&game->world, &game->camera, game->renderer.virtual_width, game->renderer.virtual_height, game->show_collision_debug, &game->visible_tiles_count);

    if (!game->demo_mode) {
        /* Draw World Merchant NPC Stand */
        dh_shop_draw_world(&game->shop, game->camera.position.x, (float)game->world.tilemap.tile_size, game->player.position);

        /* Draw Dungeon Loot Pickups */
        dh_loot_draw(&game->loot);

        /* Draw Active Enemies */
        dh_enemy_manager_draw(&game->enemies);

        /* Draw Boss Character */
        dh_boss_manager_draw(&game->boss_mgr);

        /* Draw Swordsman Hero Player Character */
        dh_test_player_draw(&game->player);

        /* Draw Enemy Debug Overlay if enabled */
        if (game->show_enemy_debug) {
            dh_enemy_manager_draw_debug(&game->enemies, game->player.position);
        }

        /* Draw Combat Debug Overlay if enabled */
        if (game->show_combat_debug) {
            dh_combat_draw_debug(&game->combat, &game->player, &game->enemies);
        }

        /* Draw Loot Debug Overlay if enabled */
        if (game->show_loot_debug) {
            dh_loot_draw_debug(&game->loot);
        }

        /* Draw Boss Debug Overlay if enabled */
        if (game->show_boss_debug) {
            dh_boss_manager_draw_debug(&game->boss_mgr);
        }
    }

    /* Highlight hovered tile under mouse cursor */
    int htx = 0, hty = 0;
    if (dh_tilemap_world_to_tile(&game->world.tilemap, game->input.world_mouse, &htx, &hty)) {
        Vector2 tile_wpos = dh_tilemap_tile_to_world(&game->world.tilemap, htx, hty);
        bool is_solid = dh_tilemap_is_solid(&game->world.tilemap, htx, hty);
        Color border_col = is_solid ? (Color){255, 80, 80, 255} : (Color){255, 220, 100, 255};
        DrawRectangleLines((int)tile_wpos.x, (int)tile_wpos.y, game->world.tilemap.tile_size, game->world.tilemap.tile_size, border_col);
    }

    EndMode2D();

    if (game->demo_mode) {
        dh_anim_demo_draw(&game->anim_demo, game->renderer.virtual_width, game->renderer.virtual_height);
    } else {
        dh_renderer_draw_foundation_ui(&game->renderer, game, game->visible_tiles_count);
    }

    dh_renderer_end_offscreen(&game->renderer);

    dh_renderer_draw_framebuffer(&game->renderer);
    EndDrawing();
}

void dh_game_shutdown(DHGame *game)
{
    if (game == NULL) {
        return;
    }

    dh_log_info("Shutting down %s...", game->config.title);
    dh_audio_shutdown(&game->audio);
    dh_boss_manager_shutdown(&game->boss_mgr, &game->assets);
    dh_enemy_manager_shutdown(&game->enemies, &game->assets);
    dh_test_player_shutdown(&game->player, &game->assets);
    dh_anim_demo_shutdown(&game->anim_demo, &game->assets);
    dh_world_shutdown(&game->world);
    dh_assets_shutdown(&game->assets);
    dh_renderer_shutdown(&game->renderer);
    game->running = false;
    game->state = DH_STATE_EXIT;
}
