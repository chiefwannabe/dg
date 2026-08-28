#ifndef DH_SHOP_H
#define DH_SHOP_H

#include "dh/inventory.h"
#include "dh/player.h"
#include <raylib.h>
#include <stdbool.h>

#define SHOP_CATALOG_SIZE 8
#define SHOP_INTERACTION_RADIUS 32.0f

typedef struct {
    DHItem catalog[SHOP_CATALOG_SIZE];
    int selected_idx;
    bool show_ui;
    char message[64];
    float message_timer;
} DHShop;

void dh_shop_init(DHShop *shop);
void dh_shop_update(DHShop *shop, DHTestPlayer *player, DHInventory *inv, float dt);
bool dh_shop_buy_selected(DHShop *shop, DHTestPlayer *player, DHInventory *inv);
void dh_shop_draw_world(const DHShop *shop, float camera_x, float tile_size, Vector2 player_pos);
void dh_shop_draw_ui(const DHShop *shop, const DHTestPlayer *player, int virtual_w, int virtual_h);

#endif /* DH_SHOP_H */
