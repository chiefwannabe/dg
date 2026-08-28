#include "dh/animation.h"
#include "dh/logging.h"
#include <stdio.h>
#include <string.h>

bool dh_animation_clip_create_grid(DHAnimationClip *clip,
                                   const char *name,
                                   Texture2D texture,
                                   int start_x,
                                   int start_y,
                                   int frame_width,
                                   int frame_height,
                                   int frame_count,
                                   float frame_duration,
                                   bool looping,
                                   bool horizontal)
{
    if (clip == NULL) {
        return false;
    }

    memset(clip, 0, sizeof(DHAnimationClip));

    if (frame_count <= 0 || frame_count > DH_MAX_ANIM_FRAMES ||
        frame_width <= 0 || frame_height <= 0 ||
        start_x < 0 || start_y < 0) {
        dh_log_error("Invalid animation clip parameters: count=%d, size=%dx%d, start=(%d,%d)",
                     frame_count, frame_width, frame_height, start_x, start_y);
        return false;
    }

    if (texture.id != 0 && texture.width > 0 && texture.height > 0) {
        int max_x = start_x + (horizontal ? (frame_count * frame_width) : frame_width);
        int max_y = start_y + (horizontal ? frame_height : (frame_count * frame_height));
        if (max_x > texture.width || max_y > texture.height) {
            dh_log_error("Animation clip dimensions (%dx%d) exceed texture size (%dx%d)",
                         max_x, max_y, texture.width, texture.height);
            return false;
        }
    }

    if (name != NULL) {
        snprintf(clip->name, sizeof(clip->name), "%s", name);
    } else {
        snprintf(clip->name, sizeof(clip->name), "unnamed");
    }

    clip->texture = texture;
    clip->frame_count = frame_count;
    clip->looping = looping;

    for (int i = 0; i < frame_count; i++) {
        float x = (float)(start_x + (horizontal ? (i * frame_width) : 0));
        float y = (float)(start_y + (horizontal ? 0 : (i * frame_height)));

        clip->frames[i].frame_rect = (Rectangle){ x, y, (float)frame_width, (float)frame_height };
        clip->frames[i].duration = (frame_duration > 0.001f) ? frame_duration : 0.1f;
    }

    return true;
}

void dh_animation_controller_init(DHAnimationController *controller)
{
    if (controller == NULL) {
        return;
    }

    memset(controller, 0, sizeof(DHAnimationController));
    controller->current_state = DH_ANIM_STATE_IDLE;
    controller->current_dir = DH_DIR_DOWN;
    controller->current_frame = 0;
    controller->timer = 0.0f;
    controller->speed = 1.0f;
    controller->playback_state = DH_PLAYBACK_PLAYING;
}

void dh_animation_controller_add_clip(DHAnimationController *controller,
                                      DHAnimState state,
                                      DHDirection dir,
                                      const DHAnimationClip *clip)
{
    if (controller == NULL || clip == NULL) {
        return;
    }

    if (state < 0 || state >= DH_ANIM_STATE_COUNT || dir < 0 || dir >= DH_DIR_COUNT) {
        dh_log_error("Invalid state (%d) or dir (%d) when adding animation clip", state, dir);
        return;
    }

    controller->clips[state][dir] = *clip;
    controller->clip_valid[state][dir] = true;
}

