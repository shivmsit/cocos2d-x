cmake_minimum_required(VERSION 3.6)

# This is a small, standalone project.  It builds the cocos2d-x engine once
# and packs cocos2d plus every macOS third-party archive into one archive.
project(cocos2d_x_macos_prebuilt)

get_filename_component(COCOS2DX_ROOT_PATH "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(CMAKE_MODULE_PATH "${COCOS2DX_ROOT_PATH}/cmake/Modules/")

include(CocosBuildSet)

# Script bindings are intentionally excluded: desktop C++ games do not need
# them, and excluding them keeps the shared engine build substantially smaller.
set(BUILD_LUA_LIBS OFF CACHE BOOL "Build lua libraries" FORCE)
set(BUILD_JS_LIBS OFF CACHE BOOL "Build js libraries" FORCE)
set(BUILD_EDITOR_COCOSBUILDER ON CACHE BOOL "Build cocosbuilder support" FORCE)
set(BUILD_EDITOR_COCOSTUDIO ON CACHE BOOL "Build cocostudio support" FORCE)
set(BUILD_EDITOR_SPINE ON CACHE BOOL "Build spine support" FORCE)
set(BUILD_EXTENSIONS ON CACHE BOOL "Build extensions" FORCE)

add_subdirectory("${COCOS2DX_ROOT_PATH}/cocos" "${ENGINE_BINARY_PATH}/cocos/core")

set(COCOS2DX_COMBINED_ARCHIVE "${CMAKE_BINARY_DIR}/lib/libcocos2d-prebuilt.a")
add_custom_target(cocos2d_prebuilt ALL
    COMMAND "${CMAKE_COMMAND}"
        -DCOCOS2DX_BUILD_DIR="${CMAKE_BINARY_DIR}"
        -DCOCOS2DX_ROOT_PATH="${COCOS2DX_ROOT_PATH}"
        -DCOCOS2DX_EXTERNAL_PREBUILT_SUBDIR=mac
        -DCOCOS2DX_OUTPUT_ARCHIVE="${COCOS2DX_COMBINED_ARCHIVE}"
        -DCOCOS2DX_ARCHIVER=APPLE_LIBTOOL
        -P "${CMAKE_CURRENT_LIST_DIR}/CreateCombinedArchive.cmake"
    DEPENDS cocos2d
    COMMENT "Packing reusable cocos2d-x macOS archive"
)
