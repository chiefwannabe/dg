#ifndef DH_COORDS_H
#define DH_COORDS_H

#include <raylib.h>

typedef struct {
    Vector2 screen;   /* Window / Device physical pixels */
    Vector2 virtual;  /* Offscreen canvas pixels (320x180) */
    Vector2 world;    /* World space pixels */
} DHCoordSet;

Vector2 dh_coords_screen_to_virtual(Vector2 screen_pos, float scale, float offset_x, float offset_y);
Vector2 dh_coords_virtual_to_screen(Vector2 virtual_pos, float scale, float offset_x, float offset_y);
Vector2 dh_coords_virtual_to_world(Vector2 virtual_pos, Vector2 camera_pos, Vector2 virtual_size);
Vector2 dh_coords_world_to_virtual(Vector2 world_pos, Vector2 camera_pos, Vector2 virtual_size);
Vector2 dh_coords_screen_to_world(Vector2 screen_pos, Vector2 camera_pos, float scale, float offset_x, float offset_y, Vector2 virtual_size);

#endif /* DH_COORDS_H */
