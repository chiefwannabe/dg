#include "dh/anim_demo.h"
#include "dh/logging.h"
#include <raylib.h>
#include <stdio.h>
#include <string.h>

static const char *SLIME_IDLE_PATH   = "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime1/With_shadow/Slime1_Idle_with_shadow.png";
static const char *SLIME_WALK_PATH   = "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime1/With_shadow/Slime1_Walk_with_shadow.png";
static const char *SLIME_RUN_PATH    = "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime1/With_shadow/Slime1_Run_with_shadow.png";
static const char *SLIME_ATTACK_PATH = "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime1/With_shadow/Slime1_Attack_with_shadow.png";
static const char *SLIME_HURT_PATH   = "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime1/With_shadow/Slime1_Hurt_with_shadow.png";
static const char *SLIME_DEATH_PATH  = "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime1/With_shadow/Slime1_Death_with_shadow.png";

bool dh_anim_demo_init(DHAnimDemo *demo, DHAssetManager *assets)
{
    if (demo == NULL || assets == NULL) {
        return false;
    }

    memset(demo, 0, sizeof(DHAnimDemo));
    dh_animation_controller_init(&demo->controller);

    demo->current_speed = 1.0f;
    demo->is_paused = false;

    // Direction mapping for 4-row sprite sheets (Row 0: Down, Row 1: Up, Row 2: Left, Row 3: Right)
    DHDirection dirs[4] = { DH_DIR_DOWN, DH_DIR_UP, DH_DIR_LEFT, DH_DIR_RIGHT };

    // 1. IDLE (6 frames, 64x64, looping)
    Texture2D tex_idle = dh_assets_load_texture(assets, SLIME_IDLE_PATH);
    if (tex_idle.id != 0) {
        for (int d = 0; d < 4; d++) {
            DHAnimationClip clip;
            dh_animation_clip_create_grid(&clip, "slime_idle", tex_idle, 0, d * 64, 64, 64, 6, 0.12f, true, true);
            dh_animation_controller_add_clip(&demo->controller, DH_ANIM_STATE_IDLE, dirs[d], &clip);
        }
    }

    // 2. WALK (8 frames, 64x64, looping)
    Texture2D tex_walk = dh_assets_load_texture(assets, SLIME_WALK_PATH);
    if (tex_walk.id != 0) {
        for (int d = 0; d < 4; d++) {
            DHAnimationClip clip;
            dh_animation_clip_create_grid(&clip, "slime_walk", tex_walk, 0, d * 64, 64, 64, 8, 0.10f, true, true);
            dh_animation_controller_add_clip(&demo->controller, DH_ANIM_STATE_WALK, dirs[d], &clip);
        }
    }

    // 3. RUN (8 frames, 64x64, looping)
    Texture2D tex_run = dh_assets_load_texture(assets, SLIME_RUN_PATH);
    if (tex_run.id != 0) {
        for (int d = 0; d < 4; d++) {
            DHAnimationClip clip;
            dh_animation_clip_create_grid(&clip, "slime_run", tex_run, 0, d * 64, 64, 64, 8, 0.08f, true, true);
            dh_animation_controller_add_clip(&demo->controller, DH_ANIM_STATE_RUN, dirs[d], &clip);
        }
    }

    // 4. ATTACK (10 frames, 64x64, one-shot)
    Texture2D tex_attack = dh_assets_load_texture(assets, SLIME_ATTACK_PATH);
    if (tex_attack.id != 0) {
        for (int d = 0; d < 4; d++) {
            DHAnimationClip clip;
            dh_animation_clip_create_grid(&clip, "slime_attack", tex_attack, 0, d * 64, 64, 64, 10, 0.07f, false, true);
            dh_animation_controller_add_clip(&demo->controller, DH_ANIM_STATE_ATTACK, dirs[d], &clip);
        }
    }

    // 5. HURT (5 frames, 64x64, one-shot)
    Texture2D tex_hurt = dh_assets_load_texture(assets, SLIME_HURT_PATH);
    if (tex_hurt.id != 0) {
        for (int d = 0; d < 4; d++) {
            DHAnimationClip clip;
            dh_animation_clip_create_grid(&clip, "slime_hurt", tex_hurt, 0, d * 64, 64, 64, 5, 0.10f, false, true);
            dh_animation_controller_add_clip(&demo->controller, DH_ANIM_STATE_HURT, dirs[d], &clip);
        }
    }

    // 6. DEATH (10 frames, 64x64, one-shot)
    Texture2D tex_death = dh_assets_load_texture(assets, SLIME_DEATH_PATH);
    if (tex_death.id != 0) {
        for (int d = 0; d < 4; d++) {
            DHAnimationClip clip;
            dh_animation_clip_create_grid(&clip, "slime_death", tex_death, 0, d * 64, 64, 64, 10, 0.09f, false, true);
            dh_animation_controller_add_clip(&demo->controller, DH_ANIM_STATE_DEATH, dirs[d], &clip);
        }
    }

    dh_animation_play(&demo->controller, DH_ANIM_STATE_IDLE, DH_DIR_DOWN, true);
    demo->loaded = true;

    dh_log_info("Animation test demo scene initialized successfully");
    return true;
}

