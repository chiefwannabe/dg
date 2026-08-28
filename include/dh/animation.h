#ifndef DH_ANIMATION_H
#define DH_ANIMATION_H

#include <raylib.h>
#include <stdbool.h>

#define DH_MAX_ANIM_FRAMES 16

typedef enum {
    DH_ANIM_STATE_IDLE = 0,
    DH_ANIM_STATE_WALK,
    DH_ANIM_STATE_RUN,
    DH_ANIM_STATE_ATTACK,
    DH_ANIM_STATE_HURT,
    DH_ANIM_STATE_DEATH,
    DH_ANIM_STATE_COUNT
} DHAnimState;

typedef enum {
    DH_DIR_DOWN = 0,
    DH_DIR_UP,
    DH_DIR_LEFT,
    DH_DIR_RIGHT,
    DH_DIR_COUNT
} DHDirection;

typedef enum {
    DH_PLAYBACK_PLAYING = 0,
    DH_PLAYBACK_PAUSED,
    DH_PLAYBACK_FINISHED
} DHPlaybackState;

typedef struct {
    Rectangle frame_rect;
    float duration;
} DHAnimationFrame;

typedef struct {
    char name[32];
    DHAnimationFrame frames[DH_MAX_ANIM_FRAMES];
    int frame_count;
    Texture2D texture;
    bool looping;
} DHAnimationClip;

typedef struct {
    DHAnimationClip clips[DH_ANIM_STATE_COUNT][DH_DIR_COUNT];
    bool clip_valid[DH_ANIM_STATE_COUNT][DH_DIR_COUNT];
    
    DHAnimState current_state;
    DHDirection current_dir;
    int current_frame;
    float timer;
    float speed;
    DHPlaybackState playback_state;
} DHAnimationController;

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
                                   bool horizontal);

void dh_animation_controller_init(DHAnimationController *controller);
void dh_animation_controller_add_clip(DHAnimationController *controller,
                                      DHAnimState state,
                                      DHDirection dir,
                                      const DHAnimationClip *clip);

bool dh_animation_play(DHAnimationController *controller,
                        DHAnimState state,
                        DHDirection dir,
                        bool restart_if_same);

void dh_animation_update(DHAnimationController *controller, float delta_time);
void dh_animation_reset(DHAnimationController *controller);
bool dh_animation_is_finished(const DHAnimationController *controller);
void dh_animation_set_speed(DHAnimationController *controller, float speed);
void dh_animation_set_paused(DHAnimationController *controller, bool paused);

void dh_animation_draw(const DHAnimationController *controller, Vector2 position, Color tint);
void dh_animation_draw_pro(const DHAnimationController *controller,
                           Rectangle dest_rect,
                           Vector2 origin,
                           float rotation,
                           Color tint);

const char *dh_anim_state_to_string(DHAnimState state);
const char *dh_direction_to_string(DHDirection dir);

#endif /* DH_ANIMATION_H */
