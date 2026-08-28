#ifndef DH_RENDERER_H
#define DH_RENDERER_H

#include "dh/config.h"
#include "dh/camera.h"
#include <raylib.h>
#include <stdbool.h>

struct DHGame;

typedef struct {
    RenderTexture2D target;
    int virtual_width;
    int virtual_height;
    float scale;
    float offset_x;
    float offset_y;
    Rectangle dest_rect;
    bool fullscreen;
} DHRenderer;

bool dh_renderer_init(DHRenderer *renderer, const DHConfig *config);
void dh_renderer_begin_offscreen(DHRenderer *renderer);
void dh_renderer_end_offscreen(DHRenderer *renderer);
void dh_renderer_draw_framebuffer(const DHRenderer *renderer);
void dh_renderer_draw_foundation_ui(const DHRenderer *renderer, const struct DHGame *game, int visible_tiles);
void dh_renderer_shutdown(DHRenderer *renderer);
void dh_renderer_toggle_fullscreen(DHRenderer *renderer);

#endif /* DH_RENDERER_H */
