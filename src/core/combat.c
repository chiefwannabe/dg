#include "dh/combat.h"
#include "dh/logging.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

void dh_combat_init(DHCombatSystem *combat)
{
    if (combat == NULL) return;

    memset(combat, 0, sizeof(DHCombatSystem));
    combat->attack_active = false;
    combat->sword_hitbox = (Rectangle){ 0, 0, 0, 0 };
    combat->damage_dealt_count = 0;
    combat->show_combat_debug = false;

    dh_log_info("Combat system initialized (Sword Damage: %d, Slime Max HP: %d)", SWORD_DAMAGE, SLIME_MAX_HEALTH);
}

void dh_combat_update(DHCombatSystem *combat, DHTestPlayer *player, DHEnemyManager *enemies, DHPickupManager *pickups, float dt)
{
    (void)dt;
    if (combat == NULL || player == NULL || enemies == NULL) return;

    /* 1. Check if player attack is currently inside the active hit window (0.10s to 0.25s of attack) */
    if (player->is_attacking && player->attack_timer >= 0.10f && player->attack_timer <= 0.25f) {
        combat->attack_active = true;

        /* Calculate sword attack hitbox in world space based on facing direction */
        float box_w = 24.0f;
        float box_h = 20.0f;
        float box_y = player->position.y - 10.0f;
        float box_x = (player->facing_dir == DH_DIR_SIDE_RIGHT) ? (player->position.x + 4.0f) : (player->position.x - 28.0f);

        combat->sword_hitbox = (Rectangle){ box_x, box_y, box_w, box_h };

        /* 2. Check collision between active attack hitbox and active enemies */
        for (int i = 0; i < DH_MAX_ENEMIES; i++) {
            DHEnemy *enemy = &enemies->pool[i];
            if (!enemy->active || enemy->is_dying) continue;

            /* Check if this enemy has already been hit during this swing */
            if (player->attack_hit_registered[enemy->id]) continue;

            float e_half_w = enemy->bbox_size.x * 0.5f;
            float e_half_h = enemy->bbox_size.y * 0.5f;
            Rectangle enemy_box = { enemy->position.x - e_half_w, enemy->position.y - e_half_h, enemy->bbox_size.x, enemy->bbox_size.y };

            if (CheckCollisionRecs(combat->sword_hitbox, enemy_box)) {
                /* Register hit to prevent multi-hit per swing bug */
                player->attack_hit_registered[enemy->id] = true;
                combat->damage_dealt_count++;

                /* Apply player attack damage (upgraded per level!) */
                enemy->hp -= player->attack_damage;
                enemy->hurt_timer = 0.25f;

                /* Apply horizontal knockback away from player (scaled by archetype knockback resistance) */
                float kb_dir = (player->position.x < enemy->position.x) ? 1.0f : -1.0f;
                float kb_res = (enemy->def != NULL) ? enemy->def->knockback_resistance : 1.0f;
                enemy->velocity.x += kb_dir * 120.0f * kb_res;

                dh_log_info("Sword HIT %s #%d dealing %d DMG! Remaining HP: %d/%d",
                            (enemy->def != NULL) ? enemy->def->name : "Enemy",
                            enemy->id, player->attack_damage, enemy->hp, enemy->max_hp);

                /* Handle Enemy Death, XP Reward, and Loot Pickup Spawning */
                if (enemy->hp <= 0 && !enemy->is_dying) {
                    enemy->is_dying = true;
                    enemy->death_timer = 0.0f;

                    int xp_reward = (enemy->def != NULL) ? enemy->def->xp_reward : SLIME_XP_REWARD;
                    int gold_reward = (enemy->def != NULL) ? enemy->def->gold_reward : SLIME_KILL_REWARD;

                    /* Award XP to Player */
                    dh_test_player_add_xp(player, xp_reward);

                    /* Spawn Gold Pickup on dungeon floor */
                    if (pickups != NULL) {
                        dh_loot_spawn_pickup(pickups, DH_PICKUP_GOLD, enemy->position, gold_reward);
                    }

                    dh_log_info("%s #%d DEFEATED! Granted +%d XP and spawned Gold Pickup (+%d Gold).",
                                (enemy->def != NULL) ? enemy->def->name : "Enemy", enemy->id, xp_reward, gold_reward);
                }
            }
        }
    } else {
        combat->attack_active = false;
        combat->sword_hitbox = (Rectangle){ 0, 0, 0, 0 };
    }

    /* 3. Check enemy attacks against player */
    if (!player->is_dead && player->invuln_timer <= 0.0f) {
        float p_half_w = player->bbox_size.x * 0.5f;
        float p_half_h = player->bbox_size.y * 0.5f;
        Rectangle player_box = { player->position.x - p_half_w, player->position.y - p_half_h, player->bbox_size.x, player->bbox_size.y };

        for (int i = 0; i < DH_MAX_ENEMIES; i++) {
            DHEnemy *enemy = &enemies->pool[i];
            if (!enemy->active || enemy->is_dying || !enemy->is_attacking) continue;

            /* Damage occurs ONLY during the active hit window (0.15s to 0.35s) */
            if (enemy->attack_timer >= 0.15f && enemy->attack_timer <= 0.35f && !enemy->attack_hit_dealt) {
                float e_atk_w = 20.0f;
                float e_atk_h = 16.0f;
                float e_atk_x = (enemy->facing_dir == DH_DIR_SIDE_RIGHT) ? (enemy->position.x + 2.0f) : (enemy->position.x - 22.0f);
                Rectangle e_atk_box = { e_atk_x, enemy->position.y - 8.0f, e_atk_w, e_atk_h };

                if (CheckCollisionRecs(e_atk_box, player_box)) {
                    enemy->attack_hit_dealt = true;
                    int atk_damage = (enemy->damage > 0) ? enemy->damage : 1;
                    player->hp -= atk_damage;
                    player->invuln_timer = PLAYER_INVULNERABILITY_TIME;

                    /* Apply horizontal knockback to player */
                    float kb_dir = (enemy->position.x < player->position.x) ? 1.0f : -1.0f;
                    player->velocity.x += kb_dir * 100.0f;

                    dh_log_info("Enemy #%d (%s) HIT Player dealing %d DMG! Remaining Player HP: %d/%d",
                                enemy->id, (enemy->def != NULL) ? enemy->def->name : "Slime", atk_damage, player->hp, player->max_hp);

                    if (player->hp <= 0) {
                        player->hp = 0;
                        player->is_dead = true;
                        player->death_timer = 0.80f;
                        dh_log_info("PLAYER DIED! Respawn queued in 0.8s...");
                        break;
                    }
                }
            }
        }
    }
}

