#ifndef DH_CONFIG_H
#define DH_CONFIG_H

#include <stdbool.h>

#define DH_APP_NAME "Dungeon Hunter"
#define DH_APP_VERSION "0.1.0"

#define DH_DEFAULT_WINDOW_WIDTH 960
#define DH_DEFAULT_WINDOW_HEIGHT 540

#define DH_VIRTUAL_WIDTH 320
#define DH_VIRTUAL_HEIGHT 180

#define DH_DEFAULT_TARGET_FPS 60

typedef struct {
    int window_width;
    int window_height;
    int virtual_width;
    int virtual_height;
    int target_fps;
    const char *title;
    const char *version;
    bool fullscreen;
    bool debug_mode;
    bool vsync;
} DHConfig;

DHConfig dh_config_get_default(void);

#endif /* DH_CONFIG_H */
