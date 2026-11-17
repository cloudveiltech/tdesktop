## CloudVeil Messenger (CVM) build for macOS

This section describes how CloudVeil Messenger 7.4.x is built. The upstream Telegram instructions follow below for reference.

### Requirements

- **Xcode 16.4.** Newer Xcode releases (26 and later) cannot target macOS versions below 12.0, and they break parts of the 7.4.x dependency tree. You can install Xcode 16.4 next to a newer Xcode (for example as `/Applications/Xcode-16-4.app`) and select it for the build only:

      export DEVELOPER_DIR=/Applications/Xcode-16-4.app/Contents/Developer

- Homebrew packages: everything in the upstream list below, plus `git-lfs` (needed to fetch some library sources):

      brew install git git-lfs automake libtool cmake wget pkg-config gnu-tar ninja nasm meson
      git lfs install

- About **55 GB** of free disk space for a universal (arm64 + x86_64) build.

### Choose the minimum macOS version

The minimum macOS version is set twice and **both must match**: once when the libraries are prepared, and once when the app is configured.

| Target | Notes |
| --- | --- |
| `10.13` | Same minimum as stock Telegram. Needs Xcode 16.4. |
| `12.0` | Works with newer Xcode releases too. |

The library cache keys include the environment, so changing `MACOSX_DEPLOYMENT_TARGET` rebuilds the libraries.

### Prepare libraries

Go to ***BuildPath*** (the folder that holds `tdesktop`) and run

    git clone --recursive <CVM desktop repository URL> tdesktop
    export DEVELOPER_DIR=/Applications/Xcode-16-4.app/Contents/Developer
    TDESKTOP_MAC_TOOLCHAIN=system MACOSX_DEPLOYMENT_TARGET=10.13 \
        ./tdesktop/Telegram/build/prepare/mac.sh silent

`TDESKTOP_MAC_TOOLCHAIN=system` uses the selected Xcode instead of a downloaded toolchain. `silent` skips the interactive prompts. This step takes several hours on the first run. The libraries land in ***BuildPath*/Libraries** and ***BuildPath*/ThirdParty**.

Before switching to a new tdesktop version or deployment target, back up these folders. An APFS clone is instant and takes no extra space:

    cp -c -R Libraries Libraries-backup
    cp -c -R ThirdParty ThirdParty-backup

### Store the API credentials in the Keychain

The Telegram **api_id** and **api_hash** must stay private. Keep them out of scripts, shell history, and commits. Store them once in the login Keychain:

    security add-generic-password -s cvm-tdesktop-api -a api_id -w
    security add-generic-password -s cvm-tdesktop-api -a api_hash -w

(Each command prompts for the value.)

### Configure

    CVM_MACOS_TARGET=10.13 ./tdesktop/Telegram/build/cvm/configure-mac.sh tdesktop

The script reads the keys from the Keychain. It picks the Qt version the same way `configure.py` does, then generates `tdesktop/out/Telegram.xcodeproj` (Release) with these options:

- `-D DESKTOP_APP_DISABLE_AUTOUPDATE=ON`: CVM updates come from CloudVeil, not Telegram's updater.
- `-D DESKTOP_APP_DISABLE_CRASH_REPORTS=ON`: Telegram's crash reporter is off. CVM uses its own Sentry integration (see `Telegram/SourceFiles/cloudveil/sentry/BUILD.md`).

`CVM_MACOS_TARGET` defaults to `12.0`. Set it to match the libraries.

### Build (universal arm64 + x86_64)

    cd tdesktop/out
    xcodebuild -project Telegram.xcodeproj -scheme Telegram -configuration Release \
        -destination 'generic/platform=macOS' ONLY_ACTIVE_ARCH=NO build

The result is `out/Release/CloudVeil Messenger.app`. Check it with:

    lipo -archs "Release/CloudVeil Messenger.app/Contents/MacOS/CloudVeil Messenger"   # x86_64 arm64
    vtool -show-build "Release/CloudVeil Messenger.app/Contents/MacOS/CloudVeil Messenger" | grep minos

For a quick arm64-only debug build, open `out/Telegram.xcodeproj` in Xcode instead.

**Tip:** don't edit headers while a build is running. Changing a precompiled header mid-build fails the build.

### Test without touching your normal profile

    "out/Release/CloudVeil Messenger.app/Contents/MacOS/CloudVeil Messenger" -workdir ~/cvm-test-profile/

### Package, sign and notarize

See [cvm_create_mac_dmg.md](cvm_create_mac_dmg.md).

---

## Build instructions for macOS

### Prepare folder

Choose a folder for the future build, for example **/Users/user/TBuild**. It will be named ***BuildPath*** in the rest of this document. All commands will be launched from Terminal.

**Note about disk space:** The full build process will require approximately **55 GB** of free space. This includes:
- **~35 GB** for libraries (when building for both x64 and arm64 architectures)
- **~20 GB** for the compiled Telegram app (in the `out` folder)

### Obtain your API credentials

You will require **api_id** and **api_hash** to access the Telegram API servers. To learn how to obtain them [click here][api_credentials].

### Clone source code and prepare libraries

Go to ***BuildPath*** and run

    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    brew install git automake libtool cmake wget pkg-config gnu-tar ninja nasm meson

    sudo xcode-select -s /Applications/Xcode.app/Contents/Developer

    git clone --recursive https://github.com/telegramdesktop/tdesktop.git
    ./tdesktop/Telegram/build/prepare/mac.sh

### Building the project

Go to ***BuildPath*/tdesktop/Telegram** and run (using [your **api_id** and **api_hash**](#obtain-your-api-credentials))

    ./configure.sh -D TDESKTOP_API_ID=YOUR_API_ID -D TDESKTOP_API_HASH=YOUR_API_HASH

Then launch Xcode, open ***BuildPath*/tdesktop/out/Telegram.xcodeproj** and build for Debug / Release.

[api_credentials]: api_credentials.md
