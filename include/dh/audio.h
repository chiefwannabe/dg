#ifndef DH_AUDIO_H
#define DH_AUDIO_H

#include <stdbool.h>

typedef enum {
    DH_SFX_ATTACK = 0,
    DH_SFX_HIT,
    DH_SFX_DEATH,
    DH_SFX_PICKUP,
    DH_SFX_LEVELUP,
    DH_SFX_BOSS,
    DH_SFX_COUNT
} DHSfxType;

typedef struct {
    bool enabled;
    float master_volume;
    float music_volume;
    float sfx_volume;
} DHAudioSystem;

bool dh_audio_init(DHAudioSystem *audio);
void dh_audio_play_sfx(DHAudioSystem *audio, DHSfxType sfx);
void dh_audio_shutdown(DHAudioSystem *audio);

#endif /* DH_AUDIO_H */
