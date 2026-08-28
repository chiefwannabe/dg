#include "dh/loot.h"
#include "dh/logging.h"
#include <math.h>
#include <string.h>

void dh_loot_init(DHPickupManager *mgr)
{
    if (mgr == NULL) return;

    memset(mgr, 0, sizeof(DHPickupManager));
    for (int i = 0; i < DH_MAX_PICKUPS; i++) {
        mgr->pool[i].id = i;
        mgr->pool[i].active = false;
    }
    mgr->active_count = 0;

    dh_log_info("Loot pickup manager initialized (Pool size: %d pickups)", DH_MAX_PICKUPS);
}

bool dh_loot_spawn_pickup(DHPickupManager *mgr, DHPickupType type, Vector2 position, int value)
{
    if (mgr == NULL) return false;

    int slot = -1;
    for (int i = 0; i < DH_MAX_PICKUPS; i++) {
        if (!mgr->pool[i].active) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        dh_log_warn("Pickup pool full! Failed to spawn pickup.");
        return false;
    }

    DHPickup *p = &mgr->pool[slot];
    p->id = slot;
    p->type = type;
    p->active = true;
    p->position = position;
    p->bbox_size = (Vector2){ 10.0f, 10.0f };
    p->value = value;
    p->bob_timer = (float)(slot * 0.5f);

    dh_log_info("Spawned Gold Pickup #%d at Pos:(%.0f,%.0f) Value:%d", slot, position.x, position.y, value);
    return true;
}

void dh_loot_update(DHPickupManager *mgr, Rectangle player_box, int *out_gold_collected, float dt)
{
    if (mgr == NULL || dt <= 0.0f) return;

    int active_cnt = 0;

    for (int i = 0; i < DH_MAX_PICKUPS; i++) {
        DHPickup *p = &mgr->pool[i];
        if (!p->active) continue;

        active_cnt++;
        p->bob_timer += dt * 4.0f;

        float bob_offset = sinf(p->bob_timer) * 1.5f;
        Rectangle p_box = { p->position.x - 5.0f, p->position.y - 5.0f + bob_offset, p->bbox_size.x, p->bbox_size.y };

        if (CheckCollisionRecs(player_box, p_box)) {
            p->active = false;
            if (out_gold_collected != NULL) {
                *out_gold_collected += p->value;
            }
            dh_log_info("Player collected Gold Pickup #%d! (+%d Gold)", p->id, p->value);
        }
    }

    mgr->active_count = active_cnt;
}

void dh_loot_draw(const DHPickupManager *mgr)
{
    if (mgr == NULL) return;

    for (int i = 0; i < DH_MAX_PICKUPS; i++) {
        const DHPickup *p = &mgr->pool[i];
        if (!p->active) continue;

        float bob_offset = sinf(p->bob_timer) * 1.5f;
        int px = (int)p->position.x;
        int py = (int)(p->position.y + bob_offset);

        /* Small floor shadow */
        DrawEllipse(px, (int)(p->position.y + 4.0f), 4.5f, 1.5f, (Color){ 0, 0, 0, 110 });

        /* Pixel-art Gold Coin */
        DrawCircle(px, py, 4.0f, (Color){ 255, 215, 0, 255 });
        DrawCircleLines(px, py, 4.0f, (Color){ 180, 130, 0, 255 });
        DrawRectangle(px - 1, py - 2, 2, 4, (Color){ 255, 245, 140, 255 });
    }
}

void dh_loot_draw_debug(const DHPickupManager *mgr)
{
    if (mgr == NULL) return;

    for (int i = 0; i < DH_MAX_PICKUPS; i++) {
        const DHPickup *p = &mgr->pool[i];
        if (!p->active) continue;

        float bob_offset = sinf(p->bob_timer) * 1.5f;
        Rectangle p_box = { p->position.x - 5.0f, p->position.y - 5.0f + bob_offset, p->bbox_size.x, p->bbox_size.y };
        DrawRectangleLinesEx(p_box, 1.0f, (Color){ 255, 220, 80, 240 });
    }
}
