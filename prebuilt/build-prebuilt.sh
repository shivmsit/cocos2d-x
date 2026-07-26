#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
platform=macosx
arch=$(uname -m)
config=Release
android_api=23
android_ndk=${ANDROID_NDK_HOME:-${ANDROID_NDK_ROOT:-}}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --platform) platform="$2"; shift 2 ;;
        --arch) arch="$2"; shift 2 ;;
        --config) config="$2"; shift 2 ;;
        --ndk) android_ndk="$2"; shift 2 ;;
        --api) android_api="$2"; shift 2 ;;
        *) echo "Usage: $0 --platform macosx|android [--arch ARCH] [--config Debug|Release] [--ndk PATH --api LEVEL]" >&2; exit 2 ;;
    esac
done

case "$platform" in
    macosx)
        cmake -S "$script_dir/macosx" -B "$script_dir/macosx/build/$arch-$config" \
            -DCMAKE_BUILD_TYPE="$config" -DCMAKE_OSX_ARCHITECTURES="$arch"
        cmake --build "$script_dir/macosx/build/$arch-$config" --target cocos2d_prebuilt
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
        cmake --build "$script_dir/android/build/$arch-$config" --target cocos2d_prebuilt
        ;;
    *) echo "Unsupported platform: $platform" >&2; exit 2 ;;
esac
