#ifndef DH_INVENTORY_H
#define DH_INVENTORY_H

#include <stdbool.h>

#define MAX_INVENTORY_ITEMS 24

typedef enum {
    DH_ITEM_WEAPON = 0,
    DH_ITEM_ARMOR,
    DH_ITEM_POTION,
    DH_ITEM_TYPE_COUNT
} DHItemType;

typedef enum {
    DH_RARITY_COMMON = 0,
    DH_RARITY_UNCOMMON,
    DH_RARITY_RARE,
    DH_RARITY_EPIC,
    DH_RARITY_COUNT
} DHRarity;

typedef struct {
    int id;
    DHItemType type;
    char name[32];
    DHRarity rarity;
    int attack_bonus;
    int hp_bonus;
    int gold_value;
    bool equipped;
} DHItem;

typedef struct {
    DHItem items[MAX_INVENTORY_ITEMS];
    int count;
    int equipped_weapon_idx; /* Index in items array (-1 if none) */
    int equipped_armor_idx;  /* Index in items array (-1 if none) */
    int selected_idx;
    bool show_ui;
} DHInventory;

void dh_inventory_init(DHInventory *inv);
bool dh_inventory_add_item(DHInventory *inv, DHItem item);
bool dh_inventory_remove_item(DHInventory *inv, int index);
bool dh_inventory_equip_toggle(DHInventory *inv, int index);

int dh_inventory_get_total_attack_bonus(const DHInventory *inv);
int dh_inventory_get_total_hp_bonus(const DHInventory *inv);

void dh_inventory_draw_ui(const DHInventory *inv, int virtual_w, int virtual_h);

#endif /* DH_INVENTORY_H */
