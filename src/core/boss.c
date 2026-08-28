#include "dh/boss.h"
#include "dh/logging.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SWORDSMAN3_IDLE_PATH "assets/craftpix-net-180537-free-swordsman-1-3-level-pixel-top-down-sprite-character/Tiled_files/Swordsman3/Swordsman_lvl3_Idle_with_shadow.png"
#define SWORDSMAN3_RUN_PATH "assets/craftpix-net-180537-free-swordsman-1-3-level-pixel-top-down-sprite-character/Tiled_files/Swordsman3/Swordsman_lvl3_Run_with_shadow.png"
#define SWORDSMAN3_ATTACK_PATH "assets/craftpix-net-180537-free-swordsman-1-3-level-pixel-top-down-sprite-character/Tiled_files/Swordsman3/Swordsman_lvl3_attack_with_shadow.png"
#define SWORDSMAN3_DEATH_PATH "assets/craftpix-net-180537-free-swordsman-1-3-level-pixel-top-down-sprite-character/Tiled_files/Swordsman3/Swordsman_lvl3_Death_with_shadow.png"

static void setup_boss_animations(DHBoss *boss)
{
    dh_animation_controller_init(&boss->anim_controller);

    if (boss->tex_idle.id != 0) {
        DHAnimationClip clip_idle;
        if (dh_animation_clip_create_grid(&clip_idle, "boss_idle", boss->tex_idle, 0, 192, 64, 64, 12, 0.08f, true, true)) {
            dh_animation_controller_add_clip(&boss->anim_controller, DH_ANIM_STATE_IDLE, DH_DIR_SIDE_RIGHT, &clip_idle);
            dh_animation_controller_add_clip(&boss->anim_controller, DH_ANIM_STATE_IDLE, DH_DIR_RIGHT, &clip_idle);
        }
    }

    if (boss->tex_run.id != 0) {
        DHAnimationClip clip_run;
        if (dh_animation_clip_create_grid(&clip_run, "boss_run", boss->tex_run, 0, 192, 64, 64, 8, 0.08f, true, true)) {
            dh_animation_controller_add_clip(&boss->anim_controller, DH_ANIM_STATE_RUN, DH_DIR_SIDE_RIGHT, &clip_run);
            dh_animation_controller_add_clip(&boss->anim_controller, DH_ANIM_STATE_WALK, DH_DIR_SIDE_RIGHT, &clip_run);
            dh_animation_controller_add_clip(&boss->anim_controller, DH_ANIM_STATE_RUN, DH_DIR_RIGHT, &clip_run);
            dh_animation_controller_add_clip(&boss->anim_controller, DH_ANIM_STATE_WALK, DH_DIR_RIGHT, &clip_run);
        }
    }

    if (boss->tex_attack.id != 0) {
        DHAnimationClip clip_atk;
        if (dh_animation_clip_create_grid(&clip_atk, "boss_attack", boss->tex_attack, 0, 192, 64, 64, 8, 0.05f, false, true)) {
            dh_animation_controller_add_clip(&boss->anim_controller, DH_ANIM_STATE_ATTACK, DH_DIR_SIDE_RIGHT, &clip_atk);
            dh_animation_controller_add_clip(&boss->anim_controller, DH_ANIM_STATE_ATTACK, DH_DIR_RIGHT, &clip_atk);
        }
    }

    if (boss->tex_death.id != 0) {
        DHAnimationClip clip_death;
        if (dh_animation_clip_create_grid(&clip_death, "boss_death", boss->tex_death, 0, 192, 64, 64, 7, 0.08f, false, true)) {
            dh_animation_controller_add_clip(&boss->anim_controller, DH_ANIM_STATE_DEATH, DH_DIR_SIDE_RIGHT, &clip_death);
            dh_animation_controller_add_clip(&boss->anim_controller, DH_ANIM_STATE_DEATH, DH_DIR_RIGHT, &clip_death);
        }
    }

    dh_animation_play(&boss->anim_controller, DH_ANIM_STATE_IDLE, DH_DIR_SIDE_RIGHT, true);
}

