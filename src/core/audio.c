#include "dh/audio.h"
#include "dh/logging.h"
#include <raylib.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static Sound s_sounds[DH_SFX_COUNT];
static bool s_audio_initialized = false;

static Sound generate_sfx_wave(int type)
{
    int sample_rate = 22050;
    float duration = 0.15f;
    if (type == DH_SFX_LEVELUP || type == DH_SFX_BOSS) duration = 0.40f;

    int sample_count = (int)((float)sample_rate * duration);
    short *samples = (short *)calloc(sample_count, sizeof(short));
    if (samples == NULL) return (Sound){ 0 };

    for (int i = 0; i < sample_count; i++) {
        float t = (float)i / (float)sample_rate;
        float freq = 440.0f;
        float env = 1.0f - (t / duration);

        if (type == DH_SFX_ATTACK) {
            freq = 600.0f - t * 2500.0f;
        } else if (type == DH_SFX_HIT) {
            freq = 150.0f + (float)(rand() % 80);
        } else if (type == DH_SFX_DEATH) {
            freq = 300.0f - t * 600.0f;
        } else if (type == DH_SFX_PICKUP) {
            freq = 880.0f + sinf(t * 50.0f) * 200.0f;
        } else if (type == DH_SFX_LEVELUP) {
            freq = 440.0f + floorf(t * 10.0f) * 110.0f;
        } else if (type == DH_SFX_BOSS) {
            freq = 90.0f + sinf(t * 30.0f) * 40.0f;
        }

        if (freq < 40.0f) freq = 40.0f;

        float val = sinf(2.0f * 3.14159f * freq * t) * env;
        samples[i] = (short)(val * 12000.0f);
    }

    Wave wave = {
        .frameCount = sample_count,
        .sampleRate = sample_rate,
        .sampleSize = 16,
        .channels = 1,
        .data = samples
    };

    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return sound;
}

bool dh_audio_init(DHAudioSystem *audio)
{
    if (audio == NULL) return false;

    audio->enabled = true;
    audio->master_volume = 0.8f;
    audio->music_volume = 0.7f;
    audio->sfx_volume = 0.8f;

    InitAudioDevice();
    if (!IsAudioDeviceReady()) {
        dh_log_warn("Audio device not ready. SFX disabled.");
        audio->enabled = false;
        return false;
    }

    SetMasterVolume(audio->master_volume);

    for (int i = 0; i < DH_SFX_COUNT; i++) {
        s_sounds[i] = generate_sfx_wave(i);
    }

    s_audio_initialized = true;
    dh_log_info("Procedural audio system initialized (%d SFX channels).", DH_SFX_COUNT);
    return true;
}

void dh_audio_play_sfx(DHAudioSystem *audio, DHSfxType sfx)
{
    if (audio == NULL || !audio->enabled || !s_audio_initialized) return;
    if (sfx < 0 || sfx >= DH_SFX_COUNT) return;

    if (s_sounds[sfx].stream.buffer != NULL) {
        SetSoundVolume(s_sounds[sfx], audio->sfx_volume);
        PlaySound(s_sounds[sfx]);
    }
}

void dh_audio_shutdown(DHAudioSystem *audio)
{
    if (audio == NULL || !s_audio_initialized) return;

    for (int i = 0; i < DH_SFX_COUNT; i++) {
        if (s_sounds[i].stream.buffer != NULL) {
            UnloadSound(s_sounds[i]);
        }
    }

    CloseAudioDevice();
    s_audio_initialized = false;
    audio->enabled = false;
    dh_log_info("Audio system shut down cleanly.");
}
