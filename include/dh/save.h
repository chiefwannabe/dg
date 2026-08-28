#ifndef DH_SAVE_H
#define DH_SAVE_H

#include "dh/player.h"
#include "dh/inventory.h"
#include <stdbool.h>

#define SAVE_FILE_MAGIC 0x44485356 /* 'DHSV' */
#define SAVE_FILE_VERSION 1

typedef struct {
    unsigned int magic;
    unsigned int version;
    int level;
    int xp;
    int xp_to_next_level;
    int gold;
    int base_max_hp;
    int base_damage;
    int inventory_count;
    DHItem inventory_items[MAX_INVENTORY_ITEMS];
    int equipped_weapon_idx;
    int equipped_armor_idx;
} DHSaveData;

bool dh_save_game(const DHTestPlayer *player, const DHInventory *inv);
bool dh_load_game(DHTestPlayer *player, DHInventory *inv);
const char *dh_save_get_path(void);

#endif /* DH_SAVE_H */
