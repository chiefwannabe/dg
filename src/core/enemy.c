#include "dh/enemy.h"
#include "dh/logging.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const DHEnemyDefinition DH_ENEMY_DEFINITIONS[DH_ENEMY_TYPE_COUNT] = {
    [DH_ENEMY_TYPE_SLIME] = {
        .name = "Green Slime",
        .type = DH_ENEMY_TYPE_SLIME,
        .max_hp = 3,
        .move_speed = 35.0f,
        .damage = 1,
        .attack_range = 22.0f,
        .attack_cooldown = 0.80f,
        .detection_range = 140.0f,
        .xp_reward = 10,
        .gold_reward = 1,
        .knockback_resistance = 1.0f,
        .bbox_size = { 14.0f, 14.0f }
    },
    [DH_ENEMY_TYPE_ORC] = {
        .name = "Red Orc Slime",
        .type = DH_ENEMY_TYPE_ORC,
        .max_hp = 6,
        .move_speed = 28.0f,
        .damage = 2,
        .attack_range = 25.0f,
        .attack_cooldown = 1.10f,
        .detection_range = 150.0f,
        .xp_reward = 25,
        .gold_reward = 2,
        .knockback_resistance = 0.60f,
        .bbox_size = { 16.0f, 16.0f }
    },
    [DH_ENEMY_TYPE_HEAVY] = {
        .name = "Heavy Golem Slime",
        .type = DH_ENEMY_TYPE_HEAVY,
        .max_hp = 12,
        .move_speed = 20.0f,
        .damage = 3,
        .attack_range = 28.0f,
        .attack_cooldown = 1.50f,
        .detection_range = 160.0f,
        .xp_reward = 50,
        .gold_reward = 4,
        .knockback_resistance = 0.25f,
        .bbox_size = { 18.0f, 18.0f }
    }
};

static const char *SLIME_PATHS[DH_ENEMY_TYPE_COUNT][5] = {
    [DH_ENEMY_TYPE_SLIME] = {
        "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime1/With_shadow/Slime1_Idle_with_shadow.png",
        "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime1/With_shadow/Slime1_Run_with_shadow.png",
        "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime1/With_shadow/Slime1_Death_with_shadow.png",
        "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime1/With_shadow/Slime1_Hurt_with_shadow.png",
        "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime1/With_shadow/Slime1_Attack_with_shadow.png"
    },
    [DH_ENEMY_TYPE_ORC] = {
        "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime2/With_shadow/Slime2_Idle_with_shadow.png",
        "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime2/With_shadow/Slime2_Run_with_shadow.png",
        "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime2/With_shadow/Slime2_Death_with_shadow.png",
        "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime2/With_shadow/Slime2_Hurt_with_shadow.png",
        "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime2/With_shadow/Slime2_Attack_with_shadow.png"
    },
    [DH_ENEMY_TYPE_HEAVY] = {
        "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime3/With_shadow/Slime3_Idle_with_shadow.png",
        "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime3/With_shadow/Slime3_Run_with_shadow.png",
        "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime3/With_shadow/Slime3_Death_with_shadow.png",
        "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime3/With_shadow/Slime3_Hurt_with_shadow.png",
        "assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack/PNG/Slime3/With_shadow/Slime3_Attack_with_shadow.png"
    }
};

const DHEnemyDefinition *dh_enemy_definition_get(DHEnemyType type)
{
    if (type < 0 || type >= DH_ENEMY_TYPE_COUNT) {
        type = DH_ENEMY_TYPE_SLIME;
    }
    return &DH_ENEMY_DEFINITIONS[type];
}