bool dh_boss_manager_init(DHBossManager *mgr, DHAssetManager *assets)
{
    if (mgr == NULL) return false;

    memset(mgr, 0, sizeof(DHBossManager));
    mgr->boss.active = false;
    mgr->boss.state = DH_BOSS_STATE_INACTIVE;
    mgr->show_boss_debug = false;

    if (assets != NULL) {
        mgr->boss.tex_idle   = dh_assets_load_texture(assets, SWORDSMAN3_IDLE_PATH);
        mgr->boss.tex_run    = dh_assets_load_texture(assets, SWORDSMAN3_RUN_PATH);
        mgr->boss.tex_attack = dh_assets_load_texture(assets, SWORDSMAN3_ATTACK_PATH);
        mgr->boss.tex_death  = dh_assets_load_texture(assets, SWORDSMAN3_DEATH_PATH);
        mgr->boss.loaded     = (mgr->boss.tex_idle.id != 0);
    }

    dh_log_info("Boss system manager initialized successfully.");
    return true;
}

void dh_boss_manager_check_spawning(DHBossManager *mgr, float camera_x, float tile_size)
{
    if (mgr == NULL || mgr->boss.active) return;

    /* Calculate current milestone index based on camera/world position */
    int milestone_id = (int)floorf(camera_x / BOSS_MILESTONE_INTERVAL);
    if (milestone_id <= 0 || milestone_id >= DH_MAX_BOSS_MILESTONES) return;

    /* If this boss milestone was already defeated in this run, do NOT spawn again! */
    if (mgr->milestone_defeated[milestone_id]) return;

    float spawn_x = (float)milestone_id * BOSS_MILESTONE_INTERVAL + 160.0f;
    if (camera_x >= spawn_x - 100.0f && camera_x <= spawn_x + 100.0f) {
        DHBoss *boss = &mgr->boss;
        boss->active = true;
        boss->milestone_id = milestone_id;
        boss->spawn_world_x = spawn_x;
        boss->boundary_min_x = spawn_x - 180.0f;
        boss->boundary_max_x = spawn_x + 180.0f;

        float ts = (tile_size > 0.0f) ? tile_size : 16.0f;
        boss->position = (Vector2){ spawn_x + 60.0f, 10.0f * ts - 16.0f };
        boss->velocity = (Vector2){ 0.0f, 0.0f };
        boss->bbox_size = (Vector2){ 24.0f, 32.0f };
        boss->move_speed = 24.0f;
        boss->gravity = 500.0f;
        boss->grounded = false;

        boss->max_hp = BOSS_BASE_HP + (milestone_id - 1) * 40;
        boss->hp = boss->max_hp;
        boss->phase = 1;
        boss->state = DH_BOSS_STATE_IDLE;

        boss->windup_timer = 0.0f;
        boss->attack_timer = 0.0f;
        boss->attack_cooldown = 0.0f;
        boss->attack_hit_dealt = false;
        boss->active_attack_type = 1;

        boss->hurt_timer = 0.0f;
        boss->death_timer = 0.0f;
        boss->facing_dir = DH_DIR_SIDE_LEFT;

        setup_boss_animations(boss);

        dh_log_info("DEMON KING BOSS SPAWNED! Milestone #%d at Pos:(%.0f,%.0f) HP:%d",
                    milestone_id, boss->position.x, boss->position.y, boss->hp);
    }
}

