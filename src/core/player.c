#include "dh/player.h"
#include "dh/logging.h"
#include <math.h>
#include <stddef.h>
#include <string.h>

static const char *SWORDSMAN_IDLE_PATH   = "assets/craftpix-net-180537-free-swordsman-1-3-level-pixel-top-down-sprite-character/Tiled_files/Swordsman1/Swordsman_lvl1_Idle_with_shadow.png";
static const char *SWORDSMAN_RUN_PATH    = "assets/craftpix-net-180537-free-swordsman-1-3-level-pixel-top-down-sprite-character/Tiled_files/Swordsman1/Swordsman_lvl1_Run_with_shadow.png";
static const char *SWORDSMAN_ATTACK_PATH = "assets/craftpix-net-180537-free-swordsman-1-3-level-pixel-top-down-sprite-character/Tiled_files/Swordsman1/Swordsman_lvl1_attack_with_shadow.png";

void dh_test_player_init(DHTestPlayer *player, Vector2 start_pos, DHAssetManager *assets)
{
    if (player == NULL) return;

    memset(player, 0, sizeof(DHTestPlayer));
    player->position = start_pos;
    player->respawn_pos = start_pos;
    player->velocity = (Vector2){ 0.0f, 0.0f };
    player->speed = 100.0f;     /* 100 pixels/sec horizontal movement */
    player->gravity = 500.0f;   /* Gravity acceleration px/s^2 */
    player->bbox_size = (Vector2){ 12.0f, 16.0f };
    player->grounded = false;
    player->color = (Color){ 80, 210, 255, 255 };
    player->facing_dir = DH_DIR_SIDE_RIGHT;
    player->anim_state = DH_ANIM_STATE_IDLE;
    player->loaded = false;
    player->gold_count = 0;

    player->max_hp = PLAYER_MAX_HEALTH;
    player->hp = player->max_hp;
    player->invuln_timer = 0.0f;
    player->is_dead = false;
    player->death_timer = 0.0f;

    player->level = 1;
    player->xp = 0;
    player->xp_to_next_level = 100;
    player->attack_damage = 1;
    player->level_up_notify_timer = 0.0f;

    dh_animation_controller_init(&player->anim_controller);

    if (assets != NULL) {
        /* Row 3 (start_y = 192) contains right-facing side platformer sprites */
        Texture2D tex_idle = dh_assets_load_texture(assets, SWORDSMAN_IDLE_PATH);
        if (tex_idle.id != 0) {
            DHAnimationClip clip_idle;
            if (dh_animation_clip_create_grid(&clip_idle, "swordsman_idle", tex_idle, 0, 192, 64, 64, 12, 0.08f, true, true)) {
                dh_animation_controller_add_clip(&player->anim_controller, DH_ANIM_STATE_IDLE, DH_DIR_SIDE_RIGHT, &clip_idle);
                dh_animation_controller_add_clip(&player->anim_controller, DH_ANIM_STATE_IDLE, DH_DIR_RIGHT, &clip_idle);
            }
        }

        Texture2D tex_run = dh_assets_load_texture(assets, SWORDSMAN_RUN_PATH);
        if (tex_run.id != 0) {
            DHAnimationClip clip_run;
            if (dh_animation_clip_create_grid(&clip_run, "swordsman_run", tex_run, 0, 192, 64, 64, 8, 0.08f, true, true)) {
                dh_animation_controller_add_clip(&player->anim_controller, DH_ANIM_STATE_RUN, DH_DIR_SIDE_RIGHT, &clip_run);
                dh_animation_controller_add_clip(&player->anim_controller, DH_ANIM_STATE_WALK, DH_DIR_SIDE_RIGHT, &clip_run);
                dh_animation_controller_add_clip(&player->anim_controller, DH_ANIM_STATE_RUN, DH_DIR_RIGHT, &clip_run);
                dh_animation_controller_add_clip(&player->anim_controller, DH_ANIM_STATE_WALK, DH_DIR_RIGHT, &clip_run);
            }
        }

        Texture2D tex_atk = dh_assets_load_texture(assets, SWORDSMAN_ATTACK_PATH);
        if (tex_atk.id != 0) {
            DHAnimationClip clip_atk;
            if (dh_animation_clip_create_grid(&clip_atk, "swordsman_attack", tex_atk, 0, 192, 64, 64, 8, 0.05f, false, true)) {
                dh_animation_controller_add_clip(&player->anim_controller, DH_ANIM_STATE_ATTACK, DH_DIR_SIDE_RIGHT, &clip_atk);
                dh_animation_controller_add_clip(&player->anim_controller, DH_ANIM_STATE_ATTACK, DH_DIR_RIGHT, &clip_atk);
            }
        }

        dh_animation_play(&player->anim_controller, DH_ANIM_STATE_IDLE, DH_DIR_SIDE_RIGHT, true);
        player->loaded = true;
        dh_log_info("Swordsman hero player initialized (Level %d, HP: %d/%d, Damage: %d)",
                    player->level, player->hp, player->max_hp, player->attack_damage);
    }
}

