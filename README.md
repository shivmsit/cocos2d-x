<img src="docs/cocos2dx_portrait.png" width="200" alt="Cocos2d-x logo">


cocos2d-x
=========

| macOS | Android |
| --- | --- |
| [![macOS](https://github.com/shivmsit/cocos2d-x/actions/workflows/build.yml/badge.svg?branch=v4.1)](https://github.com/shivmsit/cocos2d-x/actions/workflows/build.yml?query=branch%3Av4.1) | [![Android](https://github.com/shivmsit/cocos2d-x/actions/workflows/android.yml/badge.svg?branch=v4.1)](https://github.com/shivmsit/cocos2d-x/actions/workflows/android.yml?query=branch%3Av4.1) |

[Cocos2d-x][1] is a multi-platform C++ framework for building 2D games,
interactive books, demos and other graphical applications.

**Cocos2d-x Framework Architecture**:

![](docs/framework_architecture_v4.png)

Cocos2d-x is:

  * Fast
  * Free
  * Easy to use
  * Community supported

Project maintenance
-------------------

The [original Cocos2d-x project][2] is no longer actively maintained. This
fork keeps the Cocos2d-x name and APIs while updating native toolchains,
third-party dependencies and project templates for existing games.

This is an independent community-maintained project and is not an official
Cocos product. Bug reports, fixes and testing on existing Cocos2d-x games are
welcome.

Git checkout
------------

Clone the repository with its submodules:

```sh
git clone --recursive --branch v4.1 \
  https://github.com/shivmsit/cocos2d-x.git
cd cocos2d-x
```

For an existing clone:

```sh
git submodule update --init --recursive
```

All dependencies used by supported builds are versioned in the `external`
submodule and built from source. There is no dependency-download step.

Build requirements
------------------

* Git
* CMake 3.22 or later
* Python 3.8 or later
* A C/C++ compiler for the target platform
* Current Xcode or Xcode Command Line Tools for macOS
* JDK 17 or later for Android
* Android SDK 36
* Android API 24 or later
* Android NDK `30.0.15729638`
* Android CMake `3.22.1`

How to start a new game
-----------------------

```sh
python3 setup.py
source FILE_REPORTED_BY_SETUP
cocos new MyGame -p com.your_company.mygame -l cpp -d NEW_PROJECTS_DIR
cd NEW_PROJECTS_DIR/MyGame
```

Use `-l lua` to create a Lua project.

Generated projects reuse this engine checkout instead of copying the complete
engine into every game.

Building prebuilt libraries for faster builds
---------------------------------------------

Generated C++ projects use reusable engine libraries by default on macOS and
Android. If a matching library is missing, the first game build creates it.

Build macOS Debug explicitly:

```sh
./prebuilt/build-prebuilt.sh \
  --platform macosx \
  --arch "$(uname -m)" \
  --config Debug
```

Build Android arm64 Debug explicitly:

```sh
./prebuilt/build-prebuilt.sh \
  --platform android \
  --arch arm64-v8a \
  --api 24 \
  --config Debug \
  --ndk "$ANDROID_NDK_HOME"
```

See the [prebuilt library guide](prebuilt/README.md) for configuration options.

Build and run a new project for macOS
-------------------------------------

From the generated game directory:

```sh
cmake -S . -B build/macos-debug \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_OSX_ARCHITECTURES="$(uname -m)"
cmake --build build/macos-debug --parallel
open build/macos-debug/bin/MyGame/MyGame.app
```

Build and run a new project for Android
---------------------------------------

Open `proj.android` in Android Studio, or build from the command line:

```sh
cd proj.android
./gradlew assembleDebug
adb install -r app/build/outputs/apk/debug/app-debug.apk
adb shell am start -n com.your_company.mygame/org.cocos2dx.cpp.AppActivity
```

Replace the package name in the last command with the package passed to
`cocos new`.

Build and run a new project for Linux
-------------------------------------

Install the host packages once from the Cocos2d-x checkout:

```sh
./install-deps-linux.sh
```

Then build from the generated game directory:

```sh
cmake -S . -B build/linux-debug \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCOCOS2DX_ENGINE_MODE=SOURCE
cmake --build build/linux-debug --parallel
./build/linux-debug/bin/MyGame/MyGame
```

Windows and iOS
---------------

The inherited Windows and iOS implementations remain in the source tree, but
have not yet been validated for this release. Windows source builds can be
tested with Visual Studio 2022 and
`-DCOCOS2DX_ENGINE_MODE=SOURCE`.

Running tests
-------------

Build the C++ test suite on macOS:

```sh
cmake -S tests/cpp-tests \
  -B tests/cpp-tests/build-macos-debug \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_OSX_ARCHITECTURES="$(uname -m)"
cmake --build tests/cpp-tests/build-macos-debug --parallel
```

Build the C++ test APK:

```sh
cd tests/cpp-tests/proj.android
./gradlew assembleDebug
```

`cpp-tests` includes the Chipmunk2D tests and the Box2D 3.1.1 examples added
in this release.

Documentation
-------------

* [Release notes](docs/RELEASE_NOTES.md)
* [Changelog](CHANGELOG)
* [Prebuilt library guide](prebuilt/README.md)
* [Dependency sources and versions](https://github.com/shivmsit/cocos2d-x-dependencies/blob/main/SOURCES.md)
* [Historical Cocos2d-x documentation](https://github.com/cocos2d/cocos2d-x/tree/v4/docs)

Where to get help
-----------------

* [Bug tracker](https://github.com/shivmsit/cocos2d-x/issues)
* [Pull requests](https://github.com/shivmsit/cocos2d-x/pulls)
* `cpp-tests`, the primary source of working engine examples

When reporting a problem, include the target platform and architecture, build
configuration, compiler and SDK versions, relevant logs and a small
reproduction when possible.

Contributing
------------

Cocos2d-x is licensed under the [MIT License](licenses/LICENSE_cocos2d-x.txt).
Contributions are welcome through this repository's issue tracker and pull
requests.

[1]: https://www.cocos.com/en/cocos2d-x "Cocos2d-x"
[2]: https://github.com/cocos2d/cocos2d-x "Original Cocos2d-x repository"
