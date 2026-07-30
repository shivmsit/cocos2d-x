# Cocos2d-x 4.1.0 release notes

Cocos2d-x 4.1 updates the native engine for current development toolchains
while preserving the Cocos2d-x name and APIs.

## Highlights

* Builds third-party dependencies from pinned source in the `external`
  submodule.
* Adds reusable Debug and Release engine libraries for macOS and Android.
* Updates Android projects to Gradle 9.5, Android Gradle Plugin 9.3.1,
  SDK 36, API 24 and NDK 30.
* Updates Box2D to 3.1.1 and adds a basic test with debug drawing.
* Builds LuaJIT, tolua++ and LuaSocket from source.
* Updates `setup.py` and the Cocos Console for Python 3.
* Builds Linux audio with miniaudio instead of the retired FMOD package.

Generated projects use a shared Cocos2d-x checkout. They create a matching
prebuilt engine library when one is not already available.

## Compatibility

Projects using the old Box2D API need to migrate their Box2D code to 3.1.1.
Existing C++ and Lua projects can continue to use their Cocos2d-x APIs, but
should adopt the updated CMake and Android project files.

macOS, Android and Linux source builds were validated for this release.
Windows and iOS have not yet been revalidated with all updated dependencies.

## Cocos2d-x 4.0

Cocos2d-x 4.0 introduced Metal rendering on macOS and iOS, moved all
platforms to CMake and removed the deprecated JavaScript bindings.
