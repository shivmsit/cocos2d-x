# Locate the cocos2d-x checkout used by this game without copying the engine
# into every generated project.

function(_cocos2dx_validate_root candidate out_var)
    if(NOT candidate)
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    get_filename_component(candidate_abs "${candidate}" ABSOLUTE)
    if(EXISTS "${candidate_abs}/cocos/cocos2d.h"
       AND EXISTS "${candidate_abs}/cmake/Modules/CocosBuildSet.cmake"
       AND EXISTS "${candidate_abs}/prebuilt/CMakeLists.txt")
        get_filename_component(candidate_real "${candidate_abs}" REALPATH)
        set(${out_var} "${candidate_real}" PARENT_SCOPE)
    else()
        set(${out_var} FALSE PARENT_SCOPE)
    endif()
endfunction()

function(cocos2dx_resolve_root out_var)
    set(COCOS2DX_ROOT_PATH "" CACHE PATH
        "Path to the cocos2d-x source checkout")

    if(COCOS2DX_ROOT_PATH)
        _cocos2dx_validate_root("${COCOS2DX_ROOT_PATH}" resolved_root)
        if(NOT resolved_root)
            message(FATAL_ERROR
                "COCOS2DX_ROOT_PATH is not a compatible cocos2d-x checkout: "
                "${COCOS2DX_ROOT_PATH}")
        endif()
    elseif(DEFINED ENV{COCOS2DX_ROOT} AND NOT "$ENV{COCOS2DX_ROOT}" STREQUAL "")
        _cocos2dx_validate_root("$ENV{COCOS2DX_ROOT}" resolved_root)
        if(NOT resolved_root)
            message(FATAL_ERROR
                "COCOS2DX_ROOT does not point to a compatible cocos2d-x checkout: "
                "$ENV{COCOS2DX_ROOT}")
        endif()
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cocos2d")
        _cocos2dx_validate_root("${CMAKE_CURRENT_SOURCE_DIR}/cocos2d" resolved_root)
        if(NOT resolved_root)
            message(FATAL_ERROR
                "${CMAKE_CURRENT_SOURCE_DIR}/cocos2d exists but is not a compatible "
                "cocos2d-x checkout")
        endif()
    else()
        file(GLOB nearby_candidates LIST_DIRECTORIES TRUE
            "${CMAKE_CURRENT_SOURCE_DIR}/../cocos2d-x"
            "${CMAKE_CURRENT_SOURCE_DIR}/../cocos2d-x-*"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../cocos2d-x"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../cocos2d-x-*"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../cocos2d-x"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../cocos2d-x-*")

        set(valid_roots)
        foreach(candidate IN LISTS nearby_candidates)
            _cocos2dx_validate_root("${candidate}" valid_root)
            if(valid_root)
                list(APPEND valid_roots "${valid_root}")
            endif()
        endforeach()
        list(REMOVE_DUPLICATES valid_roots)
        list(LENGTH valid_roots valid_root_count)

        if(valid_root_count EQUAL 1)
            list(GET valid_roots 0 resolved_root)
        elseif(valid_root_count GREATER 1)
            string(REPLACE ";" "\n  " formatted_roots "${valid_roots}")
            message(FATAL_ERROR
                "Multiple nearby cocos2d-x checkouts were found:\n"
                "  ${formatted_roots}\n"
                "Select one with -DCOCOS2DX_ROOT_PATH=/path/to/cocos2d-x "
                "or the COCOS2DX_ROOT environment variable.")
        else()
            message(FATAL_ERROR
                "Unable to locate cocos2d-x. Set COCOS2DX_ROOT, pass "
                "-DCOCOS2DX_ROOT_PATH=/path/to/cocos2d-x, or create an optional "
                "cocos2d link in the project root.")
        endif()
    endif()

    set(COCOS2DX_ROOT_PATH "${resolved_root}" CACHE PATH
        "Path to the cocos2d-x source checkout" FORCE)
    set(${out_var} "${resolved_root}" PARENT_SCOPE)
    message(STATUS "Using cocos2d-x from ${resolved_root}")
endfunction()
