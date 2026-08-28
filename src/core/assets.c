#include "dh/assets.h"
#include "dh/logging.h"
#include <raylib.h>
#include <stdio.h>
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

const char *dh_assets_get_resolved_path(const char *filepath, char *out_buf, size_t out_size)
{
    if (filepath == NULL || filepath[0] == '\0' || out_buf == NULL || out_size == 0) {
        if (out_buf && out_size > 0) out_buf[0] = '\0';
        return "";
    }

    /* 1. Direct path check (Current Working Directory or Absolute Path or Web Virtual FS) */
    if (filepath[0] == '/' || FileExists(filepath)) {
        snprintf(out_buf, out_size, "%s", filepath);
        return out_buf;
    }

    /* 2. Executable Directory relative resolution (GetApplicationDirectory()) */
    const char *app_dir = GetApplicationDirectory();
    if (app_dir != NULL && app_dir[0] != '\0') {
        snprintf(out_buf, out_size, "%s%s", app_dir, filepath);
        if (FileExists(out_buf)) {
            return out_buf;
        }

        /* 3. Check AppDir parent directory (e.g. app_dir/../assets/...) */
        snprintf(out_buf, out_size, "%s../%s", app_dir, filepath);
        if (FileExists(out_buf)) {
            return out_buf;
        }
    }

    /* Fallback to original filepath string */
    snprintf(out_buf, out_size, "%s", filepath);
    return out_buf;
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

    char resolved[512];
    dh_assets_get_resolved_path(filepath, resolved, sizeof(resolved));

    Texture2D tex = LoadTexture(resolved);
    if (tex.id == 0) {
        dh_log_error("Failed to load texture from path: %s (resolved: %s)", filepath, resolved);
        return tex;
    }

    SetTextureFilter(tex, TEXTURE_FILTER_POINT);

    int idx = assets->texture_count;
    snprintf(assets->textures[idx].key, sizeof(assets->textures[idx].key), "%s", filepath);
    assets->textures[idx].texture = tex;
    assets->textures[idx].ref_count = 1;
    assets->texture_count++;

    dh_log_info("Loaded sprite sheet texture: %s [resolved: %s] (%dx%d)", filepath, resolved, tex.width, tex.height);
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