void dh_boss_manager_update(DHBossManager *mgr, DHTestPlayer *player, DHTilemap *tilemap, DHPickupManager *pickups, float dt)
{
    if (mgr == NULL || player == NULL || tilemap == NULL || dt <= 0.0f) return;

    DHBoss *boss = &mgr->boss;
    if (!boss->active) return;

    /* Enforce Horizontal Arena Boundary during active encounter */
    if (boss->state != DH_BOSS_STATE_DYING) {
        if (player->position.x < boss->boundary_min_x) player->position.x = boss->boundary_min_x;
        if (player->position.x > boss->boundary_max_x) player->position.x = boss->boundary_max_x;
    }

    /* 1. Handle Boss Death Sequence */
    if (boss->state == DH_BOSS_STATE_DYING) {
        boss->velocity.x = 0.0f;
        boss->death_timer += dt;

        dh_animation_play(&boss->anim_controller, DH_ANIM_STATE_DEATH, boss->facing_dir, false);
        dh_animation_update(&boss->anim_controller, dt);

        if (boss->death_timer >= 0.80f || dh_animation_is_finished(&boss->anim_controller)) {
            boss->active = false;
            boss->state = DH_BOSS_STATE_INACTIVE;
            mgr->milestone_defeated[boss->milestone_id] = true;

            /* Award Major Defeat Rewards */
            dh_test_player_add_xp(player, 500);
            if (pickups != NULL) {
                dh_loot_spawn_pickup(pickups, DH_PICKUP_GOLD, boss->position, 25); /* 25 Gold coins worth */
            }
            player->gold_count += 225; /* Total 250 Gold reward */

            dh_log_info("DUNGEON DEMON KING DEFEATED! Milestone #%d cleared! Granted +500 XP and +250 Gold.", boss->milestone_id);
        }
        return;
    }

    /* Update Hurt Flash Timer */
    if (boss->hurt_timer > 0.0f) {
        boss->hurt_timer -= dt;
        if (boss->hurt_timer < 0.0f) boss->hurt_timer = 0.0f;
    }

    /* 2. Check Phase 2 Transition (50% HP threshold) */
    if (boss->hp <= boss->max_hp / 2 && boss->phase == 1) {
        boss->phase = 2;
        boss->move_speed = 30.0f; /* 1.25x speed in Phase 2 */
        dh_log_info("BOSS ENTERED PHASE 2! Red aura active, increased speed & unlocked Heavy Slam attack.");
    }

    /* 3. Check Player Sword Attack Hits against Boss */
    if (player->is_attacking && player->attack_timer >= 0.10f && player->attack_timer <= 0.25f) {
        float box_w = 24.0f;
        float box_h = 20.0f;
        float box_y = player->position.y - 10.0f;
        float box_x = (player->facing_dir == DH_DIR_SIDE_RIGHT) ? (player->position.x + 4.0f) : (player->position.x - 28.0f);
        Rectangle sword_hitbox = { box_x, box_y, box_w, box_h };

        float b_half_w = boss->bbox_size.x * 0.5f;
        float b_half_h = boss->bbox_size.y * 0.5f;
        Rectangle boss_box = { boss->position.x - b_half_w, boss->position.y - b_half_h, boss->bbox_size.x, boss->bbox_size.y };

        if (!player->attack_hit_registered[16 + boss->milestone_id] && CheckCollisionRecs(sword_hitbox, boss_box)) {
            player->attack_hit_registered[16 + boss->milestone_id] = true;
            boss->hp -= player->attack_damage;
            boss->hurt_timer = 0.20f;

            /* Strong Boss Knockback Resistance (0.15x impulse multiplier) */
            float kb_dir = (player->position.x < boss->position.x) ? 1.0f : -1.0f;
            boss->velocity.x += kb_dir * 18.0f;

            dh_log_info("Sword HIT Boss dealing %d DMG! Boss HP: %d/%d", player->attack_damage, boss->hp, boss->max_hp);

            if (boss->hp <= 0) {
                boss->hp = 0;
                boss->state = DH_BOSS_STATE_DYING;
                boss->death_timer = 0.0f;
                dh_log_info("BOSS HEALTH REACHED ZERO! Entering DYING sequence...");
                return;
            }
        }
    }

    /* 4. AI State Machine (Chase, Attack 1 Slash, Attack 2 Slam) */
    float dx = player->position.x - boss->position.x;
    float abs_dx = fabsf(dx);

    if (boss->state == DH_BOSS_STATE_WINDUP_SLASH || boss->state == DH_BOSS_STATE_WINDUP_SLAM) {
        boss->velocity.x = 0.0f;
        boss->windup_timer += dt;
        float windup_limit = (boss->state == DH_BOSS_STATE_WINDUP_SLASH) ? 0.40f : 0.60f;

        if (boss->windup_timer >= windup_limit) {
            boss->state = (boss->state == DH_BOSS_STATE_WINDUP_SLASH) ? DH_BOSS_STATE_ATTACK_SLASH : DH_BOSS_STATE_ATTACK_SLAM;
            boss->attack_timer = 0.0f;
            boss->attack_hit_dealt = false;
        }
    } else if (boss->state == DH_BOSS_STATE_ATTACK_SLASH || boss->state == DH_BOSS_STATE_ATTACK_SLAM) {
        boss->velocity.x = 0.0f;
        boss->attack_timer += dt;
        boss->anim_state = DH_ANIM_STATE_ATTACK;

        /* Execute Boss Attack Damage Resolution against Player */
        if (!player->is_dead && player->invuln_timer <= 0.0f && boss->attack_timer >= 0.15f && boss->attack_timer <= 0.35f && !boss->attack_hit_dealt) {
            float p_half_w = player->bbox_size.x * 0.5f;
            float p_half_h = player->bbox_size.y * 0.5f;
            Rectangle player_box = { player->position.x - p_half_w, player->position.y - p_half_h, player->bbox_size.x, player->bbox_size.y };

            float b_atk_w = (boss->active_attack_type == 2) ? 36.0f : 28.0f;
            float b_atk_h = 24.0f;
            float b_atk_x = (boss->facing_dir == DH_DIR_SIDE_RIGHT) ? (boss->position.x + 4.0f) : (boss->position.x - 32.0f);
            Rectangle b_atk_box = { b_atk_x, boss->position.y - 12.0f, b_atk_w, b_atk_h };

            if (CheckCollisionRecs(b_atk_box, player_box)) {
                boss->attack_hit_dealt = true;
                int dmg = (boss->active_attack_type == 2) ? 3 : 2;
                player->hp -= dmg;
                player->invuln_timer = PLAYER_INVULNERABILITY_TIME;

                float kb_dir = (boss->position.x < player->position.x) ? 1.0f : -1.0f;
                player->velocity.x += kb_dir * 120.0f;

                dh_log_info("Boss Attack #%d HIT Player dealing %d DMG! Player HP: %d/%d", boss->active_attack_type, dmg, player->hp, player->max_hp);

                if (player->hp <= 0) {
                    player->hp = 0;
                    player->is_dead = true;
                    player->death_timer = 0.80f;
                }
            }
        }

        if (boss->attack_timer >= 0.50f || dh_animation_is_finished(&boss->anim_controller)) {
            boss->state = DH_BOSS_STATE_CHASE;
            boss->attack_cooldown = (boss->active_attack_type == 2) ? 1.80f : 1.20f;
            boss->anim_state = DH_ANIM_STATE_RUN;
        }
    } else {
        if (boss->attack_cooldown > 0.0f) {
            boss->attack_cooldown -= dt;
            if (boss->attack_cooldown < 0.0f) boss->attack_cooldown = 0.0f;
        }

        boss->facing_dir = (dx > 0.0f) ? DH_DIR_SIDE_RIGHT : DH_DIR_SIDE_LEFT;

        if (abs_dx <= 32.0f && boss->attack_cooldown <= 0.0f) {
            /* Decide attack pattern (Phase 2 unlocks Heavy Slam) */
            if (boss->phase == 2 && (rand() % 100 < 50)) {
                boss->active_attack_type = 2; /* Heavy Slam */
                boss->state = DH_BOSS_STATE_WINDUP_SLAM;
            } else {
                boss->active_attack_type = 1; /* Horizontal Slash */
                boss->state = DH_BOSS_STATE_WINDUP_SLASH;
            }
            boss->windup_timer = 0.0f;
            boss->velocity.x = 0.0f;
            boss->anim_state = DH_ANIM_STATE_IDLE;
        } else {
            float move_dir = (dx > 0.0f) ? 1.0f : -1.0f;
            boss->velocity.x = move_dir * boss->move_speed;
            boss->state = DH_BOSS_STATE_CHASE;
            boss->anim_state = DH_ANIM_STATE_RUN;
        }
    }

    /* 5. Physics & Tilemap Collision Resolution */
    boss->velocity.y += boss->gravity * dt;
    if (boss->velocity.y > 400.0f) boss->velocity.y = 400.0f;

    Vector2 pos = boss->position;
    float half_w = boss->bbox_size.x * 0.5f;
    float half_h = boss->bbox_size.y * 0.5f;
    int ts = tilemap->tile_size > 0 ? tilemap->tile_size : 16;

    /* Horizontal X */
    float target_x = pos.x + boss->velocity.x * dt;
    Rectangle box_x = { target_x - half_w, pos.y - half_h + 0.1f, boss->bbox_size.x, boss->bbox_size.y - 0.2f };
    if (dh_tilemap_check_rect_collision(tilemap, box_x)) {
        boss->velocity.x = 0.0f;
    } else {
        pos.x = target_x;
    }

    /* Vertical Y */
    float target_y = pos.y + boss->velocity.y * dt;
    Rectangle box_y = { pos.x - half_w + 0.1f, target_y - half_h, boss->bbox_size.x - 0.2f, boss->bbox_size.y };
    if (dh_tilemap_check_rect_collision(tilemap, box_y)) {
        if (boss->velocity.y > 0.0f) {
            boss->grounded = true;
            boss->velocity.y = 0.0f;
            int foot_ty = (int)floorf((target_y + half_h) / (float)ts);
            pos.y = (float)(foot_ty * ts) - half_h;
        }
    } else {
        pos.y = target_y;
    }

    boss->position = pos;

    /* Update Boss Animation Controller */
    dh_animation_play(&boss->anim_controller, boss->anim_state, boss->facing_dir, false);
    dh_animation_update(&boss->anim_controller, dt);
}