void dh_test_player_add_xp(DHTestPlayer *player, int xp_amount)
{
    if (player == NULL || xp_amount <= 0) return;

    player->xp += xp_amount;
    dh_log_info("Player gained +%d XP! (Total XP: %d/%d)", xp_amount, player->xp, player->xp_to_next_level);

    while (player->xp >= player->xp_to_next_level) {
        player->xp -= player->xp_to_next_level;
        player->level += 1;
        player->max_hp += 1;
        player->hp = player->max_hp; /* Restore HP to new maximum */
        player->attack_damage += 1;
        player->xp_to_next_level += 50;
        player->level_up_notify_timer = 2.0f;

        dh_log_info("LEVEL UP! Reached Level %d! Max HP: %d, Damage: %d, Next XP: %d",
                    player->level, player->max_hp, player->attack_damage, player->xp_to_next_level);
    }
}

void dh_test_player_update(DHTestPlayer *player, const DHInput *input, DHTilemap *tilemap, float dt)
{
    if (player == NULL || input == NULL || tilemap == NULL || dt <= 0.0f) {
        return;
    }

    /* Update invulnerability countdown */
    if (player->invuln_timer > 0.0f) {
        player->invuln_timer -= dt;
        if (player->invuln_timer < 0.0f) player->invuln_timer = 0.0f;
    }

    /* Update Level Up Banner Timer */
    if (player->level_up_notify_timer > 0.0f) {
        player->level_up_notify_timer -= dt;
        if (player->level_up_notify_timer < 0.0f) player->level_up_notify_timer = 0.0f;
    }

    float half_w = player->bbox_size.x * 0.5f;
    float half_h = player->bbox_size.y * 0.5f;
    int ts = tilemap->tile_size > 0 ? tilemap->tile_size : 16;

    /* Handle Player Death State & Safe Respawn */
    if (player->is_dead) {
        player->velocity = (Vector2){ 0.0f, 0.0f };
        player->is_attacking = false;
        player->death_timer -= dt;

        if (player->death_timer <= 0.0f) {
            player->is_dead = false;
            player->hp = player->max_hp;
            player->invuln_timer = 1.0f; /* Safe spawn protection */

            /* Respawn on top of floor row 10 */
            player->position = (Vector2){ player->position.x, 10.0f * (float)ts - half_h };
            player->velocity = (Vector2){ 0.0f, 0.0f };

            dh_log_info("Player RESPAWNED safely! HP: %d/%d, Gold preserved: %d",
                        player->hp, player->max_hp, player->gold_count);
        }
        return;
    }

    /* Update attack cooldown timer */
    if (player->attack_cooldown > 0.0f) {
        player->attack_cooldown -= dt;
        if (player->attack_cooldown < 0.0f) player->attack_cooldown = 0.0f;
    }

    /* Handle Attack Input Trigger (SPACE key) */
    if (dh_input_is_pressed(input, DH_INPUT_ACTION_PRIMARY) && !player->is_attacking && player->attack_cooldown <= 0.0f) {
        player->is_attacking = true;
        player->attack_timer = 0.0f;
        memset(player->attack_hit_registered, 0, sizeof(player->attack_hit_registered));
        dh_animation_play(&player->anim_controller, DH_ANIM_STATE_ATTACK, player->facing_dir, true);
    }

    /* 1. Apply Polished Horizontal Acceleration & Deceleration (A/D only, W/S do nothing) */
    float target_vx = input->move_x * player->speed;
    float accel = (input->move_x != 0.0f) ? 600.0f : 800.0f; /* Acceleration vs Deceleration */

    if (player->velocity.x < target_vx) {
        player->velocity.x += accel * dt;
        if (player->velocity.x > target_vx) player->velocity.x = target_vx;
    } else if (player->velocity.x > target_vx) {
        player->velocity.x -= accel * dt;
        if (player->velocity.x < target_vx) player->velocity.x = target_vx;
    }

    /* 2. Apply Gravity to Vertical Velocity */
    player->velocity.y += player->gravity * dt;
    if (player->velocity.y > 400.0f) {
        player->velocity.y = 400.0f; /* Terminal velocity cap */
    }

    Vector2 current_pos = player->position;

    /* 3. Resolve Horizontal X Movement & Wall Collision */
    float target_x = current_pos.x + player->velocity.x * dt;
    /* Inset height by 0.2px to avoid catching floor or ceiling seam joints */
    Rectangle box_x = {
        target_x - half_w,
        current_pos.y - half_h + 0.1f,
        player->bbox_size.x,
        player->bbox_size.y - 0.2f
    };

    if (dh_tilemap_check_rect_collision(tilemap, box_x)) {
        if (player->velocity.x > 0.0f) {
            /* Moving right into a wall -> snap left edge of wall */
            int wall_tx = (int)floorf((target_x + half_w) / (float)ts);
            current_pos.x = (float)(wall_tx * ts) - half_w - 0.01f;
        } else if (player->velocity.x < 0.0f) {
            /* Moving left into a wall -> snap right edge of wall */
            int wall_tx = (int)floorf((target_x - half_w) / (float)ts);
            current_pos.x = (float)((wall_tx + 1) * ts) + half_w + 0.01f;
        }
        player->velocity.x = 0.0f;
    } else {
        current_pos.x = target_x;
    }

    /* 4. Resolve Vertical Y Movement & Floor/Ceiling Collision */
    player->grounded = false;
    float target_y = current_pos.y + player->velocity.y * dt;
    /* Inset width by 0.2px so side walls are NEVER detected as floors/ceilings */
    Rectangle box_y = {
        current_pos.x - half_w + 0.1f,
        target_y - half_h,
        player->bbox_size.x - 0.2f,
        player->bbox_size.y
    };

    if (dh_tilemap_check_rect_collision(tilemap, box_y)) {
        if (player->velocity.y > 0.0f) {
            /* Falling down onto a floor/platform */
            player->grounded = true;
            player->velocity.y = 0.0f;
            int foot_ty = (int)floorf((target_y + half_h) / (float)ts);
            current_pos.y = (float)(foot_ty * ts) - half_h;
        } else if (player->velocity.y < 0.0f) {
            /* Moving up into a ceiling */
            player->velocity.y = 0.0f;
            int head_ty = (int)floorf((target_y - half_h) / (float)ts);
            current_pos.y = (float)((head_ty + 1) * ts) + half_h;
        }
    } else {
        current_pos.y = target_y;
    }

    /* 5. Left World Boundary (Player cannot move left past X = half_w) */
    if (current_pos.x < half_w) {
        current_pos.x = half_w;
        player->velocity.x = 0.0f;
    }

    /* 6. Fall Safety Mechanism (Safety check if player falls out of world bounds) */
    if (current_pos.y > 280.0f) {
        current_pos = (Vector2){ current_pos.x, 10.0f * (float)ts - half_h };
        player->velocity = (Vector2){ 0.0f, 0.0f };
        dh_log_info("Player fall safety triggered: respawned to safe ground position");
    }

    player->position = current_pos;

    /* 7. Update Character Facing Direction & Animation Priority */
    if (input->move_x > 0.0f) {
        player->facing_dir = DH_DIR_SIDE_RIGHT;
    } else if (input->move_x < 0.0f) {
        player->facing_dir = DH_DIR_SIDE_LEFT;
    }

    if (player->is_attacking) {
        player->attack_timer += dt;
        player->anim_state = DH_ANIM_STATE_ATTACK;

        if (player->attack_timer >= 0.40f || dh_animation_is_finished(&player->anim_controller)) {
            player->is_attacking = false;
            player->attack_cooldown = 0.35f;
            player->anim_state = (input->move_x != 0.0f) ? DH_ANIM_STATE_RUN : DH_ANIM_STATE_IDLE;
        }
    } else {
        if (input->move_x != 0.0f) {
            player->anim_state = DH_ANIM_STATE_RUN;
        } else if (fabsf(player->velocity.x) < 5.0f) {
            player->anim_state = DH_ANIM_STATE_IDLE;
        }
    }

    if (player->loaded) {
        dh_animation_play(&player->anim_controller, player->anim_state, player->facing_dir, false);
        dh_animation_update(&player->anim_controller, dt);
    }
}

