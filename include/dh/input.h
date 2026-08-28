#ifndef DH_INPUT_H
#define DH_INPUT_H

#include <raylib.h>
#include <stdbool.h>

typedef enum {
    DH_INPUT_UP = 0,
    DH_INPUT_DOWN,
    DH_INPUT_LEFT,
    DH_INPUT_RIGHT,
    DH_INPUT_ACTION_PRIMARY,
    DH_INPUT_ACTION_SECONDARY,
    DH_INPUT_PAUSE,
    DH_INPUT_RESET_CAM,
    DH_INPUT_COUNT
} DHInputAction;

typedef struct {
    bool down[DH_INPUT_COUNT];
    bool pressed[DH_INPUT_COUNT];
    bool released[DH_INPUT_COUNT];
    float move_x;
    float move_y;
    Vector2 raw_mouse_screen;
    Vector2 virtual_mouse;
    Vector2 world_mouse;
    bool mouse_clicked;
    bool is_dragging;
    Vector2 drag_delta_virtual;
    bool touch_active;
} DHInput;

void dh_input_init(DHInput *input);
void dh_input_update(DHInput *input, int virtual_width, int virtual_height, float scale, float offset_x, float offset_y, Camera2D raylib_cam);

bool dh_input_is_down(const DHInput *input, DHInputAction action);
bool dh_input_is_pressed(const DHInput *input, DHInputAction action);
bool dh_input_is_released(const DHInput *input, DHInputAction action);

#endif /* DH_INPUT_H */
