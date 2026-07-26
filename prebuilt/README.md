# Cocos2d-x reusable prebuilts

Build the engine once for each target you ship; game projects then use the
`use_prebuilt_cocos2dx()` CMake helper and compile only their own sources.

```sh
# Native macOS (Apple Silicon example)
./prebuilt/build-prebuilt.sh --platform macosx --arch arm64 --config Release

# Android: repeat once for every ABI you ship
./prebuilt/build-prebuilt.sh --platform android --arch arm64-v8a \
  --ndk "$ANDROID_NDK_HOME" --api 23 --config Release
```

The Android SDK is used by Gradle to package an app. The prebuilt library
itself needs the Android NDK, which supplies the compiler and platform API.
Generated archives remain ignored by Git; only this build infrastructure is
versioned.