void dh_test_player_draw(const DHTestPlayer *player)
{
    if (player == NULL) return;

    float half_w = player->bbox_size.x * 0.5f;
    float half_h = player->bbox_size.y * 0.5f;

    /* Draw player shadow under feet */
    DrawEllipse((int)player->position.x, (int)(player->position.y + half_h), half_w * 1.2f, 3.0f, (Color){ 0, 0, 0, 120 });

    Color draw_tint = WHITE;
    if (player->is_dead) {
        draw_tint = (Color){ 100, 100, 120, 160 }; /* Fade/darken on death */
    } else if (player->invuln_timer > 0.0f) {
        draw_tint = (Color){ 255, 130, 130, 200 }; /* Invulnerability hit flash */
    }

    if (player->loaded) {
        /* Render 64x64 Swordsman sprite aligned with platform floor */
        float sprite_w = 64.0f;
        float sprite_h = 64.0f;
        Rectangle dest = { player->position.x, player->position.y + half_h, sprite_w, sprite_h };
        Vector2 origin = { sprite_w * 0.5f, sprite_h - 1.0f }; /* Feet pivot at bottom center */

        dh_animation_draw_pro(&player->anim_controller, dest, origin, 0.0f, draw_tint);
    } else {
        /* Fallback rectangle marker */
        Rectangle rect = {
            player->position.x - half_w,
            player->position.y - half_h,
            player->bbox_size.x,
            player->bbox_size.y
        };
        DrawRectangleRec(rect, draw_tint);
        DrawRectangleLinesEx(rect, 1.0f, (Color){ 255, 255, 255, 220 });
    }
}

void dh_test_player_shutdown(DHTestPlayer *player, DHAssetManager *assets)
{
    if (player == NULL || assets == NULL || !player->loaded) return;

    dh_assets_unload_texture(assets, SWORDSMAN_IDLE_PATH);
    dh_assets_unload_texture(assets, SWORDSMAN_RUN_PATH);
    dh_assets_unload_texture(assets, SWORDSMAN_ATTACK_PATH);
    player->loaded = false;
    dh_log_info("Swordsman player resources shut down");
}
