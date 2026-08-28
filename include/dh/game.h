#ifndef DH_GAME_H
#define DH_GAME_H

#include "dh/config.h"
#include "dh/time.h"
#include "dh/input.h"
#include "dh/renderer.h"
#include "dh/assets.h"
#include "dh/camera.h"
#include "dh/world.h"
#include "dh/player.h"
#include "dh/enemy.h"
#include "dh/combat.h"
#include "dh/loot.h"
#include "dh/boss.h"
#include "dh/inventory.h"
#include "dh/shop.h"
#include "dh/save.h"
#include "dh/audio.h"
#include "dh/animation.h"
#include "dh/anim_demo.h"
#include <stdbool.h>

typedef enum {
    DH_STATE_BOOT = 0,
    DH_STATE_MENU,
    DH_STATE_PLAYING,
    DH_STATE_PAUSED,
    DH_STATE_EXIT
} DHGameState;

typedef struct DHGame {
    DHConfig config;
    DHGameState state;
    bool running;
    DHTime time;
    DHInput input;
    DHRenderer renderer;
    DHAssetManager assets;
    DHCamera2D camera;
    DHWorld world;
    DHTestPlayer player;
    DHEnemyManager enemies;
    DHCombatSystem combat;
    DHPickupManager loot;
    DHBossManager boss_mgr;
    DHInventory inventory;
    DHShop shop;
    DHAudioSystem audio;
    DHAnimDemo anim_demo;
    int menu_selected_idx;
    bool has_save_file;
    bool demo_mode;
    bool show_collision_debug;
    bool show_enemy_debug;
    bool show_combat_debug;
    bool show_loot_debug;
    bool show_boss_debug;
    bool show_inventory_debug;
    int visible_tiles_count;
} DHGame;

bool dh_game_init(DHGame *game, DHConfig config);
void dh_game_update(DHGame *game);
void dh_game_draw(DHGame *game);
void dh_game_shutdown(DHGame *game);

#endif /* DH_GAME_H */
