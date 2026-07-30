<img src="docs/cocos2dx_portrait.png" width="200" alt="Cocos2d-x logo">


cocos2d-x
=========

| macOS | Android |
| --- | --- |
| [![macOS](https://github.com/shivmsit/cocos2d-x/actions/workflows/build.yml/badge.svg?branch=v4.1)](https://github.com/shivmsit/cocos2d-x/actions/workflows/build.yml?query=branch%3Av4.1) | [![Android](https://github.com/shivmsit/cocos2d-x/actions/workflows/android.yml/badge.svg?branch=v4.1)](https://github.com/shivmsit/cocos2d-x/actions/workflows/android.yml?query=branch%3Av4.1) |

[cocos2d-x][1] is a multi-platform framework for building 2D games, interactive
books, demos and other graphical applications. It is based on
[cocos2d-iphone][2], but instead of using Objective-C, it uses C++.

**Cocos2d-x Framework Architecture**:

![](docs/framework_architecture_v4.png)

cocos2d-x is:

  * Fast
  * Free
  * Easy to use
  * Community supported

Project maintenance
-------------------

The [original Cocos2d-x project][3] is no longer actively maintaining the
native engine. This repository preserves its history and continues
development for teams that value a lightweight, source-available C++ game
engine. It focuses on current toolchains, source-built dependencies, prebuilt
engine libraries, compatibility fixes and support for existing games.

This is an independent community-maintained project and is not an official
Cocos product. See the [changelog](CHANGELOG) for changes made since the
original 4.0 release.

Git checkout
------------

Clone this repository together with its tool and test-resource submodules:

```sh
git clone --recursive --branch v4.1 \
  https://github.com/shivmsit/cocos2d-x.git
cd cocos2d-x
```

All third-party source required by the supported macOS, Android and Linux
builds is versioned in the `external/` submodule. There is no
dependency-download step and `download-deps.py` is no longer used.

If the repository was cloned without `--recursive`, initialize its remaining
submodules afterwards:

```sh
git submodule update --init --recursive
```

The Android SDK/NDK and platform compilers remain normal prerequisites supplied
by the developer machine.

How to start a new game
-----------------------

```sh
cd cocos2d-x
python3 setup.py
source FILE_TO_SAVE_SYSTEM_VARIABLE
cocos new MyGame -p com.your_company.mygame -l cpp -d NEW_PROJECTS_DIR
cd NEW_PROJECTS_DIR/MyGame
```

Use `-l lua` to create a Lua project.

New games reuse this engine checkout instead of storing another full copy,
keeping project directories smaller and engine updates in one place.

Building prebuilt libraries for faster builds
---------------------------------------------

Generated C++ projects use prebuilt engine libraries by default on macOS and
Android. Build the engine once for each target and configuration, then reuse it
across game projects. If a matching library does not exist, the first project
build creates it automatically.

macOS Debug:

```sh
./prebuilt/build-prebuilt.sh \
  --platform macosx \
  --arch "$(uname -m)" \
  --config Debug
```

Android arm64 Debug:

```sh
./prebuilt/build-prebuilt.sh \
  --platform android \
  --arch arm64-v8a \
  --api 24 \
  --config Debug \
  --ndk "$ANDROID_NDK_HOME"
```

Repeat the Android command for each ABI the game ships. Debug builds retain the
native information needed to diagnose crashes.

Set `COCOS2DX_BUILD_PREBUILT_IF_MISSING=OFF` to require an explicit prebuild,
or use `-DCOCOS2DX_ENGINE_MODE=SOURCE` to compile the engine with the game. See
the [prebuilt library guide](prebuilt/README.md) for more information.

Android
-------

Open the generated `proj.android` directory in Android Studio, or build it from
the command line:

```sh
cd proj.android
./gradlew assembleDebug
```

Install and launch the app:

```sh
adb install -r app/build/outputs/apk/debug/app-debug.apk
adb shell am start -n com.your_company.mygame/org.cocos2dx.cpp.AppActivity
```

Replace the package name in the last command with the package passed to
`cocos new`.

macOS
-----

From the generated game directory:

```sh
cmake -S . -B build/macos-debug \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_OSX_ARCHITECTURES="$(uname -m)"
cmake --build build/macos-debug --parallel
```

Run the app:

```sh
open build/macos-debug/bin/MyGame/MyGame.app
```

To generate an Xcode project:

```sh
cmake -S . -B build/xcode -G Xcode
open build/xcode/*.xcodeproj
```

Linux
-----

On Ubuntu or Debian, install the host packages once from the Cocos2d-x
checkout:

```sh
./install-deps-linux.sh
```

Then build and run from the generated game directory:

```sh
cmake -S . -B build/linux-debug \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCOCOS2DX_ENGINE_MODE=SOURCE
cmake --build build/linux-debug --parallel
./build/linux-debug/bin/MyGame/MyGame
```

Windows
-------

Windows support is **work in progress**.
`prebuilt/build-prebuilt.sh` is a POSIX shell script for macOS and Android
cross-compilation; it is not the Windows build interface. A native
PowerShell/CMake entrypoint and prebuilt Windows library still need to
be implemented and tested.

The inherited CMake source build can be used experimentally with Visual Studio
2022:

```powershell
cmake -S . -B build/windows -G "Visual Studio 17 2022" -A x64 `
  -DCOCOS2DX_ENGINE_MODE=SOURCE
cmake --build build/windows --config Debug
```

Windows builds are not yet part of the validated release workflow. The
original iOS implementation also remains in the source tree but has not yet
been revalidated with all updated dependencies.

Using CMake
-----------

Cocos2d-x uses CMake to build the engine, its external libraries, tests and
generated games. Use out-of-source build directories so generated files do not
modify the checkout.

Documentation
-------------

* [Prebuilt library guide](prebuilt/README.md)
* [Third-party dependency versions](https://github.com/shivmsit/cocos2d-x-dependencies/blob/main/SOURCES.md)
* [Release notes](docs/RELEASE_NOTES.md)
* [Changelog](CHANGELOG)

The changelog summarizes the major updates and fixes made in this maintained
version.

Main features
-------------

   * Scene management (workflow)
   * Transitions between scenes
   * Sprites and Sprite Sheets
   * Effects: Lens, Ripple, Waves, Liquid, etc.
   * Actions (behaviours):
     * Transformation Actions: Move, Rotate, Scale, Fade, Tint, etc.
     * Composable actions: Sequence, Spawn, Repeat, Reverse
     * Ease Actions: Exp, Sin, Cubic, Elastic, etc.
     * Misc actions: CallFunc, OrbitCamera, Follow, Tween
   * Basic menus and buttons
   * Integrated with physics engines: [Box2D][5] and [Chipmunk][6]
   * Particle system
   * Skeleton Animations: [Spine][7] and Armature support
   * Fonts:
     * Fast font rendering using Fixed and Variable width fonts
     * Support for .ttf fonts
   * Tile Map support: Orthogonal, Isometric and Hexagonal
   * Parallax scrolling
   * Motion Streak
   * Render To Texture
   * Touch/Accelerometer on mobile devices
   * Touch/Mouse/Keyboard on desktop
   * Sound playback through platform audio backends
   * Integrated Slow motion/Fast forward
   * Fast and compressed textures: PVR compressed and uncompressed textures,
     ETC1 compressed textures, and more
   * Resolution Independent
   * Language: C++, with Lua bindings
   * Open Source Commercial Friendly (MIT): compatible with open and closed
     source projects
   * Metal and OpenGL rendering backends

Build requirements
------------------

* Git
* CMake 3.22 or later
* Python 3.8 or later for `setup.py` and the `cocos` console
* A C/C++ compiler for the target platform
* Current Xcode or Xcode Command Line Tools for macOS
* JDK 17 or later for Android
* Android SDK 36
* Android API 24 or later
* Android NDK `30.0.15729638`
* Android CMake `3.22.1`

The Android versions above are the tested defaults in generated projects and
are configured in `proj.android/gradle.properties`.

Running tests
-------------

Build the macOS C++ and Lua test applications:

```sh
cmake -S . -B build-local/macos-tests \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_OSX_ARCHITECTURES="$(uname -m)"
cmake --build build-local/macos-tests --parallel
```

Build the Android C++ tests:

```sh
cd tests/cpp-tests/proj.android
./gradlew assembleDebug
```

Build the Android Lua tests:

```sh
cd tests/lua-tests/project/proj.android
./gradlew assembleDebug
```

`cpp-tests` is the primary engine behavior and compatibility suite. It includes
the Chipmunk2D tests and the Box2D 3.1.1 examples added in this version.

Learning resources
------------------

* [C++ coding style](docs/CODING_STYLE.md)
* [Android fundamentals](https://developer.android.com/guide/components/fundamentals)
* [Games From Scratch Cocos2d-x tutorials](http://www.gamefromscratch.com/page/Cocos2d-x-CPP-Game-Programming-Tutorial-Series.aspx)
* [Historical Cocos2d-x documentation](https://github.com/cocos2d/cocos2d-x/tree/v4/docs)

Where to get help
-----------------

* [Bug Tracker](https://github.com/shivmsit/cocos2d-x/issues)
* [Pull Requests](https://github.com/shivmsit/cocos2d-x/pulls)
* `cpp-tests`, the primary source of working engine examples

When reporting a problem, include the target platform and architecture, build
configuration, compiler and SDK versions, relevant logs and a minimal
reproduction when possible.

Contributing to the project
---------------------------

Cocos2d-x is licensed under the [MIT License](licenses/LICENSE_cocos2d-x.txt).
Contributions are welcome through this repository's issue tracker and pull
requests.

Keep third-party source and engine integration changes in separate
commits when practical, update `external/SOURCES.md` when dependency revisions
change and test the affected platform.

[1]: https://github.com/cocos2d/cocos2d-x "Cocos2d-x"
[2]: https://github.com/cocos2d/cocos2d-iphone "Cocos2d for iPhone"
[3]: https://github.com/cocos2d/cocos2d-x "Original Cocos2d-x repository"
[5]: https://box2d.org "Box2D"
[6]: https://chipmunk-physics.net "Chipmunk2D"
[7]: https://esotericsoftware.com/ "Spine"