static void setup_enemy_animation(DHEnemy *enemy, Texture2D tex_idle, Texture2D tex_run, Texture2D tex_death, Texture2D tex_atk)
{
    dh_animation_controller_init(&enemy->anim_controller);

    if (tex_idle.id != 0) {
        DHAnimationClip clip_idle;
        if (dh_animation_clip_create_grid(&clip_idle, "enemy_idle", tex_idle, 0, 192, 64, 64, 6, 0.1f, true, true)) {
            dh_animation_controller_add_clip(&enemy->anim_controller, DH_ANIM_STATE_IDLE, DH_DIR_SIDE_RIGHT, &clip_idle);
            dh_animation_controller_add_clip(&enemy->anim_controller, DH_ANIM_STATE_IDLE, DH_DIR_RIGHT, &clip_idle);
        }
    }

    if (tex_run.id != 0) {
        DHAnimationClip clip_run;
        if (dh_animation_clip_create_grid(&clip_run, "enemy_run", tex_run, 0, 192, 64, 64, 8, 0.09f, true, true)) {
            dh_animation_controller_add_clip(&enemy->anim_controller, DH_ANIM_STATE_RUN, DH_DIR_SIDE_RIGHT, &clip_run);
            dh_animation_controller_add_clip(&enemy->anim_controller, DH_ANIM_STATE_WALK, DH_DIR_SIDE_RIGHT, &clip_run);
            dh_animation_controller_add_clip(&enemy->anim_controller, DH_ANIM_STATE_RUN, DH_DIR_RIGHT, &clip_run);
            dh_animation_controller_add_clip(&enemy->anim_controller, DH_ANIM_STATE_WALK, DH_DIR_RIGHT, &clip_run);
        }
    }

    if (tex_atk.id != 0) {
        DHAnimationClip clip_atk;
        if (dh_animation_clip_create_grid(&clip_atk, "enemy_attack", tex_atk, 0, 192, 64, 64, 9, 0.05f, false, true)) {
            dh_animation_controller_add_clip(&enemy->anim_controller, DH_ANIM_STATE_ATTACK, DH_DIR_SIDE_RIGHT, &clip_atk);
            dh_animation_controller_add_clip(&enemy->anim_controller, DH_ANIM_STATE_ATTACK, DH_DIR_RIGHT, &clip_atk);
        }
    }

    if (tex_death.id != 0) {
        DHAnimationClip clip_death;
        if (dh_animation_clip_create_grid(&clip_death, "enemy_death", tex_death, 0, 192, 64, 64, 10, 0.06f, false, true)) {
            dh_animation_controller_add_clip(&enemy->anim_controller, DH_ANIM_STATE_DEATH, DH_DIR_SIDE_RIGHT, &clip_death);
            dh_animation_controller_add_clip(&enemy->anim_controller, DH_ANIM_STATE_DEATH, DH_DIR_RIGHT, &clip_death);
        }
    }

    dh_animation_play(&enemy->anim_controller, DH_ANIM_STATE_IDLE, DH_DIR_SIDE_RIGHT, true);
}

bool dh_enemy_manager_init(DHEnemyManager *mgr, DHAssetManager *assets)
{
    if (mgr == NULL) return false;

    memset(mgr, 0, sizeof(DHEnemyManager));

    for (int i = 0; i < DH_MAX_ENEMIES; i++) {
        mgr->pool[i].id = i;
        mgr->pool[i].active = false;
        mgr->pool[i].spawn_chunk_idx = -1;
    }

    if (assets != NULL) {
        for (int t = 0; t < DH_ENEMY_TYPE_COUNT; t++) {
            mgr->slime_idle_tex[t]   = dh_assets_load_texture(assets, SLIME_PATHS[t][0]);
            mgr->slime_run_tex[t]    = dh_assets_load_texture(assets, SLIME_PATHS[t][1]);
            mgr->slime_death_tex[t]  = dh_assets_load_texture(assets, SLIME_PATHS[t][2]);
            mgr->slime_hurt_tex[t]   = dh_assets_load_texture(assets, SLIME_PATHS[t][3]);
            mgr->slime_attack_tex[t] = dh_assets_load_texture(assets, SLIME_PATHS[t][4]);
        }
        mgr->loaded = (mgr->slime_idle_tex[0].id != 0);
    }

    dh_log_info("Enemy manager initialized (Pool size: %d, Archetypes: %d)", DH_MAX_ENEMIES, DH_ENEMY_TYPE_COUNT);
    return true;
}

