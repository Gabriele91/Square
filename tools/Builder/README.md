# Builder

The builds of a game of Square, from the command line or from a window (the UI of the engine, RmlUi):
cmake and ninja, the compiler of a platform (or its SDK), a configuration (Debug, Release, Retail)
of a target; optionally its release (the archives in the compression of the GPUs of the platform, the
shaders packed), its deploy on a mobile, its run.

## The tools

Each one the one given (the command line, the window, the settings), else found:

| Tool | Found in |
|---|---|
| cmake | the PATH; `C:/Program Files/CMake`; the one of Visual Studio; the ones of the Android SDK |
| ninja | the PATH; the one of Visual Studio; the ones of the Android SDK |
| msvc | Visual Studio with its C++ tools (vswhere): its environment from `vcvars64.bat` |
| clang-cl | the LLVM of Visual Studio, `C:/Program Files/LLVM`, the PATH (the environment of `vcvars64.bat`) |
| gcc, clang | the PATH (Linux; macOS: `/usr/bin/clang`) |
| ndk | the NDK of the Android SDK (`ANDROID_HOME`, `ANDROID_SDK_ROOT`, the folder of Android Studio) |
| JDK 17 | `JAVA_HOME`, the JDKs installed, the one of Android Studio (Gradle, the APK) |

`Builder -s` writes what it found.

## The platforms

| Platform | Built by | Compilers | Deploy |
|---|---|---|---|
| windows | Windows | msvc, clang-cl | none (its release: the assets) |
| linux | Linux | gcc, clang | none |
| macos | macOS | clang | none |
| android | any host | ndk | the APK installed, its resources pushed |

## The steps

| Step | What |
|---|---|
| configure | `cmake -G Ninja` in `<builds>/<platform>-<compiler>-<config>` (`SQUARE_PLATFORM`; the compiler only the first time) |
| build | the target (Android: `<target>_apk`; the desktop: with the drivers of the host) |
| assets | the release of the platform, as `engine/cmake/SquareRelease.cmake`: the tools of the host (`AssetPack`, `ShaderPack`, Release), the game installed, its archives (`assets/*.sqz` but the `EXCLUDE` of `release.cmake`), its shaders packed in `<builds>/release/<platform>-<config>` |
| deploy | a mobile: `<target>_android_install`, then the release pushed (`SquareAndroidPush.cmake`), else `<target>_android_push` |
| run | the device: `<target>_android_run`; the desktop: the game of the release, else the one of the build from the folder of the game |

## The command line

```
Builder -s                                              the tools found
Builder -g . -t Rush -p windows -c Release              a build
Builder -g . -t Rush -p android -c Retail -a -d -r      the release on the device, started
Builder -p android -c Debug -n                          its steps, not run
```

| Option | |
|---|---|
| `-g` game | the folder of the game (its `CMakeLists.txt`; default: the working folder) |
| `-t` target | the target of the game |
| `-p` platform | windows, linux, macos, android (a build from the command line) |
| `-c` config | Debug, Release, Retail |
| `-x` compiler | msvc, clang-cl; gcc, clang; ndk (default: the first found) |
| `-o` output | the folder of the builds (default: `<game>/builds`) |
| `-e` engine | the folder of the engine (default: the one of the game, `-DSQUARE_DIR`) |
| `-a` | the assets packed for the platform, in any configuration (Debug, Release, Retail): the archives in the compression of its GPUs, shaders.sqz |
| `-d` | deploy: a mobile |
| `-r` | run |
| `-n` | the steps, not run |
| `-s` | the tools found |
| `-m`, `-j`, `-k`, `-v` | cmake, ninja, the Android SDK, the JDK (kept in the settings) |
| `-b` | the render of the window: vk, ogl, d3d, mtl |

Builder runs with the engine of its own build (`Squarer`): a build of that same folder cannot link
the engine while Builder runs from it (the builds of Builder are its own: `<builds>/...`).

Without `-p`, `-s` or `-n`: the window. What was asked last is kept in
`<app data>/square/builder.json` (the tools too: `vcvars`, `clang_cl`, `clang`, `gcc` only there).

## The window

The fields of a build, the tools (empty: found), Build, Plan (its steps), Detect (the tools found
again), Stop (the command running and all it started), the log of the last 400 lines.
