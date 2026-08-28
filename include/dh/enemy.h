#ifndef DH_ENEMY_H
#define DH_ENEMY_H

#include "dh/input.h"
#include "dh/tilemap.h"
#include "dh/animation.h"
#include "dh/assets.h"
#include <raylib.h>
#include <stdbool.h>

#define DH_MAX_ENEMIES 32

typedef enum {
    DH_ENEMY_TYPE_SLIME = 0,
    DH_ENEMY_TYPE_ORC,
    DH_ENEMY_TYPE_HEAVY,
    DH_ENEMY_TYPE_COUNT
} DHEnemyType;

typedef struct {
    const char *name;
    DHEnemyType type;
    int max_hp;
    float move_speed;
    int damage;
    float attack_range;
    float attack_cooldown;
    float detection_range;
    int xp_reward;
    int gold_reward;
    float knockback_resistance; /* 1.0 = normal, 0.6 = reduced, 0.25 = heavy resistance */
    Vector2 bbox_size;
} DHEnemyDefinition;

typedef struct {
    int id;                                /* Active enemy slot ID */
    DHEnemyType type;                      /* Enemy archetype identifier */
    const DHEnemyDefinition *def;          /* Pointer to archetype definition */
    bool active;                           /* Is currently active in world */
    Vector2 position;                      /* World position (center in px) */
    Vector2 velocity;                      /* Current velocity (vx, vy) */
    Vector2 bbox_size;                     /* Collision box dimensions */
    float move_speed;                      /* Scaled movement speed */
    float gravity;                         /* Gravity acceleration (px/s^2) */
    bool grounded;                         /* Standing on solid floor */
    float detection_range;                 /* Horizontal detection radius */

    int hp;                                /* Current Health Points */
    int max_hp;                            /* Scaled Maximum Health Points */
    int damage;                            /* Scaled Attack Damage */
    float hurt_timer;                      /* Hit flash / recoil timer */
    bool is_dying;                         /* Is executing death animation */
    float death_timer;                     /* Death animation timer */

    bool is_attacking;                     /* Is performing attack animation */
    float attack_timer;                    /* Attack animation timer */
    float attack_cooldown;                 /* Cooldown between attacks */
    bool attack_hit_dealt;                 /* Damage dealt flag for current swing */

    DHDirection facing_dir;                 /* Facing direction (SIDE_RIGHT / SIDE_LEFT) */
    DHAnimState anim_state;                /* Current animation state (IDLE / RUN / ATTACK / DEATH) */
    DHAnimationController anim_controller; /* Animation controller */
    int spawn_chunk_idx;                   /* Chunk index this enemy belongs to */
} DHEnemy;

typedef struct {
    DHEnemy pool[DH_MAX_ENEMIES];
    Texture2D slime_idle_tex[DH_ENEMY_TYPE_COUNT];
    Texture2D slime_run_tex[DH_ENEMY_TYPE_COUNT];
    Texture2D slime_death_tex[DH_ENEMY_TYPE_COUNT];
    Texture2D slime_hurt_tex[DH_ENEMY_TYPE_COUNT];
    Texture2D slime_attack_tex[DH_ENEMY_TYPE_COUNT];
    bool loaded;
    int active_count;
} DHEnemyManager;

const DHEnemyDefinition *dh_enemy_definition_get(DHEnemyType type);

bool dh_enemy_manager_init(DHEnemyManager *mgr, DHAssetManager *assets);
void dh_enemy_manager_update(DHEnemyManager *mgr, Vector2 player_pos, DHTilemap *tilemap, float camera_x, float dt);
void dh_enemy_manager_draw(const DHEnemyManager *mgr);
void dh_enemy_manager_draw_debug(const DHEnemyManager *mgr, Vector2 player_pos);
void dh_enemy_manager_shutdown(DHEnemyManager *mgr, DHAssetManager *assets);

/* Spawning helper tied to endless world chunk generation */
void dh_enemy_manager_on_chunk_generated(DHEnemyManager *mgr, int chunk_idx, int tile_size);

/* Overlap detection helper for player contact */
bool dh_enemy_manager_check_player_overlap(const DHEnemyManager *mgr, Rectangle player_box, int *out_enemy_id);

#endif /* DH_ENEMY_H */