void dh_enemy_manager_on_chunk_generated(DHEnemyManager *mgr, int chunk_idx, int tile_size)
{
    if (mgr == NULL || chunk_idx <= 0) return; /* Chunk 0 is safe intro zone */

    /* Check if an active enemy is already spawned for this chunk index */
    for (int i = 0; i < DH_MAX_ENEMIES; i++) {
        if (mgr->pool[i].active && mgr->pool[i].spawn_chunk_idx == chunk_idx) {
            return;
        }
    }

    /* Deterministic pseudo-random seed per chunk */
    unsigned int hash = (unsigned int)(chunk_idx * 2654435761u);
    if ((hash % 100) > 65) {
        return; /* ~35% chance to spawn an enemy in this chunk */
    }

    /* Find inactive slot in pool */
    int slot = -1;
    for (int i = 0; i < DH_MAX_ENEMIES; i++) {
        if (!mgr->pool[i].active) {
            slot = i;
            break;
        }
    }

    if (slot == -1) return; /* Pool full */

    float spawn_tx = (float)(chunk_idx * DH_CHUNK_WIDTH + 8 + (hash % 6));
    float ts = (tile_size > 0) ? (float)tile_size : 16.0f;
    float world_x = spawn_tx * ts;

    /* 1. Distance-Based Weighted Archetype Selection */
    DHEnemyType selected_type = DH_ENEMY_TYPE_SLIME;
    unsigned int type_roll = (hash >> 4) % 100;

    if (world_x < 2000.0f) {
        /* Chunks 0..6: 80% Green Slime, 20% Red Orc */
        selected_type = (type_roll < 80) ? DH_ENEMY_TYPE_SLIME : DH_ENEMY_TYPE_ORC;
    } else if (world_x < 5000.0f) {
        /* Chunks 7..15: 40% Green Slime, 45% Red Orc, 15% Heavy Golem */
        if (type_roll < 40) selected_type = DH_ENEMY_TYPE_SLIME;
        else if (type_roll < 85) selected_type = DH_ENEMY_TYPE_ORC;
        else selected_type = DH_ENEMY_TYPE_HEAVY;
    } else {
        /* Chunk 16+: 30% Green Slime, 45% Red Orc, 25% Heavy Golem */
        if (type_roll < 30) selected_type = DH_ENEMY_TYPE_SLIME;
        else if (type_roll < 75) selected_type = DH_ENEMY_TYPE_ORC;
        else selected_type = DH_ENEMY_TYPE_HEAVY;
    }

    const DHEnemyDefinition *def = dh_enemy_definition_get(selected_type);

    /* 2. Controlled Distance Scaling Multipliers */
    float hp_mult = 1.0f + fminf(1.5f, world_x / 10000.0f);   /* Max 2.5x HP cap */
    float dmg_mult = 1.0f + fminf(1.0f, world_x / 15000.0f);  /* Max 2.0x DMG cap */
    float spd_mult = 1.0f + fminf(0.2f, world_x / 20000.0f);  /* Max 1.2x Speed cap */

    DHEnemy *enemy = &mgr->pool[slot];
    enemy->id = slot;
    enemy->type = selected_type;
    enemy->def = def;
    enemy->active = true;
    enemy->spawn_chunk_idx = chunk_idx;

    enemy->position = (Vector2){ world_x, 10.0f * ts - 7.0f };
    enemy->velocity = (Vector2){ 0.0f, 0.0f };
    enemy->bbox_size = def->bbox_size;
    enemy->move_speed = def->move_speed * spd_mult;
    enemy->gravity = 500.0f;
    enemy->grounded = false;
    enemy->detection_range = def->detection_range;

    /* Scaled Combat Stats */
    enemy->max_hp = (int)ceilf((float)def->max_hp * hp_mult);
    enemy->hp = enemy->max_hp;
    enemy->damage = (int)ceilf((float)def->damage * dmg_mult);
    enemy->hurt_timer = 0.0f;
    enemy->is_dying = false;
    enemy->death_timer = 0.0f;

    enemy->is_attacking = false;
    enemy->attack_timer = 0.0f;
    enemy->attack_cooldown = 0.0f;
    enemy->attack_hit_dealt = false;

    enemy->facing_dir = DH_DIR_SIDE_LEFT;
    enemy->anim_state = DH_ANIM_STATE_IDLE;

    setup_enemy_animation(enemy,
                          mgr->slime_idle_tex[selected_type],
                          mgr->slime_run_tex[selected_type],
                          mgr->slime_death_tex[selected_type],
                          mgr->slime_attack_tex[selected_type]);

    dh_log_info("Spawned %s #%d [HP:%d/%d DMG:%d] in Chunk #%d at Pos:(%.0f,%.0f)",
                def->name, slot, enemy->hp, enemy->max_hp, enemy->damage, chunk_idx, enemy->position.x, enemy->position.y);
}