static void resolve_active_clip(const DHAnimationController *controller,
                                DHAnimState state,
                                DHDirection dir,
                                DHAnimState *out_state,
                                DHDirection *out_dir,
                                const DHAnimationClip **out_clip)
{
    *out_state = state;
    *out_dir = dir;
    *out_clip = NULL;

    if (controller == NULL) {
        return;
    }

    // 1. Direct match
    if (state >= 0 && state < DH_ANIM_STATE_COUNT && dir >= 0 && dir < DH_DIR_COUNT) {
        if (controller->clip_valid[state][dir]) {
            *out_clip = &controller->clips[state][dir];
            return;
        }
    }

    // 2. Side-scrolling platformer direction mapping (SIDE_LEFT -> SIDE_RIGHT, LEFT -> RIGHT)
    if (dir == DH_DIR_SIDE_LEFT && controller->clip_valid[state][DH_DIR_SIDE_RIGHT]) {
        *out_dir = DH_DIR_SIDE_RIGHT;
        *out_clip = &controller->clips[state][DH_DIR_SIDE_RIGHT];
        return;
    }
    if (dir == DH_DIR_LEFT && controller->clip_valid[state][DH_DIR_RIGHT]) {
        *out_dir = DH_DIR_RIGHT;
        *out_clip = &controller->clips[state][DH_DIR_RIGHT];
        return;
    }

    // 3. Fallback direction for requested state (e.g. DOWN or RIGHT)
    if (state >= 0 && state < DH_ANIM_STATE_COUNT) {
        if (controller->clip_valid[state][DH_DIR_DOWN]) {
            *out_dir = DH_DIR_DOWN;
            *out_clip = &controller->clips[state][DH_DIR_DOWN];
            return;
        }
        for (int d = 0; d < DH_DIR_COUNT; d++) {
            if (controller->clip_valid[state][d]) {
                *out_dir = (DHDirection)d;
                *out_clip = &controller->clips[state][d];
                return;
            }
        }
    }

    // 4. Fallback to IDLE state
    if (controller->clip_valid[DH_ANIM_STATE_IDLE][dir]) {
        *out_state = DH_ANIM_STATE_IDLE;
        *out_clip = &controller->clips[DH_ANIM_STATE_IDLE][dir];
        return;
    }
    if (controller->clip_valid[DH_ANIM_STATE_IDLE][DH_DIR_DOWN]) {
        *out_state = DH_ANIM_STATE_IDLE;
        *out_dir = DH_DIR_DOWN;
        *out_clip = &controller->clips[DH_ANIM_STATE_IDLE][DH_DIR_DOWN];
        return;
    }

    // 5. Any valid clip at all
    for (int s = 0; s < DH_ANIM_STATE_COUNT; s++) {
        for (int d = 0; d < DH_DIR_COUNT; d++) {
            if (controller->clip_valid[s][d]) {
                *out_state = (DHAnimState)s;
                *out_dir = (DHDirection)d;
                *out_clip = &controller->clips[s][d];
                return;
            }
        }
    }
}

bool dh_animation_play(DHAnimationController *controller,
                        DHAnimState state,
                        DHDirection dir,
                        bool restart_if_same)
{
    if (controller == NULL) {
        return false;
    }

    DHAnimState target_state;
    DHDirection target_dir;
    const DHAnimationClip *clip;
    resolve_active_clip(controller, state, dir, &target_state, &target_dir, &clip);

    if (clip == NULL) {
        controller->current_state = state;
        controller->current_dir = dir;
        controller->current_frame = 0;
        controller->timer = 0.0f;
        controller->playback_state = DH_PLAYBACK_FINISHED;
        return false;
    }

    if (!restart_if_same &&
        controller->current_state == target_state &&
        controller->current_dir == target_dir &&
        controller->playback_state == DH_PLAYBACK_PLAYING) {
        return true;
    }

    controller->current_state = target_state;
    controller->current_dir = target_dir;
    controller->current_frame = 0;
    controller->timer = 0.0f;
    controller->playback_state = DH_PLAYBACK_PLAYING;
    return true;
}

void dh_animation_update(DHAnimationController *controller, float delta_time)
{
    if (controller == NULL || controller->playback_state != DH_PLAYBACK_PLAYING) {
        return;
    }

    DHAnimState resolved_state;
    DHDirection resolved_dir;
    const DHAnimationClip *clip;
    resolve_active_clip(controller, controller->current_state, controller->current_dir,
                       &resolved_state, &resolved_dir, &clip);

    if (clip == NULL || clip->frame_count <= 0) {
        return;
    }

    float speed = controller->speed > 0.0f ? controller->speed : 1.0f;
    controller->timer += delta_time * speed;

    if (controller->current_frame >= clip->frame_count) {
        controller->current_frame = 0;
    }

    float frame_dur = clip->frames[controller->current_frame].duration;
    if (frame_dur <= 0.0001f) {
        frame_dur = 0.1f;
    }

    while (controller->timer >= frame_dur) {
        controller->timer -= frame_dur;
        controller->current_frame++;

        if (controller->current_frame >= clip->frame_count) {
            if (clip->looping) {
                controller->current_frame = 0;
            } else {
                controller->current_frame = clip->frame_count - 1;
                controller->playback_state = DH_PLAYBACK_FINISHED;
                controller->timer = 0.0f;
                break;
            }
        }
        frame_dur = clip->frames[controller->current_frame].duration;
        if (frame_dur <= 0.0001f) {
            frame_dur = 0.1f;
        }
    }
}

