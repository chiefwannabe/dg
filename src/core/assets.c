#include "dh/assets.h"
#include "dh/logging.h"
#include <string.h>

bool dh_assets_init(DHAssetManager *assets)
{
    if (assets == NULL) {
        return false;
    }

    memset(assets, 0, sizeof(DHAssetManager));

    dh_log_info("Asset manager initialized");
    return true;
}

Texture2D dh_assets_load_texture(DHAssetManager *assets, const char *filepath)
{
    if (assets == NULL || filepath == NULL || filepath[0] == '\0') {
        Texture2D empty = { 0 };
        return empty;
    }

    for (int i = 0; i < assets->texture_count; i++) {
        if (strcmp(assets->textures[i].key, filepath) == 0) {
            assets->textures[i].ref_count++;
            return assets->textures[i].texture;
        }
    }

    if (assets->texture_count >= DH_MAX_TEXTURES) {
        dh_log_error("Asset manager texture limit reached (%d)", DH_MAX_TEXTURES);
        Texture2D empty = { 0 };
        return empty;
    }

    Texture2D tex = LoadTexture(filepath);
    if (tex.id == 0) {
        dh_log_error("Failed to load texture from path: %s", filepath);
        return tex;
    }

    SetTextureFilter(tex, TEXTURE_FILTER_POINT);

    int idx = assets->texture_count;
    strncpy(assets->textures[idx].key, filepath, sizeof(assets->textures[idx].key) - 1);
    assets->textures[idx].key[sizeof(assets->textures[idx].key) - 1] = '\0';
    assets->textures[idx].texture = tex;
    assets->textures[idx].ref_count = 1;
    assets->texture_count++;

    dh_log_info("Loaded sprite sheet texture: %s (%dx%d)", filepath, tex.width, tex.height);
    return tex;
}

void dh_assets_unload_texture(DHAssetManager *assets, const char *filepath)
{
    if (assets == NULL || filepath == NULL) {
        return;
    }

    for (int i = 0; i < assets->texture_count; i++) {
        if (strcmp(assets->textures[i].key, filepath) == 0) {
            assets->textures[i].ref_count--;
            if (assets->textures[i].ref_count <= 0) {
                UnloadTexture(assets->textures[i].texture);
                dh_log_info("Unloaded texture: %s", filepath);
                for (int j = i; j < assets->texture_count - 1; j++) {
                    assets->textures[j] = assets->textures[j + 1];
                }
                assets->texture_count--;
            }
            return;
        }
    }
}

void dh_assets_shutdown(DHAssetManager *assets)
{
    if (assets == NULL) {
        return;
    }

    int unloaded = 0;
    for (int i = 0; i < assets->texture_count; i++) {
        if (assets->textures[i].texture.id != 0) {
            UnloadTexture(assets->textures[i].texture);
            unloaded++;
        }
    }

    dh_log_info("Asset manager shutdown: unloaded %d textures", unloaded);
    assets->texture_count = 0;
}
