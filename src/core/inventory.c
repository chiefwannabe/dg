#include "dh/inventory.h"
#include "dh/logging.h"
#include <raylib.h>
#include <stdio.h>
#include <string.h>

void dh_inventory_init(DHInventory *inv)
{
    if (inv == NULL) return;

    memset(inv, 0, sizeof(DHInventory));
    inv->count = 0;
    inv->equipped_weapon_idx = -1;
    inv->equipped_armor_idx = -1;
    inv->selected_idx = 0;
    inv->show_ui = false;

    /* Add default starter gear: Rusty Sword (+1 DMG) and Cloth Armor (+2 HP) */
    DHItem starter_sword = {
        .id = 1,
        .type = DH_ITEM_WEAPON,
        .name = "Rusty Sword",
        .rarity = DH_RARITY_COMMON,
        .attack_bonus = 1,
        .hp_bonus = 0,
        .gold_value = 10,
        .equipped = false
    };

    DHItem starter_armor = {
        .id = 101,
        .type = DH_ITEM_ARMOR,
        .name = "Cloth Armor",
        .rarity = DH_RARITY_COMMON,
        .attack_bonus = 0,
        .hp_bonus = 2,
        .gold_value = 10,
        .equipped = false
    };

    dh_inventory_add_item(inv, starter_sword);
    dh_inventory_add_item(inv, starter_armor);

    /* Equip starting gear */
    dh_inventory_equip_toggle(inv, 0);
    dh_inventory_equip_toggle(inv, 1);

    dh_log_info("Inventory initialized with starter gear.");
}

bool dh_inventory_add_item(DHInventory *inv, DHItem item)
{
    if (inv == NULL || inv->count >= MAX_INVENTORY_ITEMS) {
        dh_log_warn("Inventory full! Cannot add item %s", item.name);
        return false;
    }

    inv->items[inv->count] = item;
    inv->items[inv->count].equipped = false;
    inv->count++;

    dh_log_info("Added item '%s' to inventory (Count: %d/%d)", item.name, inv->count, MAX_INVENTORY_ITEMS);
    return true;
}

bool dh_inventory_remove_item(DHInventory *inv, int index)
{
    if (inv == NULL || index < 0 || index >= inv->count) return false;

    /* If item was equipped, unequip it first */
    if (inv->items[index].equipped) {
        dh_inventory_equip_toggle(inv, index);
    }

    /* Shift remaining items left */
    for (int i = index; i < inv->count - 1; i++) {
        inv->items[i] = inv->items[i + 1];
    }
    inv->count--;

    /* Fix equipped indices */
    if (inv->equipped_weapon_idx > index) inv->equipped_weapon_idx--;
    if (inv->equipped_armor_idx > index) inv->equipped_armor_idx--;

    if (inv->selected_idx >= inv->count && inv->count > 0) {
        inv->selected_idx = inv->count - 1;
    }

    return true;
}

bool dh_inventory_equip_toggle(DHInventory *inv, int index)
{
    if (inv == NULL || index < 0 || index >= inv->count) return false;

    DHItem *item = &inv->items[index];

    if (item->type == DH_ITEM_WEAPON) {
        if (item->equipped) {
            item->equipped = false;
            inv->equipped_weapon_idx = -1;
            dh_log_info("Unequipped weapon: %s", item->name);
        } else {
            /* Unequip currently equipped weapon */
            if (inv->equipped_weapon_idx >= 0 && inv->equipped_weapon_idx < inv->count) {
                inv->items[inv->equipped_weapon_idx].equipped = false;
            }
            item->equipped = true;
            inv->equipped_weapon_idx = index;
            dh_log_info("Equipped weapon: %s (+%d DMG)", item->name, item->attack_bonus);
        }
        return true;
    } else if (item->type == DH_ITEM_ARMOR) {
        if (item->equipped) {
            item->equipped = false;
            inv->equipped_armor_idx = -1;
            dh_log_info("Unequipped armor: %s", item->name);
        } else {
            /* Unequip currently equipped armor */
            if (inv->equipped_armor_idx >= 0 && inv->equipped_armor_idx < inv->count) {
                inv->items[inv->equipped_armor_idx].equipped = false;
            }
            item->equipped = true;
            inv->equipped_armor_idx = index;
            dh_log_info("Equipped armor: %s (+%d Max HP)", item->name, item->hp_bonus);
        }
        return true;
    }

    return false;
}

