# An imported archive has no CMake dependency graph to propagate the header
# paths of cocos2d-x's bundled third-party libraries. Mirror the *public*
# include roots declared by its external CMake targets. Do not recursively add
# every header directory: RapidJSON contains a Windows-only msinttypes/stdint.h
# that would otherwise shadow the platform's standard <stdint.h>.
function(cocos2dx_configure_prebuilt_abi target)
    if(NOT TARGET "${target}")
        message(FATAL_ERROR "Cannot configure ABI for missing target '${target}'")
    endif()

    # Chipmunk passes cpFloat and cpVect values across its public C API. These
    # definitions must match the ones used to compile the combined archive on
    # every platform, or callers and the prebuilt library disagree on the ABI.
    set_property(TARGET "${target}" APPEND PROPERTY
        INTERFACE_COMPILE_DEFINITIONS
        CP_USE_CGTYPES=0
        CP_USE_DOUBLES=0
    )
endfunction()

function(cocos2dx_prebuilt_external_include_dirs out_var)
    if(MACOSX)
        set(cocos2dx_prebuilt_platform mac)
    elseif(ANDROID)
        set(cocos2dx_prebuilt_platform android)
    else()
        message(FATAL_ERROR "No external-header mapping for this platform")
    endif()

    set(cocos2dx_prebuilt_external_dirs
        "${COCOS2DX_ROOT_PATH}/external"
        "${COCOS2DX_ROOT_PATH}/external/include"
        "${COCOS2DX_ROOT_PATH}/external/Box2D/include"
        "${COCOS2DX_ROOT_PATH}/external/bullet/src"
        "${COCOS2DX_ROOT_PATH}/external/chipmunk/include"
        "${COCOS2DX_ROOT_PATH}/external/ConvertUTF"
        "${COCOS2DX_ROOT_PATH}/external/clipper/include"
        "${COCOS2DX_ROOT_PATH}/external/edtaa3func"
        "${COCOS2DX_ROOT_PATH}/external/flatbuffers"
        "${COCOS2DX_ROOT_PATH}/external/json/include"
        "${COCOS2DX_ROOT_PATH}/external/freetype/include"
        "${COCOS2DX_ROOT_PATH}/external/jpeg"
        "${COCOS2DX_ROOT_PATH}/external/png"
        "${COCOS2DX_ROOT_PATH}/external/recast/DebugUtils/Include"
        "${COCOS2DX_ROOT_PATH}/external/recast/Detour/Include"
        "${COCOS2DX_ROOT_PATH}/external/recast/DetourCrowd/Include"
        "${COCOS2DX_ROOT_PATH}/external/recast/DetourTileCache/Include"
        "${COCOS2DX_ROOT_PATH}/external/recast/Recast/Include"
        "${COCOS2DX_ROOT_PATH}/external/fastlz"
        "${COCOS2DX_ROOT_PATH}/external/tiff/libtiff"
        "${COCOS2DX_ROOT_PATH}/external/curl/include"
        "${COCOS2DX_ROOT_PATH}/external/uv/include"
        # Cocos 3.17 includes WebP headers as <decode.h>; its source-built
        # upstream headers live beneath src/webp.
        "${COCOS2DX_ROOT_PATH}/external/webp/src/webp"
        "${COCOS2DX_ROOT_PATH}/external/websockets/include"
        "${COCOS2DX_ROOT_PATH}/external/tinyxml2"
        "${COCOS2DX_ROOT_PATH}/external/unzip"
        "${COCOS2DX_ROOT_PATH}/external/xxhash"
        "${COCOS2DX_ROOT_PATH}/external/xxtea"
        "${COCOS2DX_ROOT_PATH}/external/zlib"
    )

    # libpng generates pnglibconf.h in the source-build directory.  A
    # prebuilt consumer still includes png.h directly, so expose that matching
    # generated header alongside the vendored public headers.
    if(DEFINED COCOS2DX_PREBUILT_GENERATED_EXTERNAL_INCLUDE_DIRS)
        list(APPEND cocos2dx_prebuilt_external_dirs
            ${COCOS2DX_PREBUILT_GENERATED_EXTERNAL_INCLUDE_DIRS}
        )
    endif()

    if(MACOSX)
        list(APPEND cocos2dx_prebuilt_external_dirs
            # GLFW is source-built.  Keep Cocos 3.17's historical flat
            # <glfw3.h> include style by exporting GLFW's header directory.
            "${COCOS2DX_ROOT_PATH}/external/glfw/include/GLFW"
        )
    endif()

    set(${out_var} "${cocos2dx_prebuilt_external_dirs}" PARENT_SCOPE)
endfunction()