void dh_enemy_manager_update(DHEnemyManager *mgr, Vector2 player_pos, DHTilemap *tilemap, float camera_x, float dt)
{
    if (mgr == NULL || tilemap == NULL || dt <= 0.0f) return;

    int active_cnt = 0;
    int ts = tilemap->tile_size > 0 ? tilemap->tile_size : 16;

    for (int i = 0; i < DH_MAX_ENEMIES; i++) {
        DHEnemy *enemy = &mgr->pool[i];
        if (!enemy->active) continue;

        /* 1. Camera Distance Deactivation (Recycle slot if far outside viewport) */
        float dist_cam = fabsf(enemy->position.x - camera_x);
        if (dist_cam > 480.0f) {
            enemy->active = false;
            dh_log_info("Recycled enemy slot #%d (distance: %.0f px)", enemy->id, dist_cam);
            continue;
        }

        active_cnt++;

        /* 2. Handle Dying Enemies (Play death animation once then recycle) */
        if (enemy->is_dying) {
            enemy->velocity.x = 0.0f;
            enemy->anim_state = DH_ANIM_STATE_DEATH;
            enemy->death_timer += dt;

            dh_animation_play(&enemy->anim_controller, DH_ANIM_STATE_DEATH, enemy->facing_dir, false);
            dh_animation_update(&enemy->anim_controller, dt);

            if (enemy->death_timer >= 0.60f || dh_animation_is_finished(&enemy->anim_controller)) {
                enemy->active = false;
                dh_log_info("Enemy #%d death animation complete. Recycled slot.", enemy->id);
            }
            continue;
        }

        /* Update Hurt Flash Timer */
        if (enemy->hurt_timer > 0.0f) {
            enemy->hurt_timer -= dt;
            if (enemy->hurt_timer < 0.0f) enemy->hurt_timer = 0.0f;
        }

        /* 3. AI Horizontal Player Detection, Pursuit & Attack State Machine */
        float dx = player_pos.x - enemy->position.x;
        float abs_dx = fabsf(dx);

        if (enemy->is_attacking) {
            enemy->velocity.x = 0.0f;
            enemy->attack_timer += dt;
            enemy->anim_state = DH_ANIM_STATE_ATTACK;

            if (enemy->attack_timer >= 0.50f || dh_animation_is_finished(&enemy->anim_controller)) {
                enemy->is_attacking = false;
                enemy->attack_cooldown = enemy->def ? enemy->def->attack_cooldown : 1.0f;
                enemy->anim_state = (abs_dx <= enemy->detection_range) ? DH_ANIM_STATE_RUN : DH_ANIM_STATE_IDLE;
            }
        } else {
            if (enemy->attack_cooldown > 0.0f) {
                enemy->attack_cooldown -= dt;
                if (enemy->attack_cooldown < 0.0f) enemy->attack_cooldown = 0.0f;
            }

            enemy->facing_dir = (dx > 0.0f) ? DH_DIR_SIDE_RIGHT : DH_DIR_SIDE_LEFT;

            float atk_range = enemy->def ? enemy->def->attack_range : 22.0f;

            if (abs_dx <= atk_range && enemy->attack_cooldown <= 0.0f) {
                enemy->is_attacking = true;
                enemy->attack_timer = 0.0f;
                enemy->attack_hit_dealt = false;
                enemy->velocity.x = 0.0f;
                enemy->anim_state = DH_ANIM_STATE_ATTACK;
            } else if (abs_dx <= enemy->detection_range) {
                float move_dir = (dx > 0.0f) ? 1.0f : -1.0f;
                enemy->velocity.x = move_dir * enemy->move_speed;
                enemy->anim_state = DH_ANIM_STATE_RUN;
            } else {
                enemy->velocity.x = 0.0f;
                enemy->anim_state = DH_ANIM_STATE_IDLE;
            }
        }

        /* 4. Apply Gravity */
        enemy->velocity.y += enemy->gravity * dt;
        if (enemy->velocity.y > 400.0f) enemy->velocity.y = 400.0f;

        /* 5. Horizontal X Collision Resolution */
        Vector2 current_pos = enemy->position;
        float half_w = enemy->bbox_size.x * 0.5f;
        float half_h = enemy->bbox_size.y * 0.5f;

        float target_x = current_pos.x + enemy->velocity.x * dt;
        Rectangle box_x = { target_x - half_w, current_pos.y - half_h + 0.1f, enemy->bbox_size.x, enemy->bbox_size.y - 0.2f };

        if (dh_tilemap_check_rect_collision(tilemap, box_x)) {
            if (enemy->velocity.x > 0.0f) {
                int wall_tx = (int)floorf((target_x + half_w) / (float)ts);
                current_pos.x = (float)(wall_tx * ts) - half_w - 0.01f;
            } else if (enemy->velocity.x < 0.0f) {
                int wall_tx = (int)floorf((target_x - half_w) / (float)ts);
                current_pos.x = (float)((wall_tx + 1) * ts) + half_w + 0.01f;
            }
            enemy->velocity.x = 0.0f;
        } else {
            current_pos.x = target_x;
        }

        /* 6. Vertical Y Collision Resolution */
        enemy->grounded = false;
        float target_y = current_pos.y + enemy->velocity.y * dt;
        Rectangle box_y = { current_pos.x - half_w + 0.1f, target_y - half_h, enemy->bbox_size.x - 0.2f, enemy->bbox_size.y };

        if (dh_tilemap_check_rect_collision(tilemap, box_y)) {
            if (enemy->velocity.y > 0.0f) {
                enemy->grounded = true;
                enemy->velocity.y = 0.0f;
                int foot_ty = (int)floorf((target_y + half_h) / (float)ts);
                current_pos.y = (float)(foot_ty * ts) - half_h;
            } else if (enemy->velocity.y < 0.0f) {
                enemy->velocity.y = 0.0f;
                int head_ty = (int)floorf((target_y - half_h) / (float)ts);
                current_pos.y = (float)((head_ty + 1) * ts) + half_h;
            }
        } else {
            current_pos.y = target_y;
        }

        enemy->position = current_pos;

        /* 7. Update Enemy Animation Controller */
        dh_animation_play(&enemy->anim_controller, enemy->anim_state, enemy->facing_dir, false);
        dh_animation_update(&enemy->anim_controller, dt);
    }

    mgr->active_count = active_cnt;
}

