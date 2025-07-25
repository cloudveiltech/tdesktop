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

## 7. Build issue
- The current sentry window build is broken the telegram build. So we are using the prebuilt version.
- Please use the prebuilt version in the sentry folder tdesktop>Telegram>SourceFiles>cloudveil>sentry>include_win
- The prebuilt version for mac is available at include_mac folder

- Please rename the `include_x` and `release_x` to `inclued` and `release` before make the build. Where x is the the OS environment.

## References

- [sentry-native GitHub](https://github.com/getsentry/sentry-native)
- [Crashpad GitHub](https://github.com/chromium/crashpad)
- [Breakpad GitHub](https://github.com/google/breakpad)

---