# BUILD-SENTRY-NATIVE

## Building sentry-native Static Libraries on macOS

This guide explains how to build static `.a` libraries for [sentry-native](https://github.com/getsentry/sentry-native) on macOS, for both **Crashpad** and **Breakpad** backends.

---

## 1. Prerequisites

- **Xcode** and **Command Line Tools**  
  ```sh
  xcode-select --install
  ```
- **CMake**  
  ```sh
  brew install cmake
  ```

- **(Optional) Clean submodules if you already cloned:**  
  ```sh
  git submodule update --init --recursive
  ```

---

## 2. Clone the Repository with Submodules

```sh
git clone --recurse-submodules https://github.com/getsentry/sentry-native.git
cd sentry-native
```

---

## 3. Building with Crashpad (Default, Recommended)

Crashpad is the default backend on macOS and is recommended for most users.

### Configure

```sh
cmake -B build-macos-crashpad \
  -DSENTRY_BUILD_SHARED_LIBS=OFF \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_OSX_SYSROOT=$(xcrun --sdk macosx --show-sdk-path)
```

### Build

```sh
cmake --build build-macos-crashpad --parallel
```

### Locate the Static Libraries

- Main Sentry static library:  
  `build-macos-crashpad/src/libsentry.a`
- Crashpad static libraries:  
  - `build-macos-crashpad/external/crashpad/util/libcrashpad_util.a`
  - `build-macos-crashpad/external/crashpad/handler/libcrashpad_handler.a`
  - `build-macos-crashpad/external/crashpad/client/libcrashpad_client.a`
  - `build-macos-crashpad/external/crashpad/minidump/libcrashpad_minidump.a`
  - `build-macos-crashpad/external/crashpad/snapshot/libcrashpad_snapshot.a`

> **You should link all of the above Crashpad `.a` files with your project.**

---

## 4. Building with Breakpad

Breakpad is an older backend and is less commonly used on macOS, but is available if needed.

### Configure

MAC
```sh
cmake -B build-macos-breakpad \
  -DSENTRY_BUILD_SHARED_LIBS=OFF \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DSENTRY_BACKEND=breakpad \
  -DCMAKE_OSX_SYSROOT=$(xcrun --sdk macosx --show-sdk-path)
```

WINDOWS
```sh
cmake -B build-win-breakpad -DSENTRY_BUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSENTRY_BACKEND=breakpad
```
Note: current this window build is broken the telegram build. So we still use the old version 4.x.x

### Build
MAC
```sh
cmake --build build-macos-breakpad --parallel
```

WIN
```sh
cmake --build build-win-breakpad --parallel
```

### Locate the Static Libraries

- Main Sentry static library:  
  `build-macos-breakpad/src/libsentry.a`
- Breakpad static library:  
  `build-macos-breakpad/external/breakpad/src/client/mac/libbreakpad_client.a`

> **You should link both `libsentry.a` and `libbreakpad_client.a` with your project.**

---

## 5. (Optional) Install Artifacts

```sh
cmake --install build-macos-crashpad --prefix install-macos-crashpad
cmake --install build-macos-breakpad --prefix install-macos-breakpad
```

---

## 6. Clean Up

To remove build files:
```sh
rm -rf build-macos-crashpad build-macos-breakpad
```

---

## 7. Prebuilt libraries used by CVM

CVM links prebuilt Sentry libraries directly from per-platform folders, and you don't need to rename anything. `Telegram/CMakeLists.txt` picks the right libraries:

| Platform | Libraries |
| --- | --- |
| macOS | `release_mac/libsentry.a`, `release_mac/libbreakpad_client.a` (Breakpad backend) |
| Windows | `release_win/sentry.lib`, `release_win/breakpad_client.lib` (older 4.x build, because newer Windows builds broke the Telegram link) |

All platforms compile against the shared header `include/sentry.h`, which is currently the same as `include_win/sentry.h`. `include_mac/` holds the header that matches the macOS build. The app only calls API that both versions share. If you upgrade Sentry, keep the calls in `cloudveil/` compatible with both versions.

### Rebuilding the macOS libraries

The macOS libraries must be **universal** (arm64 + x86_64). Their deployment target must also be **no newer than the app's** (`CMAKE_OSX_DEPLOYMENT_TARGET` / `CVM_MACOS_TARGET`), or the linker warns and the app may fail on older macOS. Build with the same Xcode as the app. Use `-ffile-prefix-map` so that no local paths from the build machine end up in the committed binaries:

```sh
export DEVELOPER_DIR=/Applications/Xcode-16-4.app/Contents/Developer
SRC="$PWD"   # the sentry-native checkout
cmake -B build-macos-breakpad \
  -DSENTRY_BUILD_SHARED_LIBS=OFF \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DSENTRY_BACKEND=breakpad \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13 \
  -DCMAKE_C_FLAGS="-ffile-prefix-map=$SRC=/sentry-native" \
  -DCMAKE_CXX_FLAGS="-ffile-prefix-map=$SRC=/sentry-native" \
  -DCMAKE_OSX_SYSROOT=$(xcrun --sdk macosx --show-sdk-path)
cmake --build build-macos-breakpad --parallel
cp build-macos-breakpad/src/libsentry.a \
   build-macos-breakpad/external/breakpad/src/client/mac/libbreakpad_client.a \
   <tdesktop>/Telegram/SourceFiles/cloudveil/sentry/release_mac/
```

Before you commit, check the result:

```sh
lipo -archs release_mac/*.a                      # x86_64 arm64
strings -a release_mac/*.a | grep -c "$HOME"     # expect 0
```

## References

- [sentry-native GitHub](https://github.com/getsentry/sentry-native)
- [Crashpad GitHub](https://github.com/chromium/crashpad)
- [Breakpad GitHub](https://github.com/google/breakpad)

---