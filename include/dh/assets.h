#ifndef DH_ASSETS_H
#define DH_ASSETS_H

#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>

#define DH_MAX_TEXTURES 64

typedef struct {
    char key[512];
    Texture2D texture;
    int ref_count;
} DHTextureEntry;

typedef struct {
    DHTextureEntry textures[DH_MAX_TEXTURES];
    int texture_count;
    int sound_count;
    int music_count;
    int font_count;
} DHAssetManager;

bool dh_assets_init(DHAssetManager *assets);
const char *dh_assets_get_resolved_path(const char *filepath, char *out_buf, size_t out_size);
Texture2D dh_assets_load_texture(DHAssetManager *assets, const char *filepath);
void dh_assets_unload_texture(DHAssetManager *assets, const char *filepath);
void dh_assets_shutdown(DHAssetManager *assets);

#endif /* DH_ASSETS_H */
