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
    DHAnimDemo anim_demo;
    bool demo_mode;
    bool show_collision_debug;
    int visible_tiles_count;
} DHGame;

bool dh_game_init(DHGame *game, DHConfig config);
void dh_game_update(DHGame *game);
void dh_game_draw(DHGame *game);
void dh_game_shutdown(DHGame *game);

#endif /* DH_GAME_H */