void dh_combat_draw_debug(const DHCombatSystem *combat, const DHTestPlayer *player, const DHEnemyManager *enemies)
{
    if (combat == NULL || player == NULL || enemies == NULL) return;

    /* Draw active sword attack hitbox in green */
    if (combat->attack_active) {
        DrawRectangleRec(combat->sword_hitbox, (Color){ 80, 255, 120, 100 });
        DrawRectangleLinesEx(combat->sword_hitbox, 1.5f, (Color){ 80, 255, 120, 255 });
    }

    /* Draw HP bars above all active enemies */
    for (int i = 0; i < DH_MAX_ENEMIES; i++) {
        const DHEnemy *enemy = &enemies->pool[i];
        if (!enemy->active) continue;

        float half_h = enemy->bbox_size.y * 0.5f;
        float bar_w = 20.0f;
        float bar_h = 3.0f;
        float pct = (enemy->max_hp > 0) ? ((float)enemy->hp / (float)enemy->max_hp) : 0.0f;
        if (pct < 0.0f) pct = 0.0f;

        float fill_w = bar_w * pct;
        Color fill_col = (enemy->hp > 1) ? (Color){ 80, 240, 100, 255 } : (Color){ 255, 60, 60, 255 };

        int bar_x = (int)(enemy->position.x - 10.0f);
        int bar_y = (int)(enemy->position.y - half_h - 6.0f);

        DrawRectangle(bar_x, bar_y, (int)bar_w, (int)bar_h, (Color){ 20, 20, 30, 220 });
        DrawRectangle(bar_x, bar_y, (int)fill_w, (int)bar_h, fill_col);
        DrawRectangleLines(bar_x, bar_y, (int)bar_w, (int)bar_h, (Color){ 200, 200, 220, 255 });

        char hp_buf[32];
        snprintf(hp_buf, sizeof(hp_buf), "HP:%d/%d", enemy->hp, enemy->max_hp);
        DrawText(hp_buf, bar_x - 2, bar_y - 8, 8, RAYWHITE);
    }
}
