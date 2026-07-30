# macOS Cocos2d-x prebuilt

Build the engine once per architecture/configuration:

```sh
./prebuilt/build-prebuilt.sh --platform macosx --arch arm64 --config Debug
```

An application that uses `add_subdirectory(${COCOS2DX_ROOT_PATH}/prebuilt)` and
`use_prebuilt_cocos2dx(app)` automatically finds this archive. If it is absent,
the first CMake configure builds it; later game builds only compile the game.

The resulting archive is under `prebuilt/macosx/build/<arch>-<config>/lib/`.
Delete that specific build directory to deliberately rebuild the engine after
changing cocos2d-x sources or build settings.
