# Android Cocos2d-x prebuilt

Build one archive for each Android ABI you ship. The Android SDK is not needed
by CMake itself; the NDK provides the compiler, platform libraries, and CMake
toolchain.

```sh
./prebuilt/build-prebuilt.sh --platform android --arch arm64-v8a \
  --ndk "$ANDROID_NDK_HOME" --api 23 --config Release
```

Supported ABI values are those accepted by the installed NDK, commonly
`arm64-v8a`, `armeabi-v7a`, `x86_64`, and `x86`. The archive is written to
`prebuilt/android/build/<abi>-<config>/lib/libcocos2d-prebuilt.a`.
