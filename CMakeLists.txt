cmake_minimum_required(VERSION 3.20)

project(dungeon_hunter
    VERSION 0.1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(DH_WARNINGS "Enable extra compiler warnings" ON)

if(EMSCRIPTEN)
    set(PLATFORM_SOURCE src/platform/web/platform_web.c)
elseif(ANDROID)
    set(PLATFORM_SOURCE src/platform/android/platform_android.c)
else()
    set(PLATFORM_SOURCE src/platform/desktop/platform_desktop.c)
endif()

set(DH_SOURCES
    src/main.c
    src/core/config.c
    src/core/logging.c
    src/core/time.c
    src/core/coords.c
    src/core/camera.c
    src/core/input.c
    src/core/renderer.c
    src/core/game.c
    src/core/assets.c
    src/core/animation.c
    src/core/anim_demo.c
    src/core/player.c
    src/core/enemy.c
    src/core/combat.c
    src/core/loot.c
    src/core/boss.c
    src/core/inventory.c
    src/core/shop.c
    src/core/save.c
    src/core/audio.c
    src/world/tilemap.c
    src/world/world.c
    ${PLATFORM_SOURCE}
)

add_executable(dungeon-hunter ${DH_SOURCES})

target_include_directories(dungeon-hunter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(NOT EMSCRIPTEN AND NOT ANDROID)
    find_package(raylib CONFIG QUIET)
    if(NOT raylib_FOUND)
        find_package(PkgConfig QUIET)
        if(PKG_CONFIG_FOUND)
            pkg_check_modules(RAYLIB raylib)
        endif()
    endif()

    if(raylib_FOUND)
        target_link_libraries(dungeon-hunter PRIVATE raylib)
    elseif(RAYLIB_FOUND)
        target_include_directories(dungeon-hunter PRIVATE ${RAYLIB_INCLUDE_DIRS})
        target_link_directories(dungeon-hunter PRIVATE ${RAYLIB_LIBRARY_DIRS})
        target_link_libraries(dungeon-hunter PRIVATE ${RAYLIB_LIBRARIES})
        target_compile_options(dungeon-hunter PRIVATE ${RAYLIB_CFLAGS_OTHER})
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../raylib")
        include(FetchContent)
        FetchContent_Declare(
            raylib
            SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../raylib"
        )
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(CUSTOMIZE_BUILD ON CACHE BOOL "" FORCE)
        set(SUPPORT_MODULE_RMODELS OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(raylib)
        target_link_libraries(dungeon-hunter PRIVATE raylib)
    else()
        message(FATAL_ERROR "raylib library not found")
    endif()

    if(UNIX AND NOT APPLE)
        target_link_libraries(dungeon-hunter PRIVATE m)
    endif()
endif()

if(DH_WARNINGS)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(dungeon-hunter PRIVATE
            -Wall -Wextra -Wpedantic
        )
    endif()
endif()

set_target_properties(dungeon-hunter PROPERTIES
    OUTPUT_NAME "dungeon-hunter"
)
