# Copyright (c) 2026 Shivpratap Chauhan <shivmsit@gmail.com>
# SPDX-License-Identifier: MIT

# Reusable cocos2d-x libraries for application projects.
#
# Applications should keep using:
#   add_subdirectory(${COCOS2DX_ROOT_PATH}/prebuilt)
#   use_prebuilt_cocos2dx(${APP_NAME})

if(MACOSX)
    include(${CMAKE_CURRENT_LIST_DIR}/macosx/Cocos2dPrebuilt.cmake)
elseif(ANDROID)
    include(${CMAKE_CURRENT_LIST_DIR}/android/Cocos2dPrebuilt.cmake)
else()
    message(FATAL_ERROR "The reusable prebuilt setup currently supports macOS and Android only.")
endif()
