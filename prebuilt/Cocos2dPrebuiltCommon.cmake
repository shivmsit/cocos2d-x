# An imported archive has no CMake dependency graph to propagate the header
# paths of cocos2d-x's bundled third-party libraries. Mirror the *public*
# include roots declared by its external CMake targets. Do not recursively add
# every header directory: RapidJSON contains a Windows-only msinttypes/stdint.h
# that would otherwise shadow the platform's standard <stdint.h>.
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
        "${COCOS2DX_ROOT_PATH}/external/Box2D/include"
        "${COCOS2DX_ROOT_PATH}/external/bullet/include"
        "${COCOS2DX_ROOT_PATH}/external/bullet/include/bullet"
        "${COCOS2DX_ROOT_PATH}/external/chipmunk/include"
        "${COCOS2DX_ROOT_PATH}/external/ConvertUTF"
        "${COCOS2DX_ROOT_PATH}/external/clipper"
        "${COCOS2DX_ROOT_PATH}/external/edtaa3func"
        "${COCOS2DX_ROOT_PATH}/external/flatbuffers"
        "${COCOS2DX_ROOT_PATH}/external/freetype2/include/${cocos2dx_prebuilt_platform}/freetype2"
        "${COCOS2DX_ROOT_PATH}/external/jpeg/include/${cocos2dx_prebuilt_platform}"
        "${COCOS2DX_ROOT_PATH}/external/png/include/${cocos2dx_prebuilt_platform}"
        "${COCOS2DX_ROOT_PATH}/external/tiff/include/${cocos2dx_prebuilt_platform}"
        "${COCOS2DX_ROOT_PATH}/external/uv/include"
        "${COCOS2DX_ROOT_PATH}/external/webp/include/${cocos2dx_prebuilt_platform}"
        "${COCOS2DX_ROOT_PATH}/external/websockets/include/${cocos2dx_prebuilt_platform}"
        "${COCOS2DX_ROOT_PATH}/external/tinyxml2"
        "${COCOS2DX_ROOT_PATH}/external/unzip"
        "${COCOS2DX_ROOT_PATH}/external/xxhash"
        "${COCOS2DX_ROOT_PATH}/external/xxtea"
        "${COCOS2DX_ROOT_PATH}/external/zlib/include"
    )

    if(MACOSX)
        list(APPEND cocos2dx_prebuilt_external_dirs
            "${COCOS2DX_ROOT_PATH}/external/curl/include/mac"
            "${COCOS2DX_ROOT_PATH}/external/glfw3/include/mac"
            "${COCOS2DX_ROOT_PATH}/external/openssl/include/mac"
        )
    endif()

    set(${out_var} "${cocos2dx_prebuilt_external_dirs}" PARENT_SCOPE)
endfunction()
