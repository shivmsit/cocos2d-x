#!/bin/sh
# Copyright (c) 2026 Shivpratap Chauhan <shivmsit@gmail.com>
# SPDX-License-Identifier: MIT

set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
platform=macosx
arch=$(uname -m)
config=Release
android_api=24
android_ndk=${ANDROID_NDK_HOME:-${ANDROID_NDK_ROOT:-}}
jobs=${CMAKE_BUILD_PARALLEL_LEVEL:-}

if [ -z "$jobs" ]; then
    if command -v sysctl >/dev/null 2>&1; then
        jobs=$(sysctl -n hw.logicalcpu 2>/dev/null || true)
    fi
    if [ -z "$jobs" ] && command -v getconf >/dev/null 2>&1; then
        jobs=$(getconf _NPROCESSORS_ONLN 2>/dev/null || true)
    fi
fi
case "$jobs" in
    ''|*[!0-9]*) jobs=4 ;;
esac

while [ "$#" -gt 0 ]; do
    case "$1" in
        --platform) platform="$2"; shift 2 ;;
        --arch) arch="$2"; shift 2 ;;
        --config) config="$2"; shift 2 ;;
        --ndk) android_ndk="$2"; shift 2 ;;
        --api) android_api="$2"; shift 2 ;;
        --jobs) jobs="$2"; shift 2 ;;
        *) echo "Usage: $0 --platform macosx|android [--arch ARCH] [--config Debug|Release] [--jobs N] [--ndk PATH --api LEVEL]" >&2; exit 2 ;;
    esac
done

case "$jobs" in
    ''|0|*[!0-9]*) echo "--jobs must be a positive integer." >&2; exit 2 ;;
esac

case "$platform" in
    macosx)
        cmake -S "$script_dir/macosx" -B "$script_dir/macosx/build/$arch-$config" \
            -DCMAKE_BUILD_TYPE="$config" -DCMAKE_OSX_ARCHITECTURES="$arch"
        cmake --build "$script_dir/macosx/build/$arch-$config" --target cocos2d_prebuilt --parallel "$jobs"
        ;;
    android)
        if [ -z "$android_ndk" ] || [ ! -f "$android_ndk/build/cmake/android.toolchain.cmake" ]; then
            echo "Android NDK not found. Pass --ndk PATH or set ANDROID_NDK_HOME." >&2
            exit 2
        fi
        cmake -S "$script_dir/android" -B "$script_dir/android/build/$arch-$config" \
            -DCMAKE_BUILD_TYPE="$config" \
            -DCMAKE_TOOLCHAIN_FILE="$android_ndk/build/cmake/android.toolchain.cmake" \
            -DANDROID_ABI="$arch" -DANDROID_PLATFORM="android-$android_api"
        cmake --build "$script_dir/android/build/$arch-$config" --target cocos2d_prebuilt --parallel "$jobs"
        ;;
    *) echo "Unsupported platform: $platform" >&2; exit 2 ;;
esac
