#ifndef DH_PLAYER_H
#define DH_PLAYER_H

#include "dh/input.h"
#include "dh/tilemap.h"
#include "dh/animation.h"
#include "dh/assets.h"
#include <raylib.h>
#include <stdbool.h>

#define PLAYER_MAX_HEALTH 5
#define PLAYER_INVULNERABILITY_TIME 0.75f

typedef struct {
    Vector2 position;            /* Center position in world pixels */
    Vector2 velocity;            /* Current velocity (vx, vy) */
    float speed;                 /* Horizontal speed in pixels/sec */
    float gravity;               /* Gravity acceleration (px/sec^2) */
    Vector2 bbox_size;           /* Collision box dimensions (12x16 pixels) */
    bool grounded;               /* Is standing on top of floor/platform */
    Color color;                 /* Fallback marker visual color */

    DHAnimationController anim_controller;
    DHAnimState anim_state;
    DHDirection facing_dir;
    bool loaded;

    bool is_attacking;
    float attack_timer;
    float attack_cooldown;
    bool attack_hit_registered[32];
    int gold_count;

    int hp;
    int max_hp;
    float invuln_timer;
    bool is_dead;
    float death_timer;
    Vector2 respawn_pos;

    int level;
    int xp;
    int xp_to_next_level;
    int attack_damage;
    float level_up_notify_timer;
} DHTestPlayer;

void dh_test_player_init(DHTestPlayer *player, Vector2 start_pos, DHAssetManager *assets);
void dh_test_player_add_xp(DHTestPlayer *player, int xp_amount);
void dh_test_player_update(DHTestPlayer *player, const DHInput *input, DHTilemap *tilemap, float dt);
void dh_test_player_draw(const DHTestPlayer *player);
void dh_test_player_shutdown(DHTestPlayer *player, DHAssetManager *assets);

#endif /* DH_PLAYER_H */