void dh_enemy_manager_draw(const DHEnemyManager *mgr)
{
    if (mgr == NULL) return;

    for (int i = 0; i < DH_MAX_ENEMIES; i++) {
        const DHEnemy *enemy = &mgr->pool[i];
        if (!enemy->active) continue;

        float half_w = enemy->bbox_size.x * 0.5f;
        float half_h = enemy->bbox_size.y * 0.5f;

        /* Draw shadow under enemy feet */
        DrawEllipse((int)enemy->position.x, (int)(enemy->position.y + half_h), half_w * 1.3f, 2.5f, (Color){ 0, 0, 0, 110 });

        Color draw_tint = (enemy->hurt_timer > 0.0f) ? (Color){ 255, 130, 130, 255 } : WHITE;

        if (mgr->loaded) {
            /* Render 64x64 Slime sprite centered over collision box, feet aligned with platform surface */
            float sprite_w = 64.0f;
            float sprite_h = 64.0f;
            Rectangle dest = { enemy->position.x, enemy->position.y + half_h, sprite_w, sprite_h };
            Vector2 origin = { sprite_w * 0.5f, sprite_h - 1.0f };

            dh_animation_draw_pro(&enemy->anim_controller, dest, origin, 0.0f, draw_tint);
        } else {
            /* Fallback marker */
            Rectangle rect = { enemy->position.x - half_w, enemy->position.y - half_h, enemy->bbox_size.x, enemy->bbox_size.y };
            DrawRectangleRec(rect, (Color){ 120, 220, 90, 255 });
        }
    }
}

