#include "dh/shop.h"
#include "dh/logging.h"
#include <raylib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

void dh_shop_init(DHShop *shop)
{
    if (shop == NULL) return;

    memset(shop, 0, sizeof(DHShop));
    shop->selected_idx = 0;
    shop->show_ui = false;
    shop->message_timer = 0.0f;

    /* Catalog Items */
    shop->catalog[0] = (DHItem){ .id = 1,  .type = DH_ITEM_WEAPON, .name = "Rusty Sword",  .rarity = DH_RARITY_COMMON,   .attack_bonus = 1, .hp_bonus = 0,  .gold_value = 10 };
    shop->catalog[1] = (DHItem){ .id = 2,  .type = DH_ITEM_WEAPON, .name = "Iron Sword",   .rarity = DH_RARITY_UNCOMMON, .attack_bonus = 2, .hp_bonus = 0,  .gold_value = 50 };
    shop->catalog[2] = (DHItem){ .id = 3,  .type = DH_ITEM_WEAPON, .name = "Knight Sword", .rarity = DH_RARITY_RARE,     .attack_bonus = 4, .hp_bonus = 0,  .gold_value = 150 };
    shop->catalog[3] = (DHItem){ .id = 4,  .type = DH_ITEM_WEAPON, .name = "Demon Blade",  .rarity = DH_RARITY_EPIC,     .attack_bonus = 7, .hp_bonus = 0,  .gold_value = 350 };

    shop->catalog[4] = (DHItem){ .id = 101, .type = DH_ITEM_ARMOR, .name = "Cloth Armor",  .rarity = DH_RARITY_COMMON,   .attack_bonus = 0, .hp_bonus = 2,  .gold_value = 10 };
    shop->catalog[5] = (DHItem){ .id = 102, .type = DH_ITEM_ARMOR, .name = "Iron Armor",   .rarity = DH_RARITY_UNCOMMON, .attack_bonus = 0, .hp_bonus = 5,  .gold_value = 50 };
    shop->catalog[6] = (DHItem){ .id = 103, .type = DH_ITEM_ARMOR, .name = "Knight Armor", .rarity = DH_RARITY_RARE,     .attack_bonus = 0, .hp_bonus = 10, .gold_value = 150 };
    shop->catalog[7] = (DHItem){ .id = 104, .type = DH_ITEM_ARMOR, .name = "Demon Armor",  .rarity = DH_RARITY_EPIC,     .attack_bonus = 0, .hp_bonus = 20, .gold_value = 350 };

    dh_log_info("Shop merchant catalog initialized (%d items).", SHOP_CATALOG_SIZE);
}

void dh_shop_update(DHShop *shop, DHTestPlayer *player, DHInventory *inv, float dt)
{
    (void)player;
    (void)inv;
    if (shop == NULL) return;

    if (shop->message_timer > 0.0f) {
        shop->message_timer -= dt;
        if (shop->message_timer < 0.0f) shop->message_timer = 0.0f;
    }
}

bool dh_shop_buy_selected(DHShop *shop, DHTestPlayer *player, DHInventory *inv)
{
    if (shop == NULL || player == NULL || inv == NULL) return false;

    if (shop->selected_idx < 0 || shop->selected_idx >= SHOP_CATALOG_SIZE) return false;

    DHItem item = shop->catalog[shop->selected_idx];

    if (inv->count >= MAX_INVENTORY_ITEMS) {
        snprintf(shop->message, sizeof(shop->message), "INVENTORY FULL!");
        shop->message_timer = 2.0f;
        dh_log_warn("Shop purchase failed: Inventory full");
        return false;
    }

    if (player->gold_count < item.gold_value) {
        snprintf(shop->message, sizeof(shop->message), "NOT ENOUGH GOLD!");
        shop->message_timer = 2.0f;
        dh_log_warn("Shop purchase failed: Insufficient Gold (Have: %d, Need: %d)", player->gold_count, item.gold_value);
        return false;
    }

    player->gold_count -= item.gold_value;
    dh_inventory_add_item(inv, item);

    snprintf(shop->message, sizeof(shop->message), "PURCHASED %s!", item.name);
    shop->message_timer = 2.0f;
    dh_log_info("Shop purchased '%s' for %d Gold! Remaining Gold: %d", item.name, item.gold_value, player->gold_count);
    return true;
}