void dh_animation_reset(DHAnimationController *controller)
{
    if (controller == NULL) {
        return;
    }

    controller->current_frame = 0;
    controller->timer = 0.0f;
    controller->playback_state = DH_PLAYBACK_PLAYING;
}

bool dh_animation_is_finished(const DHAnimationController *controller)
{
    if (controller == NULL) {
        return true;
    }
    return controller->playback_state == DH_PLAYBACK_FINISHED;
}

void dh_animation_set_speed(DHAnimationController *controller, float speed)
{
    if (controller == NULL) {
        return;
    }
    controller->speed = speed > 0.0f ? speed : 0.001f;
}

void dh_animation_set_paused(DHAnimationController *controller, bool paused)
{
    if (controller == NULL) {
        return;
    }

    if (paused) {
        if (controller->playback_state == DH_PLAYBACK_PLAYING) {
            controller->playback_state = DH_PLAYBACK_PAUSED;
        }
    } else {
        if (controller->playback_state == DH_PLAYBACK_PAUSED) {
            controller->playback_state = DH_PLAYBACK_PLAYING;
        }
    }
}

void dh_animation_draw(const DHAnimationController *controller, Vector2 position, Color tint)
{
    if (controller == NULL) {
        return;
    }

    DHAnimState resolved_state;
    DHDirection resolved_dir;
    const DHAnimationClip *clip;
    resolve_active_clip(controller, controller->current_state, controller->current_dir,
                       &resolved_state, &resolved_dir, &clip);

    if (clip == NULL || clip->texture.id == 0 || clip->frame_count <= 0) {
        return;
    }

    int frame_idx = controller->current_frame;
    if (frame_idx < 0 || frame_idx >= clip->frame_count) {
        frame_idx = 0;
    }

    Rectangle src = clip->frames[frame_idx].frame_rect;
    if (controller->current_dir == DH_DIR_SIDE_LEFT && resolved_dir != DH_DIR_SIDE_LEFT) {
        src.width = -src.width;
    }
    DrawTextureRec(clip->texture, src, position, tint);
}

void dh_animation_draw_pro(const DHAnimationController *controller,
                           Rectangle dest_rect,
                           Vector2 origin,
                           float rotation,
                           Color tint)
{
    if (controller == NULL) {
        return;
    }

    DHAnimState resolved_state;
    DHDirection resolved_dir;
    const DHAnimationClip *clip;
    resolve_active_clip(controller, controller->current_state, controller->current_dir,
                       &resolved_state, &resolved_dir, &clip);

    if (clip == NULL || clip->texture.id == 0 || clip->frame_count <= 0) {
        return;
    }

    int frame_idx = controller->current_frame;
    if (frame_idx < 0 || frame_idx >= clip->frame_count) {
        frame_idx = 0;
    }

    Rectangle src = clip->frames[frame_idx].frame_rect;
    if (controller->current_dir == DH_DIR_SIDE_LEFT && resolved_dir != DH_DIR_SIDE_LEFT) {
        src.width = -src.width;
    }
    DrawTexturePro(clip->texture, src, dest_rect, origin, rotation, tint);
}

const char *dh_anim_state_to_string(DHAnimState state)
{
    switch (state) {
    case DH_ANIM_STATE_IDLE:   return "IDLE";
    case DH_ANIM_STATE_WALK:   return "WALK";
    case DH_ANIM_STATE_RUN:    return "RUN";
    case DH_ANIM_STATE_ATTACK: return "ATTACK";
    case DH_ANIM_STATE_HURT:   return "HURT";
    case DH_ANIM_STATE_DEATH:  return "DEATH";
    default:                   return "UNKNOWN";
    }
}

const char *dh_direction_to_string(DHDirection dir)
{
    switch (dir) {
    case DH_DIR_DOWN:       return "DOWN";
    case DH_DIR_UP:         return "UP";
    case DH_DIR_LEFT:       return "LEFT";
    case DH_DIR_RIGHT:      return "RIGHT";
    case DH_DIR_SIDE_LEFT:  return "SIDE_LEFT";
    case DH_DIR_SIDE_RIGHT: return "SIDE_RIGHT";
    default:                return "UNKNOWN";
    }
}
