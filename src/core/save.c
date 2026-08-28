#include "dh/save.h"
#include "dh/logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__EMSCRIPTEN__)
static const char *SAVE_PATH = "save.dat";
#else
static const char *SAVE_PATH = "save.dat";
#endif

const char *dh_save_get_path(void)
{
    return SAVE_PATH;
}

bool dh_save_game(const DHTestPlayer *player, const DHInventory *inv)
{
    if (player == NULL || inv == NULL) return false;

    DHSaveData data;
    memset(&data, 0, sizeof(DHSaveData));

    data.magic = SAVE_FILE_MAGIC;
    data.version = SAVE_FILE_VERSION;
    data.level = player->level;
    data.xp = player->xp;
    data.xp_to_next_level = player->xp_to_next_level;
    data.gold = player->gold_count;
    data.base_max_hp = player->max_hp;
    data.base_damage = player->attack_damage;

    data.inventory_count = inv->count;
    memcpy(data.inventory_items, inv->items, sizeof(DHItem) * MAX_INVENTORY_ITEMS);
    data.equipped_weapon_idx = inv->equipped_weapon_idx;
    data.equipped_armor_idx = inv->equipped_armor_idx;

    FILE *f = fopen(SAVE_PATH, "wb");
    if (f == NULL) {
        dh_log_error("Failed to open save file '%s' for writing", SAVE_PATH);
        return false;
    }

    size_t written = fwrite(&data, sizeof(DHSaveData), 1, f);
    fclose(f);

    if (written == 1) {
        dh_log_info("Game state saved successfully to '%s' (Level:%d, Gold:%d, Items:%d)",
                    SAVE_PATH, player->level, player->gold_count, inv->count);
        return true;
    }

    dh_log_error("Failed to write full save data");
    return false;
}

bool dh_load_game(DHTestPlayer *player, DHInventory *inv)
{
    if (player == NULL || inv == NULL) return false;

    FILE *f = fopen(SAVE_PATH, "rb");
    if (f == NULL) {
        dh_log_info("No existing save file '%s' found. Starting new game.", SAVE_PATH);
        return false;
    }

    DHSaveData data;
    size_t read_bytes = fread(&data, sizeof(DHSaveData), 1, f);
    fclose(f);

    if (read_bytes != 1 || data.magic != SAVE_FILE_MAGIC || data.version != SAVE_FILE_VERSION) {
        dh_log_warn("Save data corrupted or invalid version! Falling back safely to default game state.");
        return false;
    }

    player->level = data.level;
    player->xp = data.xp;
    player->xp_to_next_level = data.xp_to_next_level;
    player->gold_count = data.gold;
    player->max_hp = data.base_max_hp;
    player->hp = player->max_hp;
    player->attack_damage = data.base_damage;

    inv->count = data.inventory_count;
    memcpy(inv->items, data.inventory_items, sizeof(DHItem) * MAX_INVENTORY_ITEMS);
    inv->equipped_weapon_idx = data.equipped_weapon_idx;
    inv->equipped_armor_idx = data.equipped_armor_idx;

    dh_log_info("Game state loaded successfully from '%s' (Level:%d, Gold:%d, Items:%d)",
                SAVE_PATH, player->level, player->gold_count, inv->count);
    return true;
}
