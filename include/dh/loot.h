#ifndef DH_LOOT_H
#define DH_LOOT_H

#include "dh/tilemap.h"
#include <raylib.h>
#include <stdbool.h>

#define DH_MAX_PICKUPS 64

typedef enum {
    DH_PICKUP_GOLD = 0,
    DH_PICKUP_HEALTH,
    DH_PICKUP_WEAPON,
    DH_PICKUP_ARMOR,
    DH_PICKUP_TYPE_COUNT
} DHPickupType;

typedef struct {
    int id;
    DHPickupType type;
    bool active;
    Vector2 position;
    Vector2 bbox_size;
    int value;
    float bob_timer;
} DHPickup;

typedef struct {
    DHPickup pool[DH_MAX_PICKUPS];
    int active_count;
} DHPickupManager;

void dh_loot_init(DHPickupManager *mgr);
bool dh_loot_spawn_pickup(DHPickupManager *mgr, DHPickupType type, Vector2 position, int value);
void dh_loot_update(DHPickupManager *mgr, Rectangle player_box, int *out_gold_collected, float dt);
void dh_loot_draw(const DHPickupManager *mgr);
void dh_loot_draw_debug(const DHPickupManager *mgr);

#endif /* DH_LOOT_H */