void dh_boss_manager_draw(const DHBossManager *mgr)
{
    if (mgr == NULL) return;

    const DHBoss *boss = &mgr->boss;
    if (!boss->active) return;

    float half_w = boss->bbox_size.x * 0.5f;
    float half_h = boss->bbox_size.y * 0.5f;

    /* Drop shadow */
    DrawEllipse((int)boss->position.x, (int)(boss->position.y + half_h), half_w * 1.8f, 3.5f, (Color){ 0, 0, 0, 140 });

    /* Telegraph Warning Indicator during attack windups */
    if (boss->state == DH_BOSS_STATE_WINDUP_SLASH) {
        DrawCircle((int)boss->position.x, (int)(boss->position.y - half_h - 14.0f), 4.0f, (Color){ 255, 220, 80, 255 });
        DrawText("!", (int)boss->position.x - 2, (int)(boss->position.y - half_h - 18.0f), 10, BLACK);
    } else if (boss->state == DH_BOSS_STATE_WINDUP_SLAM) {
        DrawCircle((int)boss->position.x, (int)(boss->position.y - half_h - 14.0f), 6.0f, (Color){ 255, 60, 60, 255 });
        DrawText("!!", (int)boss->position.x - 4, (int)(boss->position.y - half_h - 18.0f), 10, WHITE);
    }

    Color tint = WHITE;
    if (boss->hurt_timer > 0.0f) {
        tint = (Color){ 255, 120, 120, 255 };
    } else if (boss->phase == 2) {
        tint = (Color){ 255, 180, 180, 255 }; /* Phase 2 Red Aura Tint */
    }

    if (boss->loaded) {
        /* Render 96x96 sprite (1.5x scale) centered over collision box */
        float sprite_w = 96.0f;
        float sprite_h = 96.0f;
        Rectangle dest = { boss->position.x, boss->position.y + half_h, sprite_w, sprite_h };
        Vector2 origin = { sprite_w * 0.5f, sprite_h - 1.0f };

        dh_animation_draw_pro(&boss->anim_controller, dest, origin, 0.0f, tint);
    } else {
        Rectangle rect = { boss->position.x - half_w, boss->position.y - half_h, boss->bbox_size.x, boss->bbox_size.y };
        DrawRectangleRec(rect, (Color){ 200, 40, 40, 255 });
    }
}

