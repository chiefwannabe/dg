#include "dh/input.h"
#include "dh/coords.h"
#include <raylib.h>
#include <stddef.h>
#include <math.h>

void dh_input_init(DHInput *input)
{
    if (input == NULL) {
        return;
    }

    for (int i = 0; i < DH_INPUT_COUNT; i++) {
        input->down[i] = false;
        input->pressed[i] = false;
        input->released[i] = false;
    }

    input->move_x = 0.0f;
    input->move_y = 0.0f;
    input->raw_mouse_screen = (Vector2){0, 0};
    input->virtual_mouse = (Vector2){0, 0};
    input->world_mouse = (Vector2){0, 0};
    input->mouse_clicked = false;
    input->is_dragging = false;
    input->drag_delta_virtual = (Vector2){0, 0};
    input->touch_active = false;
}

void dh_input_update(DHInput *input, int virtual_width, int virtual_height, float scale, float offset_x, float offset_y, Camera2D raylib_cam)
{
    if (input == NULL) {
        return;
    }

    bool prev_down[DH_INPUT_COUNT];
    for (int i = 0; i < DH_INPUT_COUNT; i++) {
        prev_down[i] = input->down[i];
        input->pressed[i] = false;
        input->released[i] = false;
    }

    /* Gameplay controls: A/D move horizontally. W/S do NOT move player. */
    bool key_up = IsKeyDown(KEY_UP);
    bool key_down = IsKeyDown(KEY_DOWN);
    bool key_left = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
    bool key_right = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);

    bool act_primary = IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_ENTER) || IsKeyDown(KEY_Z);
    bool act_secondary = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_X);
    bool pause_btn = IsKeyDown(KEY_ESCAPE) || IsKeyDown(KEY_P);
    bool reset_cam = IsKeyDown(KEY_R);

    if (IsGamepadAvailable(0)) {
        float stick_x = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        float stick_y = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);

        if (fabsf(stick_x) > 0.2f) {
            if (stick_x < 0.0f) key_left = true;
            else key_right = true;
        }
        if (fabsf(stick_y) > 0.2f) {
            if (stick_y < 0.0f) key_up = true;
            else key_down = true;
        }

        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP)) key_up = true;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) key_down = true;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) key_left = true;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) key_right = true;

        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) act_primary = true;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) act_secondary = true;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)) pause_btn = true;
    }

    bool new_down[DH_INPUT_COUNT];
    new_down[DH_INPUT_UP] = key_up;
    new_down[DH_INPUT_DOWN] = key_down;
    new_down[DH_INPUT_LEFT] = key_left;
    new_down[DH_INPUT_RIGHT] = key_right;
    new_down[DH_INPUT_ACTION_PRIMARY] = act_primary;
    new_down[DH_INPUT_ACTION_SECONDARY] = act_secondary;
    new_down[DH_INPUT_PAUSE] = pause_btn;
    new_down[DH_INPUT_RESET_CAM] = reset_cam;

    for (int i = 0; i < DH_INPUT_COUNT; i++) {
        input->pressed[i] = (!prev_down[i] && new_down[i]);
        input->released[i] = (prev_down[i] && !new_down[i]);
        input->down[i] = new_down[i];
    }

    input->move_x = (float)input->down[DH_INPUT_RIGHT] - (float)input->down[DH_INPUT_LEFT];
    input->move_y = 0.0f; /* Side-scrolling platformer: W/S do NOT move player vertically */

    Vector2 raw_mouse = GetMousePosition();
    if (GetTouchPointCount() > 0) {
        raw_mouse = GetTouchPosition(0);
        input->touch_active = true;
    } else {
        input->touch_active = false;
    }
    input->raw_mouse_screen = raw_mouse;

    Vector2 prev_v = input->virtual_mouse;
    input->virtual_mouse = dh_coords_screen_to_virtual(raw_mouse, scale, offset_x, offset_y);

    if (input->virtual_mouse.x < 0.0f) input->virtual_mouse.x = 0.0f;
    if (input->virtual_mouse.x > (float)virtual_width) input->virtual_mouse.x = (float)virtual_width;
    if (input->virtual_mouse.y < 0.0f) input->virtual_mouse.y = 0.0f;
    if (input->virtual_mouse.y > (float)virtual_height) input->virtual_mouse.y = (float)virtual_height;

    input->world_mouse = GetScreenToWorld2D(input->virtual_mouse, raylib_cam);

    bool pointer_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT) || (GetTouchPointCount() > 0);
    if (pointer_down) {
        if (!input->is_dragging) {
            input->is_dragging = true;
            input->drag_delta_virtual = (Vector2){0, 0};
        } else {
            input->drag_delta_virtual.x = input->virtual_mouse.x - prev_v.x;
            input->drag_delta_virtual.y = input->virtual_mouse.y - prev_v.y;
        }
    } else {
        input->is_dragging = false;
        input->drag_delta_virtual = (Vector2){0, 0};
    }

    input->mouse_clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool dh_input_is_down(const DHInput *input, DHInputAction action)
{
    if (input == NULL || action >= DH_INPUT_COUNT) return false;
    return input->down[action];
}

bool dh_input_is_pressed(const DHInput *input, DHInputAction action)
{
    if (input == NULL || action >= DH_INPUT_COUNT) return false;
    return input->pressed[action];
}

bool dh_input_is_released(const DHInput *input, DHInputAction action)
{
    if (input == NULL || action >= DH_INPUT_COUNT) return false;
    return input->released[action];
}
