#ifndef DH_CAMERA_H
#define DH_CAMERA_H

#include <raylib.h>
#include <stdbool.h>

typedef struct {
    Vector2 target;       /* World target position to follow */
    Vector2 position;     /* Current camera focal center position in world space */
    Vector2 offset;       /* Screen center offset (virtual_width/2, virtual_height/2) */
    float zoom;           /* Camera zoom level (default 1.0f) */
    float rotation;       /* Camera rotation angle in degrees */
    Rectangle bounds;     /* Bounding rectangle for world clamping */
    bool use_bounds;      /* Whether camera is clamped to bounds */
    float lerp_speed;     /* Smoothing factor for camera movement */
    bool pixel_snap;      /* Integer pixel snapping to prevent rendering subpixel jitter */
} DHCamera2D;

void dh_camera_init(DHCamera2D *camera, float virtual_w, float virtual_h);
void dh_camera_set_target(DHCamera2D *camera, Vector2 target);
void dh_camera_set_bounds(DHCamera2D *camera, Rectangle bounds, bool enable);
void dh_camera_update(DHCamera2D *camera, float delta_time);
Camera2D dh_camera_get_raylib_camera(const DHCamera2D *camera);
Vector2 dh_camera_screen_to_world(const DHCamera2D *camera, Vector2 virtual_pos);
Vector2 dh_camera_world_to_screen(const DHCamera2D *camera, Vector2 world_pos);

#endif /* DH_CAMERA_H */
