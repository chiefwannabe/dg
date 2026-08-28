#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "${PROJECT_DIR}"

echo "=== Building Dungeon Hunter Executable ==="
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

APPDIR="${PROJECT_DIR}/build/AppDir"
rm -rf "${APPDIR}"
mkdir -p "${APPDIR}/usr/bin"
mkdir -p "${APPDIR}/usr/share/icons/hicolor/256x256/apps"

echo "=== Copying Executable & Runtime Assets ==="
cp build/dungeon-hunter "${APPDIR}/usr/bin/dungeon-hunter"

# Copy required runtime asset directories for portable resolution
mkdir -p "${APPDIR}/usr/bin/assets"
P1="assets/craftpix-net-436971-free-top-down-roguelike-game-kit-pixel-art"
P2="assets/craftpix-net-788364-free-slime-mobs-pixel-art-top-down-sprite-pack"
P3="assets/craftpix-net-180537-free-swordsman-1-3-level-pixel-top-down-sprite-character"

if [ -d "${PROJECT_DIR}/${P1}" ]; then
    mkdir -p "${APPDIR}/usr/bin/${P1}"
    cp -r "${PROJECT_DIR}/${P1}/5Tiled_files" "${APPDIR}/usr/bin/${P1}/"
fi

if [ -d "${PROJECT_DIR}/${P2}" ]; then
    mkdir -p "${APPDIR}/usr/bin/${P2}/PNG/Slime1"
    mkdir -p "${APPDIR}/usr/bin/${P2}/PNG/Slime2"
    mkdir -p "${APPDIR}/usr/bin/${P2}/PNG/Slime3"
    cp -r "${PROJECT_DIR}/${P2}/PNG/Slime1/With_shadow" "${APPDIR}/usr/bin/${P2}/PNG/Slime1/"
    cp -r "${PROJECT_DIR}/${P2}/PNG/Slime2/With_shadow" "${APPDIR}/usr/bin/${P2}/PNG/Slime2/"
    cp -r "${PROJECT_DIR}/${P2}/PNG/Slime3/With_shadow" "${APPDIR}/usr/bin/${P2}/PNG/Slime3/"
fi

if [ -d "${PROJECT_DIR}/${P3}" ]; then
    mkdir -p "${APPDIR}/usr/bin/${P3}/Tiled_files"
    cp -r "${PROJECT_DIR}/${P3}/Tiled_files/Swordsman1" "${APPDIR}/usr/bin/${P3}/Tiled_files/"
    cp -r "${PROJECT_DIR}/${P3}/Tiled_files/Swordsman3" "${APPDIR}/usr/bin/${P3}/Tiled_files/"
fi

echo "=== Creating AppRun & Desktop Launcher ==="
cat > "${APPDIR}/AppRun" <<'EOF'
#!/bin/bash
HERE="$(dirname "$(readlink -f "${0}")")"
export PATH="${HERE}/usr/bin:${PATH}"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"
exec "${HERE}/usr/bin/dungeon-hunter" "$@"
EOF
chmod +x "${APPDIR}/AppRun"

cat > "${APPDIR}/dungeon-hunter.desktop" <<'EOF'
[Desktop Entry]
Type=Application
Name=Dungeon Hunter
Exec=dungeon-hunter
Icon=dungeon-hunter
Categories=Game;
EOF

python3 -c "from PIL import Image, ImageDraw; img = Image.new('RGB', (256, 256), color = (16, 16, 22)); d = ImageDraw.Draw(img); d.text((20,120), 'DH', fill=(255,255,255)); img.save('${APPDIR}/dungeon-hunter.png'); img.save('${APPDIR}/usr/share/icons/hicolor/256x256/apps/dungeon-hunter.png')" || touch "${APPDIR}/dungeon-hunter.png"
cp "${APPDIR}/dungeon-hunter.png" "${APPDIR}/.DirIcon" 2>/dev/null || true

echo "=== Packaging AppImage ==="
if [ ! -f "${PROJECT_DIR}/build/appimagetool" ]; then
    wget -q https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage -O "${PROJECT_DIR}/build/appimagetool"
    chmod +x "${PROJECT_DIR}/build/appimagetool"
fi

cd "${PROJECT_DIR}/build"
if [ -x "./appimagetool" ]; then
    ./appimagetool --appimage-extract >/dev/null 2>&1 || true
    if [ -d "squashfs-root" ]; then
        ./squashfs-root/AppRun "${APPDIR}" "${PROJECT_DIR}/Dungeon-Hunter-x86_64.AppImage"
    else
        ./appimagetool "${APPDIR}" "${PROJECT_DIR}/Dungeon-Hunter-x86_64.AppImage"
    fi
fi

echo "AppImage packaging complete: ${PROJECT_DIR}/Dungeon-Hunter-x86_64.AppImage"