void dh_shop_draw_world(const DHShop *shop, float camera_x, float tile_size, Vector2 player_pos)
{
    (void)shop;
    float ts = (tile_size > 0.0f) ? tile_size : 16.0f;

    /* Merchant stands at every 1500 world px milestone */
    int milestone = (int)floorf(camera_x / 1500.0f);
    if (milestone <= 0) milestone = 1;

    float shop_x = (float)milestone * 1500.0f + 120.0f;
    float shop_y = 10.0f * ts - 8.0f;

    /* Render Merchant Stand NPC */
    DrawRectangle((int)(shop_x - 8.0f), (int)(shop_y - 14.0f), 16, 16, (Color){ 80, 180, 220, 255 });
    DrawRectangleLines((int)(shop_x - 8.0f), (int)(shop_y - 14.0f), 16, 16, (Color){ 20, 40, 80, 255 });

    /* Prompt Banner when in range */
    float dist = fabsf(player_pos.x - shop_x);
    if (dist <= SHOP_INTERACTION_RADIUS) {
        DrawRectangle((int)(shop_x - 45.0f), (int)(shop_y - 28.0f), 90, 12, (Color){ 15, 15, 25, 230 });
        DrawRectangleLines((int)(shop_x - 45.0f), (int)(shop_y - 28.0f), 90, 12, (Color){ 255, 220, 80, 255 });
        DrawText("SHOP (Press E)", (int)(shop_x - 41.0f), (int)(shop_y - 26.0f), 8, (Color){ 255, 220, 80, 255 });
    }
}

void dh_shop_draw_ui(const DHShop *shop, const DHTestPlayer *player, int virtual_w, int virtual_h)
{
    if (shop == NULL || !shop->show_ui) return;

    int panel_w = 220;
    int panel_h = 140;
    int panel_x = (virtual_w - panel_w) / 2;
    int panel_y = (virtual_h - panel_h) / 2;

    /* Dark Panel */
    DrawRectangle(panel_x, panel_y, panel_w, panel_h, (Color){ 14, 14, 20, 240 });
    DrawRectangleLines(panel_x, panel_y, panel_w, panel_h, (Color){ 80, 180, 220, 255 });

    /* Header */
    char hdr_buf[64];
    snprintf(hdr_buf, sizeof(hdr_buf), "DUNGEON SHOP | GOLD: %dG", player ? player->gold_count : 0);
    DrawText(hdr_buf, panel_x + 8, panel_y + 6, 10, (Color){ 255, 220, 100, 255 });
    DrawLine(panel_x + 6, panel_y + 18, panel_x + panel_w - 6, panel_y + 18, (Color){ 80, 80, 100, 255 });

    /* Catalog List */
    int start_y = panel_y + 22;
    for (int i = 0; i < SHOP_CATALOG_SIZE; i++) {
        const DHItem *item = &shop->catalog[i];
        Color col = (i == shop->selected_idx) ? (Color){ 255, 220, 80, 255 } : RAYWHITE;

        if (i == shop->selected_idx) {
            DrawRectangle(panel_x + 6, start_y + i * 11 - 1, panel_w - 12, 10, (Color){ 40, 40, 60, 255 });
        }

        char item_buf[64];
        snprintf(item_buf, sizeof(item_buf), "[%d] %s (%s) - %dG",
                 i + 1, item->name, (item->type == DH_ITEM_WEAPON) ? "+DMG" : "+HP", item->gold_value);
        DrawText(item_buf, panel_x + 8, start_y + i * 11, 8, col);
    }

    /* Purchase Status Message */
    if (shop->message_timer > 0.0f) {
        DrawText(shop->message, panel_x + 8, panel_y + panel_h - 22, 8, (Color){ 255, 100, 100, 255 });
    }

    DrawText("1-8: Select | ENTER: Buy | E/ESC: Exit", panel_x + 8, panel_y + panel_h - 12, 8, (Color){ 160, 160, 180, 255 });
}
