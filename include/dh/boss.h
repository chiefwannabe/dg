#ifndef DH_BOSS_H
#define DH_BOSS_H

#include "dh/player.h"
#include "dh/tilemap.h"
#include "dh/loot.h"
#include "dh/animation.h"
#include "dh/assets.h"
#include <raylib.h>
#include <stdbool.h>

#define DH_MAX_BOSS_MILESTONES 16
#define BOSS_BASE_HP 120
#define BOSS_MILESTONE_INTERVAL 3000.0f /* Spawns boss every 3000 world px (Chunk 15, 30, etc.) */

typedef enum {
    DH_BOSS_STATE_INACTIVE = 0,
    DH_BOSS_STATE_IDLE,
    DH_BOSS_STATE_CHASE,
    DH_BOSS_STATE_WINDUP_SLASH,  /* Attack 1: Quick Horizontal Slash */
    DH_BOSS_STATE_ATTACK_SLASH,
    DH_BOSS_STATE_WINDUP_SLAM,   /* Attack 2: Heavy Ground Slam (Phase 2) */
    DH_BOSS_STATE_ATTACK_SLAM,
    DH_BOSS_STATE_HURT,
    DH_BOSS_STATE_DYING
} DHBossState;

typedef struct {
    bool active;
    int milestone_id;            /* World milestone index (0, 1, 2...) */
    float spawn_world_x;         /* World X position where boss spawned */
    float boundary_min_x;        /* Left arena boundary */
    float boundary_max_x;        /* Right arena boundary */

    Vector2 position;            /* World position (center px) */
    Vector2 velocity;            /* Velocity (vx, vy) */
    Vector2 bbox_size;           /* Collision box (24x32 px) */
    float move_speed;            /* Base move speed (24 px/s) */
    float gravity;               /* Gravity acceleration */
    bool grounded;

    int hp;                      /* Current Boss HP (120) */
    int max_hp;                  /* Max Boss HP (120) */
    int phase;                   /* Phase 1 (100%->50%) or Phase 2 (50%->0%) */
    DHBossState state;           /* Current Boss AI State */

    float windup_timer;          /* Warning windup timer */
    float attack_timer;          /* Active attack duration timer */
    float attack_cooldown;       /* Cooldown between attacks */
    bool attack_hit_dealt;       /* Single hit registry flag per swing */
    int active_attack_type;      /* 1 = Slash (2 DMG), 2 = Slam (3 DMG) */

    float hurt_timer;            /* Hurt flash timer */
    float death_timer;           /* Death animation timer */
    DHDirection facing_dir;
    DHAnimState anim_state;      /* Animation state (IDLE / RUN / ATTACK / DEATH) */

    DHAnimationController anim_controller;
    Texture2D tex_idle;
    Texture2D tex_run;
    Texture2D tex_attack;
    Texture2D tex_death;
    bool loaded;
} DHBoss;

typedef struct {
    DHBoss boss;
    bool milestone_defeated[DH_MAX_BOSS_MILESTONES];
    bool show_boss_debug;
} DHBossManager;

bool dh_boss_manager_init(DHBossManager *mgr, DHAssetManager *assets);
void dh_boss_manager_check_spawning(DHBossManager *mgr, float camera_x, float tile_size);
void dh_boss_manager_update(DHBossManager *mgr, DHTestPlayer *player, DHTilemap *tilemap, DHPickupManager *pickups, float dt);
void dh_boss_manager_draw(const DHBossManager *mgr);
void dh_boss_manager_draw_hud(const DHBossManager *mgr, int virtual_w, int virtual_h);
void dh_boss_manager_draw_debug(const DHBossManager *mgr);
void dh_boss_manager_shutdown(DHBossManager *mgr, DHAssetManager *assets);

#endif /* DH_BOSS_H */
