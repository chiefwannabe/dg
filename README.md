# Dungeon Hunter — Engine Foundation

A lightweight, professional, cross-platform C11 + raylib 6.x foundation designed to target **Linux Desktop**, **Web (WebAssembly/WebGL)**, and **Android (APK)** from a single unified codebase.

> [!IMPORTANT]
> **ENGINE FOUNDATION ONLY:** This repository contains the core game engine architecture:
> 1. Lifecycle state machine & time management
> 2. **Pixel-Perfect Rendering & Virtual Resolution Scaling (320x180 base)**
> 3. **2D Camera System (Smooth Follow, Boundary Clamping, Subpixel-Snapping)**
> 4. **Coordinate Space Abstraction (Screen <-> Virtual <-> World)**
> 5. **World & Tilemap Foundation with Frustum Culling**
> 
> **No gameplay (hero, monsters, combat, dungeons, items, progression) has been implemented yet.**

---

## 🏛️ Architecture

```text
DG/
├── include/dh/             # Public C headers
│   ├── config.h            # Central configuration & virtual resolution settings
│   ├── coords.h            # 3-tier coordinate system conversions
│   ├── camera.h            # 2D camera system (target tracking, bounds clamping)
│   ├── tilemap.h           # Tilemap structure & frustum-culled rendering
│   ├── world.h             # World bounds & tilemap container
│   ├── logging.h           # Cross-platform logging abstraction
│   ├── time.h              # Clamped frame timing & delta time tracker
│   ├── input.h             # Unified input state (Keyboard, Mouse, Gamepad, Touch Drag)
│   ├── renderer.h          # Virtual base canvas (320x180) & nearest-neighbor filter
│   ├── game.h              # Central game lifecycle & state machine
│   ├── assets.h            # Asset manager stubs (Textures, Audio, Fonts)
│   └── platform.h          # Platform identification helpers
│
├── src/                    # Implementation
│   ├── main.c              # Platform entry dispatcher (Desktop / Web / Android)
│   ├── core/
│   │   ├── camera.c        # 2D Camera logic with Lerp and subpixel snapping
│   │   ├── coords.c        # Screen <-> Virtual <-> World coordinate math
│   │   ├── game.c          # Game lifecycle & render dispatch
│   │   ├── input.c         # Device input & click/touch drag panning
│   │   ├── renderer.c      # Render-target scaling, offscreen pass, foundation UI
│   │   ├── time.c          # Delta time calculation & frame counting
│   │   ├── config.c        # Default application settings
│   │   ├── logging.c       # Log routing (stdout/stderr / Android logcat)
│   │   └── assets.c        # Safe asset loading/unloading foundation
│   ├── world/
│   │   ├── tilemap.c       # Tilemap storage, querying & frustum culling
│   │   └── world.c         # World bounds & draw pipeline
│   └── platform/
│       ├── desktop/        # Linux / Desktop backend
│       ├── web/            # Emscripten / WebAssembly backend
│       └── android/        # Android NDK native backend
│
├── assets/                 # Asset directories (preserved via .gitkeep)
│   ├── sprites/
│   ├── tiles/
│   ├── effects/
│   ├── audio/
│   └── fonts/
│
├── android/                # Android Gradle + NDK NativeActivity project
│   ├── app/build.gradle
│   ├── app/src/main/AndroidManifest.xml
│   └── app/src/main/cpp/CMakeLists.txt
│
├── scripts/                # Multi-platform build scripts
│   ├── build-linux.sh      # Native Release build script
│   ├── build-web.sh        # Emscripten WebAssembly compiler script
│   ├── build-android.sh    # Android build launcher script
│   └── serve-web.sh        # Local HTTP server helper for browser testing
│
├── .github/workflows/      # CI/CD Workflows
│   ├── build-check.yml     # Pull-request Linux compilation check
│   ├── appimage.yml        # Linux AppImage packager & artifact uploader
│   ├── web.yml             # WebAssembly HTML5 artifact uploader
│   └── android.yml         # Android release APK builder & uploader
│
├── CMakeLists.txt          # Native & cross-platform CMake build configuration
├── Makefile                # Fast developer shortcut interface
├── .gitignore
└── README.md
```

---

## 🛠️ Tooling & Dependencies

- **C Compiler**: GCC 13.3.0 / Clang (C11 standard enabled)
- **Build Tools**: GNU Make 4.3+, CMake 3.20+
- **Graphics Framework**: raylib 6.x (installed system-wide or sourced locally)
- **Web Compiler**: Emscripten SDK (`emcc`)
- **Mobile SDK**: Android NDK 26+ & Gradle 8+ / Java 17

---

## 🚀 Building & Running

### 1. Linux Desktop

```bash
# Using Makefile
make clean
make
make run

# Or using CMake directly
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/dungeon-hunter

# Or using script
./scripts/build-linux.sh
```

---

### 2. Web (WebAssembly / WebGL)

The web target compiles the identical C game logic into WebAssembly using Emscripten and raylib 6.x.

```bash
# Build WebAssembly output to web-build/
./scripts/build-web.sh

# Serve locally on http://localhost:8080
./scripts/serve-web.sh
```

---

### 3. Android (APK)

```bash
# Build Android release APK
./scripts/build-android.sh
```

---

### 4. Linux AppImage

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

---

## 📐 Pixel-Perfect Rendering & Coordinate Systems

- **Virtual Resolution**: Fixed **320x180** offscreen `RenderTexture2D` with `TEXTURE_FILTER_POINT` nearest-neighbor filtering.
- **Aspect Ratio Letterboxing**: Dynamically fits any window aspect ratio with pillarbox/letterbox dark borders.
- **Subpixel-Snapping Camera**: The 2D camera snaps position coordinates to integer pixels when rendering to eliminate tile gap lines and subpixel shimmer.
- **Coordinate Spaces**:
  - `Screen`: Physical window pixels (e.g. 1920x1080).
  - `Virtual`: Offscreen viewport coordinates (0..320, 0..180).
  - `World`: Global tilemap coordinates (0..1024, 0..1024).
- **Frustum Culling**: Tilemap renderer calculates visible tile range `[min_x..max_x, min_y..max_y]` based on current camera view frustum, rendering only visible tiles.
