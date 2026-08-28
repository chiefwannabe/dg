#include "dh/player.h"
#include <math.h>
#include <stddef.h>

void dh_test_player_init(DHTestPlayer *player, Vector2 start_pos)
{
    if (player == NULL) return;

    player->position = start_pos;
    player->velocity = (Vector2){ 0.0f, 0.0f };
    player->speed = 100.0f;     /* 100 pixels/sec horizontal movement */
    player->gravity = 500.0f;   /* Gravity acceleration px/s^2 */
    player->bbox_size = (Vector2){ 12.0f, 16.0f };
    player->grounded = false;
    player->color = (Color){ 80, 210, 255, 255 }; /* Vibrant cyan platformer marker */
}

void dh_test_player_update(DHTestPlayer *player, const DHInput *input, DHTilemap *tilemap, float dt)
{
    if (player == NULL || input == NULL || tilemap == NULL || dt <= 0.0f) {
        return;
    }

    /* 1. Apply Horizontal Movement (A/D or Left/Right arrows only) */
    player->velocity.x = input->move_x * player->speed;

    /* 2. Apply Gravity to Vertical Velocity */
    player->velocity.y += player->gravity * dt;
    if (player->velocity.y > 400.0f) {
        player->velocity.y = 400.0f; /* Terminal velocity cap */
    }

    Vector2 current_pos = player->position;
    float half_w = player->bbox_size.x * 0.5f;
    float half_h = player->bbox_size.y * 0.5f;

    /* 3. Resolve Horizontal X Movement & Wall Collision */
    float target_x = current_pos.x + player->velocity.x * dt;
    Rectangle box_x = {
        target_x - half_w,
        current_pos.y - half_h + 0.1f, /* Small offset to avoid snagging on floor joints */
        player->bbox_size.x,
        player->bbox_size.y - 0.2f
    };

    if (dh_tilemap_check_rect_collision(tilemap, box_x)) {
        player->velocity.x = 0.0f;
    } else {
        current_pos.x = target_x;
    }

    /* 4. Resolve Vertical Y Movement & Floor/Ceiling Collision */
    player->grounded = false;
    float target_y = current_pos.y + player->velocity.y * dt;
    Rectangle box_y = {
        current_pos.x - half_w,
        target_y - half_h,
        player->bbox_size.x,
        player->bbox_size.y
    };

    if (dh_tilemap_check_rect_collision(tilemap, box_y)) {
        if (player->velocity.y > 0.0f) {
            /* Landing on top of a solid floor/platform tile */
            player->grounded = true;
            player->velocity.y = 0.0f;

            /* Snap player precisely to platform top level */
            int foot_ty = (int)floorf((target_y + half_h - 0.001f) / (float)tilemap->tile_size);
            current_pos.y = (float)(foot_ty * tilemap->tile_size) - half_h;
        } else if (player->velocity.y < 0.0f) {
            /* Hitting solid ceiling */
            player->velocity.y = 0.0f;
            int head_ty = (int)floorf((target_y - half_h) / (float)tilemap->tile_size);
            current_pos.y = (float)((head_ty + 1) * tilemap->tile_size) + half_h;
        }
    } else {
        current_pos.y = target_y;
    }

    /* 5. Left World Boundary (Player cannot move left past X = half_w) */
    if (current_pos.x < half_w) {
        current_pos.x = half_w;
        player->velocity.x = 0.0f;
    }

    player->position = current_pos;
}

void dh_test_player_draw(const DHTestPlayer *player)
{
    if (player == NULL) return;

    float half_w = player->bbox_size.x * 0.5f;
    float half_h = player->bbox_size.y * 0.5f;

    /* Draw player shadow on floor */
    DrawEllipse((int)player->position.x, (int)(player->position.y + half_h), half_w, 3.0f, (Color){ 0, 0, 0, 120 });

    /* Draw temporary platformer player rectangle marker */
    Rectangle rect = {
        player->position.x - half_w,
        player->position.y - half_h,
        player->bbox_size.x,
        player->bbox_size.y
    };
    DrawRectangleRec(rect, player->color);
    DrawRectangleLinesEx(rect, 1.0f, (Color){ 255, 255, 255, 220 });

    /* Inner indicator dot showing grounded state (Green when grounded, White when in air) */
    Color status_col = player->grounded ? (Color){ 80, 240, 120, 255 } : (Color){ 255, 255, 255, 255 };
    DrawCircleV(player->position, 2.5f, status_col);
}