void dh_anim_demo_update(DHAnimDemo *demo, const DHInput *input, float delta_time)
{
    if (demo == NULL || !demo->loaded) {
        return;
    }

    (void)input;

    // State switching with number keys 1-6
    if (IsKeyPressed(KEY_ONE))   dh_animation_play(&demo->controller, DH_ANIM_STATE_IDLE,   demo->controller.current_dir, true);
    if (IsKeyPressed(KEY_TWO))   dh_animation_play(&demo->controller, DH_ANIM_STATE_WALK,   demo->controller.current_dir, true);
    if (IsKeyPressed(KEY_THREE)) dh_animation_play(&demo->controller, DH_ANIM_STATE_RUN,    demo->controller.current_dir, true);
    if (IsKeyPressed(KEY_FOUR))  dh_animation_play(&demo->controller, DH_ANIM_STATE_ATTACK, demo->controller.current_dir, true);
    if (IsKeyPressed(KEY_FIVE))  dh_animation_play(&demo->controller, DH_ANIM_STATE_HURT,   demo->controller.current_dir, true);
    if (IsKeyPressed(KEY_SIX))   dh_animation_play(&demo->controller, DH_ANIM_STATE_DEATH,  demo->controller.current_dir, true);

    // Direction switching with Arrow keys or WASD
    if (IsKeyPressed(KEY_DOWN)  || IsKeyPressed(KEY_S)) dh_animation_play(&demo->controller, demo->controller.current_state, DH_DIR_DOWN,  false);
    if (IsKeyPressed(KEY_UP)    || IsKeyPressed(KEY_W)) dh_animation_play(&demo->controller, demo->controller.current_state, DH_DIR_UP,    false);
    if (IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A)) dh_animation_play(&demo->controller, demo->controller.current_state, DH_DIR_LEFT,  false);
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) dh_animation_play(&demo->controller, demo->controller.current_state, DH_DIR_RIGHT, false);

    // Pause / Play toggle
    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_P)) {
        demo->is_paused = !demo->is_paused;
        dh_animation_set_paused(&demo->controller, demo->is_paused);
    }

    // Speed adjustment
    if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) {
        demo->current_speed += 0.25f;
        if (demo->current_speed > 4.0f) demo->current_speed = 4.0f;
        dh_animation_set_speed(&demo->controller, demo->current_speed);
    }
    if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) {
        demo->current_speed -= 0.25f;
        if (demo->current_speed < 0.25f) demo->current_speed = 0.25f;
        dh_animation_set_speed(&demo->controller, demo->current_speed);
    }

    // Reset playback
    if (IsKeyPressed(KEY_R)) {
        dh_animation_reset(&demo->controller);
    }

    // Update animation timing
    dh_animation_update(&demo->controller, delta_time);
}

void dh_anim_demo_draw(const DHAnimDemo *demo, int screen_width, int screen_height)
{
    if (demo == NULL || !demo->loaded) {
        return;
    }

    // Render animated sprite at center of screen scaled 2x for visibility
    float dest_w = 64.0f * 2.0f;
    float dest_h = 64.0f * 2.0f;
    float center_x = (float)screen_width * 0.5f;
    float center_y = (float)screen_height * 0.5f;

    Rectangle dest = { center_x, center_y, dest_w, dest_h };
    Vector2 origin = { dest_w * 0.5f, dest_h * 0.5f };

    dh_animation_draw_pro(&demo->controller, dest, origin, 0.0f, WHITE);

    // Render HUD overlay
    char buf[128];
    int y_off = 10;
    DrawText("--- ANIMATION DEMO CONTROLS ---", 10, y_off, 10, (Color){ 255, 220, 100, 255 });
    y_off += 14;

    snprintf(buf, sizeof(buf), "State [1-6]: %s | Dir [WASD/Arrows]: %s",
             dh_anim_state_to_string(demo->controller.current_state),
             dh_direction_to_string(demo->controller.current_dir));
    DrawText(buf, 10, y_off, 10, WHITE);
    y_off += 14;

    snprintf(buf, sizeof(buf), "Frame: %d | Speed: %.2fx (+/-) | Status: %s",
             demo->controller.current_frame,
             demo->current_speed,
             demo->is_paused ? "PAUSED" : (dh_animation_is_finished(&demo->controller) ? "FINISHED" : "PLAYING"));
    DrawText(buf, 10, y_off, 10, (Color){ 100, 220, 255, 255 });
    y_off += 14;

    DrawText("Controls: [1-6] State | [WASD] Dir | [SPACE] Pause | [+/-] Speed | [R] Reset", 10, y_off, 10, (Color){ 180, 180, 180, 255 });
}

void dh_anim_demo_shutdown(DHAnimDemo *demo, DHAssetManager *assets)
{
    if (demo == NULL) {
        return;
    }

    if (assets != NULL) {
        dh_assets_unload_texture(assets, SLIME_IDLE_PATH);
        dh_assets_unload_texture(assets, SLIME_WALK_PATH);
        dh_assets_unload_texture(assets, SLIME_RUN_PATH);
        dh_assets_unload_texture(assets, SLIME_ATTACK_PATH);
        dh_assets_unload_texture(assets, SLIME_HURT_PATH);
        dh_assets_unload_texture(assets, SLIME_DEATH_PATH);
    }

    demo->loaded = false;
    dh_log_info("Animation test demo scene shut down");
}