void dh_boss_manager_draw_hud(const DHBossManager *mgr, int virtual_w, int virtual_h)
{
    (void)virtual_h;
    if (mgr == NULL) return;

    const DHBoss *boss = &mgr->boss;
    if (!boss->active) return;

    /* Render Compact Top Boss Health Bar in 320x180 Virtual Resolution */
    int bar_w = 180;
    int bar_h = 8;
    int bar_x = (virtual_w - bar_w) / 2;
    int bar_y = 22;

    float pct = (boss->max_hp > 0) ? ((float)boss->hp / (float)boss->max_hp) : 0.0f;
    if (pct < 0.0f) pct = 0.0f;
    int fill_w = (int)((float)bar_w * pct);

    Color fill_col = (boss->phase == 2) ? (Color){ 255, 60, 60, 255 } : (Color){ 220, 180, 40, 255 };

    DrawRectangle(bar_x - 2, bar_y - 12, bar_w + 4, bar_h + 16, (Color){ 12, 12, 18, 220 });
    DrawRectangleLines(bar_x - 2, bar_y - 12, bar_w + 4, bar_h + 16, (Color){ 200, 160, 50, 255 });

    char title_buf[64];
    snprintf(title_buf, sizeof(title_buf), "DUNGEON DEMON KING [PHASE %d]", boss->phase);
    int title_w = MeasureText(title_buf, 8);
    DrawText(title_buf, (virtual_w - title_w) / 2, bar_y - 10, 8, (Color){ 255, 220, 100, 255 });

    DrawRectangle(bar_x, bar_y, bar_w, bar_h, (Color){ 30, 30, 40, 255 });
    DrawRectangle(bar_x, bar_y, fill_w, bar_h, fill_col);
    DrawRectangleLines(bar_x, bar_y, bar_w, bar_h, (Color){ 220, 220, 240, 255 });

    char hp_buf[32];
    snprintf(hp_buf, sizeof(hp_buf), "%d / %d", boss->hp, boss->max_hp);
    int hp_w = MeasureText(hp_buf, 8);
    DrawText(hp_buf, (virtual_w - hp_w) / 2, bar_y, 8, RAYWHITE);
}

