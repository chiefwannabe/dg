#include "dh/game.h"
#include "dh/logging.h"
#include <raylib.h>
#include <stddef.h>

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

    dh_time_init(&game->time);

    if (!dh_assets_init(&game->assets)) {
        dh_log_error("Failed to initialize asset manager");
        return false;
    }

    if (!dh_renderer_init(&game->renderer, &game->config)) {
        dh_log_error("Failed to initialize renderer");
        return false;
    }

    /* Initialize Endless Platformer World (16px tile size) */
    if (!dh_world_init(&game->world, 16, &game->assets)) {
        dh_log_error("Failed to initialize endless platformer world");
        return false;
    }

    /* Initialize temporary platformer test player marker */
    dh_test_player_init(&game->player, game->world.spawn_pos);

    /* Initialize 2D side-scrolling camera following player horizontally */
    dh_camera_init(&game->camera, (float)game->renderer.virtual_width, (float)game->renderer.virtual_height);
    float init_cam_x = game->player.position.x;
    if (init_cam_x < (float)game->renderer.virtual_width * 0.5f) {
        init_cam_x = (float)game->renderer.virtual_width * 0.5f;
    }
    dh_camera_set_target(&game->camera, (Vector2){ init_cam_x, (float)game->renderer.virtual_height * 0.5f });

    dh_input_init(&game->input);

    /* Initialize optional animation test demo */
    dh_anim_demo_init(&game->anim_demo, &game->assets);

    game->state = DH_STATE_PLAYING;
    dh_log_info("Side-Scrolling Platformer Foundation initialized successfully. Controls: A = Left, D = Right, F2 = Anim Demo, F3 = Collision Debug.");
    return true;
}

void dh_game_update(DHGame *game)
{
    if (game == NULL || !game->running) {
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

    if (WindowShouldClose() || dh_input_is_pressed(&game->input, DH_INPUT_PAUSE)) {
        if (game->state == DH_STATE_PLAYING) {
            game->state = DH_STATE_PAUSED;
        } else {
            game->running = false;
            game->state = DH_STATE_EXIT;
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

    if (game->demo_mode) {
        dh_anim_demo_update(&game->anim_demo, &game->input, game->time.delta_time);
    } else {
        /* Update 2D Platformer Player (Horizontal A/D movement, gravity, floor snapping) */
        dh_test_player_update(&game->player, &game->input, &game->world.tilemap, game->time.delta_time);

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

void dh_game_draw(DHGame *game)
{
    if (game == NULL || !game->running) {
        return;
    }

    dh_renderer_begin_offscreen(&game->renderer);

    Camera2D rcam = dh_camera_get_raylib_camera(&game->camera);
    BeginMode2D(rcam);

    /* Draw Endless Side-Scrolling Dungeon Tiles & Props */
    dh_world_draw(&game->world, &game->camera, game->renderer.virtual_width, game->renderer.virtual_height, game->show_collision_debug, &game->visible_tiles_count);

    if (!game->demo_mode) {
        /* Draw temporary platformer player marker */
        dh_test_player_draw(&game->player);
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
    dh_anim_demo_shutdown(&game->anim_demo, &game->assets);
    dh_world_shutdown(&game->world);
    dh_assets_shutdown(&game->assets);
    dh_renderer_shutdown(&game->renderer);
    game->running = false;
    game->state = DH_STATE_EXIT;
}
