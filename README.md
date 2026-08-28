# Dungeon Hunter v1.0.0 — Endless 2D Side-Scrolling Dungeon Game

A lightweight, professional, cross-platform 2D side-scrolling action platformer built in C11 and raylib 6.x. Targets **Linux Desktop**, **Linux AppImage**, and **Web (WebAssembly/WebGL)** from a single unified codebase.

---

## 🎮 Game Design & Core Loop

**Dungeon Hunter** is an endless horizontal side-scrolling dungeon exploration game.

```text
EXPLORE → FIGHT MONSTERS → COLLECT GOLD & XP → LEVEL UP → BUY EQUIPMENT → DEFEAT BOSSES → CONTINUOUS ENDLESS WORLD
```

### Controls
- **A**: Move Left / Backward
- **D**: Move Right / Forward
- **SPACE**: Sword Attack
- **TAB**: Open / Close Inventory & Equipment UI
- **E**: Interact with Merchant / Open Shop (when near Shop Merchant)
- **1 – 9**: Select Inventory / Shop Item Slot
- **ENTER**: Equip / Unequip Selected Item OR Buy Selected Shop Item
- **P / ESC**: Pause Game (Closes popups first)

*(Note: There is NO jumping and NO vertical movement. The hero explores horizontally along the dungeon plane.)*

---

## ⚔️ Game Features

1. **Swordsman Hero & Combat**:
   - 64x64 side-facing sprite animations (`Idle`, `Run`, `Attack`, `Hurt`, `Death`).
   - Active sword attack hit window with single-hit registry and knockback impulse.
2. **Endless Dungeon Environment**:
   - 7-chunk active windowing with bidirectional chunk recycling.
   - Parallax scrolling background layers and floor snapping physics.
3. **Enemy Variety & Distance Difficulty Scaling**:
   - **Green Slimes**: Fast basic mobs.
   - **Red Orc Slimes**: Armored medium mobs.
   - **Heavy Golem Slimes**: High HP, heavy damage brutes with strong knockback resistance.
   - **Elite Variants**: Rare golden aura mobs with $1.8\times$ HP, $1.5\times$ damage, and $2.5\times$ rewards.
4. **Dungeon Demon King Boss Encounters**:
   - Deterministic milestone encounters (every 3000 world px / Chunk 15).
   - 2-Phase battle logic with telegraph warning icons (`!` and `!!`) and horizontal arena encounter boundaries.
5. **Progression & Leveling**:
   - XP system with multi-threshold level up carryover protection.
   - Leveling increases Max HP, restores current health, and boosts base Attack Damage.
6. **Loot & RPG Equipment System**:
   - Collectible gold coin pickups on floor.
   - 24-slot inventory, Weapon Slot (+DMG), Armor Slot (+HP).
   - Catalogs ranging from *Rusty Sword* to *Demon Blade* and *Cloth Armor* to *Demon Armor*.
7. **World Shop Merchant**:
   - Merchant Stand NPC stationed every 1500 world px for gear purchasing.
8. **Local / Session Persistence**:
   - Automatic binary save serialization (`save.dat`) on purchases, equip toggles, level ups, and boss defeats.
   - Cross-platform file I/O (IndexedDB persistence on WebAssembly).
9. **Procedural Audio Synthesis**:
   - Built-in procedural SFX generator for attacks, hits, deaths, pickups, level ups, and boss rumbles.

---

## 🛠️ Debug Controls

- **F2**: Toggle Animation Test Scene
- **F3**: Toggle Tile Collision Debug Overlay
- **F4**: Toggle Enemy Pool Debug Overlay
- **F5**: Toggle Combat Hitbox Debug Overlay
- **F6**: Toggle Progression & Loot Debug Overlay
- **F7**: Toggle Boss System Debug Overlay
- **F8**: Toggle Inventory & Save Debug Overlay

---

## 🏛️ Architecture & Project Structure

```text
dg/
├── include/dh/             # Public C headers
│   ├── config.h            # Central configuration & virtual resolution settings
│   ├── coords.h            # 3-tier coordinate system conversions
│   ├── camera.h            # 2D camera system (target tracking, bounds clamping)
│   ├── tilemap.h           # Tilemap structure & frustum-culled rendering
│   ├── world.h             # World bounds & tilemap container
│   ├── logging.h           # Cross-platform logging abstraction
│   ├── time.h              # Clamped frame timing & delta time tracker
│   ├── input.h             # Unified input state (A/D controls, pause, mouse)
│   ├── renderer.h          # Virtual base canvas (320x180) & nearest-neighbor filter
│   ├── game.h              # Central game lifecycle & state machine
│   ├── assets.h            # Centralized portable asset-root manager
│   ├── animation.h         # Reusable animation system
│   ├── player.h            # Hero state, movement, and platformer physics
│   ├── enemy.h             # Multi-archetype enemy pool, scaling, and AI
│   ├── combat.h            # Combat resolution & hit registry
│   ├── loot.h              # Fixed pickup pool & gold collection
│   ├── boss.h              # Milestone boss battle state machine & arena bounds
│   ├── inventory.h         # Fixed inventory pool & equipment slots
│   ├── shop.h              # World shop merchant & purchase logic
│   ├── save.h              # Local binary save file serialization
│   └── audio.h             # Procedural audio synthesis & SFX channels
│
├── src/                    # Implementation
│   ├── main.c              # Entry point dispatcher
│   ├── core/               # Engine core modules
│   ├── world/              # Endless world & chunk tilemap system
│   └── platform/           # Platform backends (Desktop, Web, Android)
│
├── scripts/                # Multi-platform build scripts
│   ├── build-web.sh        # Emscripten WebAssembly compiler script
│   ├── serve-web.sh        # Local HTTP server helper for browser testing
│   └── build-appimage.sh   # Self-contained Linux AppImage packager script
│
├── CMakeLists.txt          # Native & cross-platform CMake build configuration
├── Makefile                # Developer shortcut interface
└── README.md
```

---

## 🚀 Building & Running

### 1. Linux Desktop

```bash
make clean
make
./build/dungeon-hunter
```

### 2. Web (WebAssembly / WebGL)

```bash
./scripts/build-web.sh
./scripts/serve-web.sh
# Open http://localhost:8080 in browser
```

### 3. Linux AppImage

```bash
./scripts/build-appimage.sh
./Dungeon-Hunter-x86_64.AppImage
```

---

## 📜 License & Credits

Developed with C11 & raylib 6.x. Artwork assets provided via CraftPix pixel-art sprite packs.
