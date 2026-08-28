#!/usr/bin/env bash
set -euo pipefail

# Determine project root directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "${PROJECT_DIR}"

EMSDK_DIR="${PROJECT_DIR}/emsdk"
if [ ! -f "${EMSDK_DIR}/emsdk_env.sh" ]; then
    echo "Error: emsdk_env.sh not found at ${EMSDK_DIR}/emsdk_env.sh" >&2
    exit 1
fi

export EMSDK_QUIET=1
source "${EMSDK_DIR}/emsdk_env.sh"

WEB_BUILD_DIR="${PROJECT_DIR}/web-build"
rm -rf "${WEB_BUILD_DIR}"
mkdir -p "${WEB_BUILD_DIR}"

RAYLIB_DIR="${HOME}/raylib"
RAYLIB_WEB_LIB=""
if [ -f "${RAYLIB_DIR}/src/libraylib.web.a" ]; then
    RAYLIB_WEB_LIB="${RAYLIB_DIR}/src/libraylib.web.a"
elif [ -f "${RAYLIB_DIR}/build/raylib/libraylib.a" ]; then
    RAYLIB_WEB_LIB="${RAYLIB_DIR}/build/raylib/libraylib.a"
fi

RAYLIB_INC="${RAYLIB_DIR}/src"
if [ ! -d "${RAYLIB_INC}" ]; then
    RAYLIB_INC="/usr/local/include"
fi

SHELL_ARG=""
if [ -f "${PROJECT_DIR}/web/shell.html" ]; then
    SHELL_ARG="--shell-file ${PROJECT_DIR}/web/shell.html"
elif [ -f "${RAYLIB_DIR}/src/minshell.html" ]; then
    SHELL_ARG="--shell-file ${RAYLIB_DIR}/src/minshell.html"
fi

PRELOAD_ARG=""
if [ -d "${PROJECT_DIR}/assets" ]; then
    PRELOAD_ARG="--preload-file assets@assets"
fi

echo "Building WebAssembly target with emcc..."

emcc \
    src/main.c \
    src/core/config.c \
    src/core/logging.c \
    src/core/time.c \
    src/core/coords.c \
    src/core/camera.c \
    src/core/input.c \
    src/core/renderer.c \
    src/core/game.c \
    src/core/assets.c \
    src/core/animation.c \
    src/core/anim_demo.c \
    src/core/player.c \
    src/world/tilemap.c \
    src/world/world.c \
    src/platform/web/platform_web.c \
    -Iinclude \
    -I"${RAYLIB_INC}" \
    ${RAYLIB_WEB_LIB:+"${RAYLIB_WEB_LIB}"} \
    -s USE_GLFW=3 \
    -s WASM=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s FORCE_FILESYSTEM=1 \
    ${PRELOAD_ARG} \
    ${SHELL_ARG} \
    -o "${WEB_BUILD_DIR}/index.html" \
    -O1

echo "WebAssembly build complete: ${WEB_BUILD_DIR}"
