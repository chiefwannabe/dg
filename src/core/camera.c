#include "dh/camera.h"
#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <stddef.h>

void dh_camera_init(DHCamera2D *camera, float virtual_w, float virtual_h)
{
    if (camera == NULL) return;

    camera->target = (Vector2){ virtual_w * 0.5f, virtual_h * 0.5f };
    camera->position = camera->target;
    camera->offset = (Vector2){ virtual_w * 0.5f, virtual_h * 0.5f };
    camera->zoom = 1.0f;
    camera->rotation = 0.0f;
    camera->bounds = (Rectangle){ 0, 0, virtual_w, virtual_h };
    camera->use_bounds = false;
    camera->lerp_speed = 8.0f;
    camera->pixel_snap = true;
}

void dh_camera_set_target(DHCamera2D *camera, Vector2 target)
{
    if (camera == NULL) return;
    camera->target = target;
}

void dh_camera_set_bounds(DHCamera2D *camera, Rectangle bounds, bool enable)
{
    if (camera == NULL) return;
    camera->bounds = bounds;
    camera->use_bounds = enable;
}

void dh_camera_update(DHCamera2D *camera, float delta_time)
{
    if (camera == NULL) return;

    float t = 1.0f - expf(-camera->lerp_speed * delta_time);
    camera->position.x = Lerp(camera->position.x, camera->target.x, t);
    camera->position.y = Lerp(camera->position.y, camera->target.y, t);

    if (camera->use_bounds) {
        float half_w = camera->offset.x / camera->zoom;
        float half_h = camera->offset.y / camera->zoom;

        float min_x = camera->bounds.x + half_w;
        float max_x = camera->bounds.x + camera->bounds.width - half_w;
        float min_y = camera->bounds.y + half_h;
        float max_y = camera->bounds.y + camera->bounds.height - half_h;

        if (max_x < min_x) {
            camera->position.x = camera->bounds.x + camera->bounds.width * 0.5f;
        } else {
            if (camera->position.x < min_x) camera->position.x = min_x;
            if (camera->position.x > max_x) camera->position.x = max_x;
        }

        if (max_y < min_y) {
            camera->position.y = camera->bounds.y + camera->bounds.height * 0.5f;
        } else {
            if (camera->position.y < min_y) camera->position.y = min_y;
            if (camera->position.y > max_y) camera->position.y = max_y;
        }
    }
}

Camera2D dh_camera_get_raylib_camera(const DHCamera2D *camera)
{
    Camera2D rcam = {0};
    if (camera == NULL) return rcam;

    rcam.target = camera->position;
    if (camera->pixel_snap) {
        rcam.target.x = roundf(rcam.target.x);
        rcam.target.y = roundf(rcam.target.y);
    }
    rcam.offset = camera->offset;
    rcam.rotation = camera->rotation;
    rcam.zoom = camera->zoom;

    return rcam;
}

Vector2 dh_camera_screen_to_world(const DHCamera2D *camera, Vector2 virtual_pos)
{
    if (camera == NULL) return virtual_pos;
    Camera2D rcam = dh_camera_get_raylib_camera(camera);
    return GetScreenToWorld2D(virtual_pos, rcam);
}

Vector2 dh_camera_world_to_screen(const DHCamera2D *camera, Vector2 world_pos)
{
    if (camera == NULL) return world_pos;
    Camera2D rcam = dh_camera_get_raylib_camera(camera);
    return GetWorldToScreen2D(world_pos, rcam);
}
