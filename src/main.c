#include "dh/game.h"
#include "dh/config.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>

static DHGame g_web_game;

static void web_main_loop(void)
{
    if (g_web_game.running) {
        dh_game_update(&g_web_game);
        dh_game_draw(&g_web_game);
    } else {
        dh_game_shutdown(&g_web_game);
        emscripten_cancel_main_loop();
    }
}

int main(void)
{
    DHConfig config = dh_config_get_default();
    if (!dh_game_init(&g_web_game, config)) {
        return 1;
    }

    emscripten_set_main_loop(web_main_loop, 0, 1);
    return 0;
}

#else

int main(void)
{
    DHConfig config = dh_config_get_default();
    DHGame game = {0};

    if (!dh_game_init(&game, config)) {
        return 1;
    }

    while (game.running) {
        dh_game_update(&game);
        dh_game_draw(&game);
    }

    dh_game_shutdown(&game);
    return 0;
}

#endif
