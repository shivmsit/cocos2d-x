# Copyright (c) 2026 Shivpratap Chauhan <shivmsit@gmail.com>
# SPDX-License-Identifier: MIT

# Imported target and first-build bootstrap for macOS application projects.

include(${CMAKE_CURRENT_LIST_DIR}/../Cocos2dPrebuiltCommon.cmake)

function(_cocos2dx_macos_arch out_var)
    if(CMAKE_OSX_ARCHITECTURES)
        list(GET CMAKE_OSX_ARCHITECTURES 0 arch)
    else()
        execute_process(COMMAND uname -m OUTPUT_VARIABLE arch OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()
    set(${out_var} "${arch}" PARENT_SCOPE)
endfunction()

_cocos2dx_macos_arch(COCOS2DX_PREBUILT_ARCH)
set(COCOS2DX_PREBUILT_CONFIG "${CMAKE_BUILD_TYPE}" CACHE STRING "Configuration of the reusable cocos2d-x archive")
if(NOT COCOS2DX_PREBUILT_CONFIG)
    set(COCOS2DX_PREBUILT_CONFIG "Debug" CACHE STRING "Configuration of the reusable cocos2d-x archive" FORCE)
endif()
set(COCOS2DX_PREBUILT_ROOT "${COCOS2DX_ROOT_PATH}/prebuilt/macosx" CACHE PATH "Location of macOS cocos2d-x prebuilts")
set(COCOS2DX_BUILD_PREBUILT_IF_MISSING ON CACHE BOOL "Build the macOS cocos2d-x prebuilt automatically when absent")

set(_cocos2dx_prebuilt_build_dir "${COCOS2DX_PREBUILT_ROOT}/build/${COCOS2DX_PREBUILT_ARCH}-${COCOS2DX_PREBUILT_CONFIG}")
set(_cocos2dx_prebuilt_archive "${_cocos2dx_prebuilt_build_dir}/lib/libcocos2d-prebuilt.a")
set(COCOS2DX_PREBUILT_GENERATED_EXTERNAL_INCLUDE_DIRS
    "${_cocos2dx_prebuilt_build_dir}/engine/external/jpeg"
    "${_cocos2dx_prebuilt_build_dir}/engine/external/png"
    "${_cocos2dx_prebuilt_build_dir}/engine/external/tiff/libtiff"
    "${_cocos2dx_prebuilt_build_dir}/engine/external/openssl/stage/include"
    "${_cocos2dx_prebuilt_build_dir}/engine/external/websockets/build"
)

if(NOT EXISTS "${_cocos2dx_prebuilt_archive}" AND COCOS2DX_BUILD_PREBUILT_IF_MISSING)
    message(STATUS "cocos2d-x macOS prebuilt is missing; building it once for ${COCOS2DX_PREBUILT_ARCH}/${COCOS2DX_PREBUILT_CONFIG}")
    execute_process(
        COMMAND sh "${COCOS2DX_ROOT_PATH}/prebuilt/build-prebuilt.sh"
            --platform macosx
            --arch "${COCOS2DX_PREBUILT_ARCH}"
            --config "${COCOS2DX_PREBUILT_CONFIG}"
        RESULT_VARIABLE _cocos2dx_prebuilt_result
    )
    if(NOT _cocos2dx_prebuilt_result EQUAL 0)
        message(FATAL_ERROR "Failed to build the cocos2d-x prebuilt. Run ${COCOS2DX_ROOT_PATH}/prebuilt/build-prebuilt.sh manually for details.")
    endif()
endif()

# The combined archive includes OpenSSL. This target gives applications a
# supported way to use its headers and symbols without host dependencies.
if(NOT TARGET cocos2d::openssl)
    add_library(cocos2d::openssl INTERFACE IMPORTED GLOBAL)
    set_target_properties(cocos2d::openssl PROPERTIES
        INTERFACE_LINK_LIBRARIES cocos2d_prebuilt
        INTERFACE_INCLUDE_DIRECTORIES "${_cocos2dx_prebuilt_build_dir}/engine/external/openssl/stage/include"
    )
endif()

if(NOT EXISTS "${_cocos2dx_prebuilt_archive}")
    message(FATAL_ERROR
        "Missing cocos2d-x prebuilt: ${_cocos2dx_prebuilt_archive}. "
        "Run ${COCOS2DX_ROOT_PATH}/prebuilt/build-prebuilt.sh --platform macosx --arch ${COCOS2DX_PREBUILT_ARCH} --config ${COCOS2DX_PREBUILT_CONFIG}, "
        "or enable COCOS2DX_BUILD_PREBUILT_IF_MISSING.")
endif()

if(NOT TARGET cocos2d_prebuilt)
    cocos2dx_prebuilt_external_include_dirs(_cocos2dx_prebuilt_external_dirs)
    add_library(cocos2d_prebuilt STATIC IMPORTED GLOBAL)
    set_target_properties(cocos2d_prebuilt PROPERTIES
        IMPORTED_LOCATION "${_cocos2dx_prebuilt_archive}"
        INTERFACE_INCLUDE_DIRECTORIES "${COCOS2DX_ROOT_PATH};${COCOS2DX_ROOT_PATH}/cocos;${COCOS2DX_ROOT_PATH}/extensions;${COCOS2DX_ROOT_PATH}/cocos/platform;${COCOS2DX_ROOT_PATH}/cocos/base;${COCOS2DX_ROOT_PATH}/cocos/editor-support;${COCOS2DX_ROOT_PATH}/cocos/audio/include;${COCOS2DX_ROOT_PATH}/cocos/platform/mac;${_cocos2dx_prebuilt_external_dirs}"
        INTERFACE_LINK_LIBRARIES "-framework SystemConfiguration;-framework CoreFoundation;-framework CoreServices"
    )
    cocos2dx_configure_prebuilt_abi(cocos2d_prebuilt)
endif()

function(use_prebuilt_cocos2dx target)
    if(NOT TARGET "${target}")
        message(FATAL_ERROR "use_prebuilt_cocos2dx: target '${target}' does not exist")
    endif()
    target_link_libraries(${target} cocos2d_prebuilt)
    use_cocos2dx_compile_define(${target})
    use_cocos2dx_compile_options(${target})
    use_cocos2dx_libs_depend(${target})
endfunction()
