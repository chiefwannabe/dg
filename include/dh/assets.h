#ifndef DH_ASSETS_H
#define DH_ASSETS_H

#include <raylib.h>
#include <stdbool.h>

#define DH_MAX_TEXTURES 64

typedef struct {
    char key[128];
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
Texture2D dh_assets_load_texture(DHAssetManager *assets, const char *filepath);
void dh_assets_unload_texture(DHAssetManager *assets, const char *filepath);
void dh_assets_shutdown(DHAssetManager *assets);

#endif /* DH_ASSETS_H */