int dh_inventory_get_total_attack_bonus(const DHInventory *inv)
{
    if (inv == NULL) return 0;
    int bonus = 0;
    if (inv->equipped_weapon_idx >= 0 && inv->equipped_weapon_idx < inv->count) {
        bonus += inv->items[inv->equipped_weapon_idx].attack_bonus;
    }
    return bonus;
}

int dh_inventory_get_total_hp_bonus(const DHInventory *inv)
{
    if (inv == NULL) return 0;
    int bonus = 0;
    if (inv->equipped_armor_idx >= 0 && inv->equipped_armor_idx < inv->count) {
        bonus += inv->items[inv->equipped_armor_idx].hp_bonus;
    }
    return bonus;
}

void dh_inventory_draw_ui(const DHInventory *inv, int virtual_w, int virtual_h)
{
    if (inv == NULL || !inv->show_ui) return;

    int panel_w = 220;
    int panel_h = 140;
    int panel_x = (virtual_w - panel_w) / 2;
    int panel_y = (virtual_h - panel_h) / 2;

    /* Dark Panel Background */
    DrawRectangle(panel_x, panel_y, panel_w, panel_h, (Color){ 14, 14, 20, 240 });
    DrawRectangleLines(panel_x, panel_y, panel_w, panel_h, (Color){ 200, 160, 50, 255 });

    /* Title */
    DrawText("INVENTORY & EQUIPMENT", panel_x + 8, panel_y + 6, 10, (Color){ 255, 220, 100, 255 });
    DrawLine(panel_x + 6, panel_y + 18, panel_x + panel_w - 6, panel_y + 18, (Color){ 80, 80, 100, 255 });

    /* Equipped Overview */
    const char *wpn_name = (inv->equipped_weapon_idx >= 0) ? inv->items[inv->equipped_weapon_idx].name : "None";
    const char *arm_name = (inv->equipped_armor_idx >= 0) ? inv->items[inv->equipped_armor_idx].name : "None";

    char eq_buf[128];
    snprintf(eq_buf, sizeof(eq_buf), "WPN: %s | ARM: %s", wpn_name, arm_name);
    DrawText(eq_buf, panel_x + 8, panel_y + 22, 8, (Color){ 220, 220, 240, 255 });

    snprintf(eq_buf, sizeof(eq_buf), "BONUS -> ATK: +%d | HP: +%d",
             dh_inventory_get_total_attack_bonus(inv),
             dh_inventory_get_total_hp_bonus(inv));
    DrawText(eq_buf, panel_x + 8, panel_y + 32, 8, (Color){ 100, 220, 120, 255 });

    DrawLine(panel_x + 6, panel_y + 42, panel_x + panel_w - 6, panel_y + 42, (Color){ 80, 80, 100, 255 });

    /* Item List Display */
    int start_y = panel_y + 46;
    for (int i = 0; i < 7 && i < inv->count; i++) {
        const DHItem *item = &inv->items[i];
        Color text_col = (i == inv->selected_idx) ? (Color){ 255, 220, 80, 255 } : RAYWHITE;

        if (i == inv->selected_idx) {
            DrawRectangle(panel_x + 6, start_y + i * 11 - 1, panel_w - 12, 10, (Color){ 40, 40, 60, 255 });
        }

        char item_buf[64];
        snprintf(item_buf, sizeof(item_buf), "[%d] %s %s (%s)",
                 i + 1, item->name, item->equipped ? "[EQ]" : "",
                 (item->type == DH_ITEM_WEAPON) ? "WPN" : "ARM");
        DrawText(item_buf, panel_x + 8, start_y + i * 11, 8, text_col);
    }

    DrawText("1-9: Select | ENTER: Equip/Unequip | TAB/ESC: Close", panel_x + 8, panel_y + panel_h - 12, 8, (Color){ 160, 160, 180, 255 });
}
