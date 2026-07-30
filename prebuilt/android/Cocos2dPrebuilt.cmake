# Copyright (c) 2026 Shivpratap Chauhan <shivmsit@gmail.com>
# SPDX-License-Identifier: MIT

include(${CMAKE_CURRENT_LIST_DIR}/../Cocos2dPrebuiltCommon.cmake)

set(COCOS2DX_PREBUILT_CONFIG "${CMAKE_BUILD_TYPE}" CACHE STRING "Configuration of the reusable cocos2d-x archive")
if(NOT COCOS2DX_PREBUILT_CONFIG)
    set(COCOS2DX_PREBUILT_CONFIG "Debug" CACHE STRING "Configuration of the reusable cocos2d-x archive" FORCE)
endif()
set(COCOS2DX_PREBUILT_ROOT "${COCOS2DX_ROOT_PATH}/prebuilt/android" CACHE PATH "Location of Android cocos2d-x prebuilts")
set(COCOS2DX_BUILD_PREBUILT_IF_MISSING ON CACHE BOOL "Build the Android cocos2d-x prebuilt automatically when absent")

if(NOT ANDROID_ABI)
    message(FATAL_ERROR "ANDROID_ABI must be set to select a cocos2d-x Android prebuilt")
endif()
set(_cocos2dx_prebuilt_build_dir "${COCOS2DX_PREBUILT_ROOT}/build/${ANDROID_ABI}-${COCOS2DX_PREBUILT_CONFIG}")
set(_cocos2dx_prebuilt_archive "${_cocos2dx_prebuilt_build_dir}/lib/libcocos2d-prebuilt.a")
set(COCOS2DX_PREBUILT_GENERATED_EXTERNAL_INCLUDE_DIRS
    "${_cocos2dx_prebuilt_build_dir}/engine/external/jpeg"
    "${_cocos2dx_prebuilt_build_dir}/engine/external/png"
    "${_cocos2dx_prebuilt_build_dir}/engine/external/tiff/libtiff"
    "${_cocos2dx_prebuilt_build_dir}/engine/external/openssl/stage/include"
    "${_cocos2dx_prebuilt_build_dir}/engine/external/websockets/build"
)

# The combined archive includes OpenSSL. Expose it to applications that use
# OpenSSL APIs directly.
if(NOT TARGET cocos2d::openssl)
    add_library(cocos2d::openssl INTERFACE IMPORTED GLOBAL)
    set_target_properties(cocos2d::openssl PROPERTIES
        INTERFACE_LINK_LIBRARIES "cocos2d_prebuilt;dl"
        INTERFACE_INCLUDE_DIRECTORIES "${_cocos2dx_prebuilt_build_dir}/engine/external/openssl/stage/include"
    )
endif()

if(NOT EXISTS "${_cocos2dx_prebuilt_archive}" AND COCOS2DX_BUILD_PREBUILT_IF_MISSING)
    if(NOT CMAKE_ANDROID_NDK)
        message(FATAL_ERROR "CMAKE_ANDROID_NDK is required to build an Android cocos2d-x prebuilt")
    endif()
    message(STATUS "cocos2d-x Android prebuilt is missing; building it once for ${ANDROID_ABI}/${COCOS2DX_PREBUILT_CONFIG}")
    execute_process(
        COMMAND sh "${COCOS2DX_ROOT_PATH}/prebuilt/build-prebuilt.sh"
            --platform android --arch "${ANDROID_ABI}" --config "${COCOS2DX_PREBUILT_CONFIG}"
            --ndk "${CMAKE_ANDROID_NDK}"
        RESULT_VARIABLE _cocos2dx_prebuilt_result
    )
    if(NOT _cocos2dx_prebuilt_result EQUAL 0)
        message(FATAL_ERROR "Failed to build the Android cocos2d-x prebuilt.")
    endif()
endif()

if(NOT EXISTS "${_cocos2dx_prebuilt_archive}")
    message(FATAL_ERROR "Missing Android cocos2d-x prebuilt: ${_cocos2dx_prebuilt_archive}")
endif()

if(NOT TARGET cocos2d_prebuilt)
    cocos2dx_prebuilt_external_include_dirs(_cocos2dx_prebuilt_external_dirs)
    add_library(cocos2d_prebuilt STATIC IMPORTED GLOBAL)
    set_target_properties(cocos2d_prebuilt PROPERTIES
        IMPORTED_LOCATION "${_cocos2dx_prebuilt_archive}"
        INTERFACE_INCLUDE_DIRECTORIES "${COCOS2DX_ROOT_PATH};${COCOS2DX_ROOT_PATH}/cocos;${COCOS2DX_ROOT_PATH}/extensions;${COCOS2DX_ROOT_PATH}/cocos/platform;${COCOS2DX_ROOT_PATH}/cocos/base;${COCOS2DX_ROOT_PATH}/cocos/editor-support;${COCOS2DX_ROOT_PATH}/cocos/audio/include;${COCOS2DX_ROOT_PATH}/cocos/platform/android;${_cocos2dx_prebuilt_external_dirs}"
        INTERFACE_LINK_OPTIONS "LINKER:--exclude-libs,libcocos2d-prebuilt.a"
    )
    cocos2dx_configure_prebuilt_abi(cocos2d_prebuilt)
endif()

function(use_prebuilt_cocos2dx target)
    target_link_libraries(${target} cocos2d_prebuilt)
    use_cocos2dx_compile_define(${target})
    use_cocos2dx_compile_options(${target})
    use_cocos2dx_libs_depend(${target})
endfunction()
