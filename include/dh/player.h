#ifndef DH_PLAYER_H
#define DH_PLAYER_H

#include "dh/input.h"
#include "dh/tilemap.h"
#include <raylib.h>
#include <stdbool.h>

typedef struct {
    Vector2 position;      /* Center position in world pixels */
    Vector2 velocity;      /* Current velocity (vx, vy) */
    float speed;           /* Horizontal speed in pixels/sec */
    float gravity;         /* Gravity acceleration (px/sec^2) */
    Vector2 bbox_size;     /* Collision box dimensions (12x16 pixels) */
    bool grounded;         /* Is standing on top of floor/platform */
    Color color;           /* Temporary platformer marker visual color */
} DHTestPlayer;

void dh_test_player_init(DHTestPlayer *player, Vector2 start_pos);
void dh_test_player_update(DHTestPlayer *player, const DHInput *input, DHTilemap *tilemap, float dt);
void dh_test_player_draw(const DHTestPlayer *player);

#endif /* DH_PLAYER_H */
