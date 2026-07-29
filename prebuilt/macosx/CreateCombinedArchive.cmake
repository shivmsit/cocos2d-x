if(NOT DEFINED COCOS2DX_BUILD_DIR OR NOT DEFINED COCOS2DX_OUTPUT_ARCHIVE)
    message(FATAL_ERROR "COCOS2DX_BUILD_DIR and COCOS2DX_OUTPUT_ARCHIVE are required")
endif()

# All cocos2d-x third-party dependencies are static archives.  Apple's libtool
# extracts their object files into one archive, so consuming games need only one
# library and cannot suffer from static-library link-order problems.
file(GLOB_RECURSE cocos2dx_archives "${COCOS2DX_BUILD_DIR}/*.a")
list(FILTER cocos2dx_archives EXCLUDE REGEX "/libcocos2d-prebuilt\\.a$")

# Several cocos2d-x external targets are IMPORTED
# targets whose archives live in external/*/prebuilt rather than the CMake build
# directory. Include them in the combined archive as well. GLFW, zlib,
# FreeType, JPEG, libpng and Bullet are deliberately excluded here: they are
# built from source and found by the build-directory glob above.
if(DEFINED COCOS2DX_ROOT_PATH AND DEFINED COCOS2DX_EXTERNAL_PREBUILT_SUBDIR)
    if(COCOS2DX_EXTERNAL_PREBUILT_SUBDIR STREQUAL "mac")
        # Keep this aligned with external/CMakeLists.txt. In particular, do
        # not add the optional Lua/JavaScript archives: they are disabled for
        # this C++ prebuilt and make the archive unnecessarily huge.
        set(cocos2dx_imported_archives)
    else()
        file(GLOB_RECURSE cocos2dx_imported_archives
            "${COCOS2DX_ROOT_PATH}/external/*/prebuilt/${COCOS2DX_EXTERNAL_PREBUILT_SUBDIR}/*.a"
        )
    endif()
    list(APPEND cocos2dx_archives ${cocos2dx_imported_archives})
endif()
list(REMOVE_DUPLICATES cocos2dx_archives)
if(NOT cocos2dx_archives)
    message(FATAL_ERROR "No static archives were produced in ${COCOS2DX_BUILD_DIR}")
endif()

file(REMOVE "${COCOS2DX_OUTPUT_ARCHIVE}")
if(COCOS2DX_ARCHIVER STREQUAL "APPLE_LIBTOOL")
    execute_process(
        COMMAND /usr/bin/libtool -static -o "${COCOS2DX_OUTPUT_ARCHIVE}" ${cocos2dx_archives}
        RESULT_VARIABLE libtool_result
    )
else()
    file(WRITE "${COCOS2DX_BUILD_DIR}/combine-archives.mri" "CREATE ${COCOS2DX_OUTPUT_ARCHIVE}\n")
    foreach(cocos2dx_archive ${cocos2dx_archives})
        file(APPEND "${COCOS2DX_BUILD_DIR}/combine-archives.mri" "ADDLIB ${cocos2dx_archive}\n")
    endforeach()
    file(APPEND "${COCOS2DX_BUILD_DIR}/combine-archives.mri" "SAVE\nEND\n")
    execute_process(
        COMMAND "${COCOS2DX_AR}" -M
        INPUT_FILE "${COCOS2DX_BUILD_DIR}/combine-archives.mri"
        RESULT_VARIABLE libtool_result
    )
endif()
if(NOT libtool_result EQUAL 0)
    message(FATAL_ERROR "Unable to create ${COCOS2DX_OUTPUT_ARCHIVE}")
endif()
