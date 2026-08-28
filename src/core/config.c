#include "dh/config.h"

DHConfig dh_config_get_default(void)
{
    DHConfig config = {
        .window_width = DH_DEFAULT_WINDOW_WIDTH,
        .window_height = DH_DEFAULT_WINDOW_HEIGHT,
        .virtual_width = DH_VIRTUAL_WIDTH,
        .virtual_height = DH_VIRTUAL_HEIGHT,
        .target_fps = DH_DEFAULT_TARGET_FPS,
        .title = DH_APP_NAME,
        .version = DH_APP_VERSION,
        .fullscreen = false,
        .debug_mode = true,
        .vsync = true
    };

    return config;
}