void dh_boss_manager_draw_debug(const DHBossManager *mgr)
{
    if (mgr == NULL) return;

    const DHBoss *boss = &mgr->boss;
    if (!boss->active) return;

    float half_w = boss->bbox_size.x * 0.5f;
    float half_h = boss->bbox_size.y * 0.5f;

    /* Draw purple boss collision box */
    Rectangle rect = { boss->position.x - half_w, boss->position.y - half_h, boss->bbox_size.x, boss->bbox_size.y };
    DrawRectangleLinesEx(rect, 1.5f, (Color){ 220, 80, 255, 255 });

    /* Draw Arena Boundaries */
    DrawLine((int)boss->boundary_min_x, 0, (int)boss->boundary_min_x, 180, (Color){ 255, 60, 60, 255 });
    DrawLine((int)boss->boundary_max_x, 0, (int)boss->boundary_max_x, 180, (Color){ 255, 60, 60, 255 });

    char dbg_buf[96];
    snprintf(dbg_buf, sizeof(dbg_buf), "BOSS M#%d Phase:%d State:%d HP:%d/%d",
             boss->milestone_id, boss->phase, boss->state, boss->hp, boss->max_hp);
    DrawText(dbg_buf, (int)(boss->position.x - 40.0f), (int)(boss->position.y - half_h - 14.0f), 8, (Color){ 255, 100, 255, 255 });
}

void dh_boss_manager_shutdown(DHBossManager *mgr, DHAssetManager *assets)
{
    if (mgr == NULL || assets == NULL || !mgr->boss.loaded) return;

    dh_assets_unload_texture(assets, SWORDSMAN3_IDLE_PATH);
    dh_assets_unload_texture(assets, SWORDSMAN3_RUN_PATH);
    dh_assets_unload_texture(assets, SWORDSMAN3_ATTACK_PATH);
    dh_assets_unload_texture(assets, SWORDSMAN3_DEATH_PATH);

    mgr->boss.loaded = false;
    dh_log_info("Boss system resources shut down.");
}