void dh_enemy_manager_draw_debug(const DHEnemyManager *mgr, Vector2 player_pos)
{
    if (mgr == NULL) return;

    for (int i = 0; i < DH_MAX_ENEMIES; i++) {
        const DHEnemy *enemy = &mgr->pool[i];
        if (!enemy->active) continue;

        float half_w = enemy->bbox_size.x * 0.5f;
        float half_h = enemy->bbox_size.y * 0.5f;

        /* Draw red enemy bounding box */
        Rectangle rect = { enemy->position.x - half_w, enemy->position.y - half_h, enemy->bbox_size.x, enemy->bbox_size.y };
        DrawRectangleLinesEx(rect, 1.0f, (Color){ 255, 60, 60, 240 });

        /* Draw yellow detection line towards player when detected */
        float dx = fabsf(player_pos.x - enemy->position.x);
        if (dx <= enemy->detection_range) {
            DrawLineV(enemy->position, player_pos, (Color){ 255, 220, 80, 200 });
            DrawCircleLines((int)enemy->position.x, (int)enemy->position.y, (int)enemy->detection_range, (Color){ 255, 220, 80, 100 });
        }

        /* Label above enemy */
        char dbg_buf[96];
        const char *type_name = (enemy->def != NULL) ? enemy->def->name : "Enemy";
        snprintf(dbg_buf, sizeof(dbg_buf), "%s #%d [HP:%d/%d DMG:%d]", type_name, enemy->id, enemy->hp, enemy->max_hp, enemy->damage);
        DrawText(dbg_buf, (int)(enemy->position.x - 30.0f), (int)(enemy->position.y - half_h - 12.0f), 8, (Color){ 255, 220, 100, 255 });
    }
}

bool dh_enemy_manager_check_player_overlap(const DHEnemyManager *mgr, Rectangle player_box, int *out_enemy_id)
{
    if (mgr == NULL) return false;

    for (int i = 0; i < DH_MAX_ENEMIES; i++) {
        const DHEnemy *enemy = &mgr->pool[i];
        if (!enemy->active) continue;

        float half_w = enemy->bbox_size.x * 0.5f;
        float half_h = enemy->bbox_size.y * 0.5f;
        Rectangle enemy_box = { enemy->position.x - half_w, enemy->position.y - half_h, enemy->bbox_size.x, enemy->bbox_size.y };

        if (CheckCollisionRecs(player_box, enemy_box)) {
            if (out_enemy_id) *out_enemy_id = enemy->id;
            return true;
        }
    }

    return false;
}

void dh_enemy_manager_shutdown(DHEnemyManager *mgr, DHAssetManager *assets)
{
    if (mgr == NULL || assets == NULL || !mgr->loaded) return;

    for (int t = 0; t < DH_ENEMY_TYPE_COUNT; t++) {
        dh_assets_unload_texture(assets, SLIME_PATHS[t][0]);
        dh_assets_unload_texture(assets, SLIME_PATHS[t][1]);
        dh_assets_unload_texture(assets, SLIME_PATHS[t][2]);
        dh_assets_unload_texture(assets, SLIME_PATHS[t][3]);
        dh_assets_unload_texture(assets, SLIME_PATHS[t][4]);
    }
    mgr->loaded = false;
    dh_log_info("Enemy manager resources shut down");
}
