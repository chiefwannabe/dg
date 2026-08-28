#include "dh/coords.h"
#include <raylib.h>

Vector2 dh_coords_screen_to_virtual(Vector2 screen_pos, float scale, float offset_x, float offset_y)
{
    Vector2 v;
    if (scale > 0.0001f) {
        v.x = (screen_pos.x - offset_x) / scale;
        v.y = (screen_pos.y - offset_y) / scale;
    } else {
        v.x = 0.0f;
        v.y = 0.0f;
    }
    return v;
}

Vector2 dh_coords_virtual_to_screen(Vector2 virtual_pos, float scale, float offset_x, float offset_y)
{
    Vector2 s;
    s.x = virtual_pos.x * scale + offset_x;
    s.y = virtual_pos.y * scale + offset_y;
    return s;
}

Vector2 dh_coords_virtual_to_world(Vector2 virtual_pos, Vector2 camera_pos, Vector2 virtual_size)
{
    Vector2 w;
    w.x = virtual_pos.x + (camera_pos.x - virtual_size.x * 0.5f);
    w.y = virtual_pos.y + (camera_pos.y - virtual_size.y * 0.5f);
    return w;
}

Vector2 dh_coords_world_to_virtual(Vector2 world_pos, Vector2 camera_pos, Vector2 virtual_size)
{
    Vector2 v;
    v.x = world_pos.x - (camera_pos.x - virtual_size.x * 0.5f);
    v.y = world_pos.y - (camera_pos.y - virtual_size.y * 0.5f);
    return v;
}

Vector2 dh_coords_screen_to_world(Vector2 screen_pos, Vector2 camera_pos, float scale, float offset_x, float offset_y, Vector2 virtual_size)
{
    Vector2 v = dh_coords_screen_to_virtual(screen_pos, scale, offset_x, offset_y);
    return dh_coords_virtual_to_world(v, camera_pos, virtual_size);
}
