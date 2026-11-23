# CloudVeil Messenger (CVM) build for Windows

This section describes how CloudVeil Messenger 7.4.x is built for Windows x64. The upstream Telegram instructions follow below for reference.

## Requirements

- **Visual Studio 2022** (17.14 or later) with the **MSVC v143 14.44** toolset, the Windows 10/11 SDK, and C++ desktop development. Visual Studio 2026 also works, as long as the 14.44 toolset is installed.
- **Python 3** (3.10 or later), first on `PATH`.
- **Git for Windows** with long paths enabled: `git config --global core.longpaths true`.
- About **60 GB** of free disk space and at least **16 GB** of RAM.
- To build the installer: **Inno Setup 6**.

Optional speedups for long builds: add a Windows Defender exclusion for ***BuildPath***, pause Windows Search, and switch to the High performance power plan. Undo these afterwards.

## Environment

`Telegram\build\cvm\env-win.cmd` puts Python first on `PATH` and opens the VS 2022 x64 environment with `-vcvars_ver=14.44`. If your Python or Visual Studio is installed somewhere else, set `CVM_PYTHON` and `CVM_VCVARS` first.

## Prepare libraries

Pick ***BuildPath*** (for example `C:\TBuild`), then run:

    cd /d C:\TBuild
    git clone --recursive <CVM desktop repository URL> tdesktop
    call tdesktop\Telegram\build\cvm\env-win.cmd
    tdesktop\Telegram\build\prepare\win.bat silent

The first run takes several hours and fills ***BuildPath*\Libraries\win64** (about 45 GB).

## Configure

The Telegram **api_id** and **api_hash** must stay private. Keep them out of scripts, shell history, and commits. The configure script reads both values from **one line** of standard input, separated by a space:

    echo YOUR_API_ID YOUR_API_HASH| tdesktop\Telegram\build\cvm\configure-win.cmd

The script runs `configure.bat x64` with `DESKTOP_APP_DISABLE_AUTOUPDATE=ON` and `DESKTOP_APP_DISABLE_CRASH_REPORTS=ON`. These match the macOS and Linux builds.

## Build

    tdesktop\Telegram\build\cvm\build-win.cmd

The script builds the `Telegram` target in Release, with output at `out\Release\CloudVeilMessenger.exe`. No renaming is needed.

It uses `/m:1 /p:CL_MPCount=4` because the compiler runs out of heap on a 16 GB machine with the default parallelism. On machines with more memory, raise these values or run `cmake --build out --config Release --target Telegram` directly.

## Windows on ARM (arm64)

The arm64 app can be built on GitHub (`.github/workflows/cvm-win-arm64.yml`) or natively on an arm64 Windows 11 machine, such as a UTM virtual machine on an Apple Silicon Mac. A local build took about 45 minutes for the libraries and 80 minutes for the app with 8 cores and 16 GB of memory; GitHub takes several hours without its cache.

Requirements, matching the GitHub runner:

- **Visual Studio 2026 Build Tools** with the arm64 and x64 C++ tools, the **MSVC 14.44** toolset (`Microsoft.VisualStudio.Component.VC.14.44.17.14.ARM64` and `.x86.x64`), the Windows 11 SDK 10.0.26100, and **ATL for both arm64 and x64** (`Microsoft.VisualStudio.Component.VC.ATL.ARM64` and `Microsoft.VisualStudio.Component.VC.ATL`). Upstream's arm64 scripts build some projects with the VS 2026 `v145` toolset, so VS 2022 alone is not enough; breakpad's `dump_syms` is an x64 tool and needs the x64 ATL.
- **Python 3** (arm64) and **Git for Windows** (arm64), with long paths enabled.
- A git identity (`git config --global user.name` / `user.email`); the dav1d step makes a local commit.
- About 100 GB of free disk space and 16 GB of memory.

Open the environment with

    call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" arm64 -vcvars_ver=14.44

(with Python and Git first on `PATH`), then run the same steps as above with `arm` in place of `x64`:

    tdesktop\Telegram\build\prepare\win.bat silent
    cd tdesktop\Telegram
    configure.bat arm -D TDESKTOP_API_ID=... -D TDESKTOP_API_HASH=... -D DESKTOP_APP_DISABLE_AUTOUPDATE=ON -D DESKTOP_APP_DISABLE_CRASH_REPORTS=ON
    cd ..\out
    cmake --build . --config Release --target Telegram -- /m:2 /p:CL_MPCount=4

Feed the API keys from a password manager or standard input rather than typing them, and keep them out of scripts and logs. While `configure.bat` runs they are visible in its process command line, so don't list process command lines on the build machine during configure.

The app is `out\Release\CloudVeilMessenger.exe`. For the arm64 installer, see [cvm_create_win_installer.md](cvm_create_win_installer.md).

## Installer and signing

See [cvm_create_win_installer.md](cvm_create_win_installer.md).

---

# Build instructions for Windows

- [Prepare folder](#prepare-folder)
- [Install third party software](#install-third-party-software)
- [Choose architecture and initialize terminal](#choose-architecture-and-initialize-terminal)
- [Clone source code and prepare libraries](#clone-source-code-and-prepare-libraries)
- [Build the project](#build-the-project)
- [Qt Visual Studio Tools](#qt-visual-studio-tools)

## Prepare folder

The build is done in **Visual Studio 2026** with **10.0.26100.0** SDK version.

Choose an empty folder for the future build, for example **D:\\TBuild**. It will be named ***BuildPath*** in the rest of this document. Create two folders there, ***BuildPath*\\ThirdParty** and ***BuildPath*\\Libraries**.

For x86 and x64, use `-vcvars_ver=14.44` (`v144.4`, based on `v143` with Windows 7 support). ARM64 uses the modern toolset described below.

### Obtain your API credentials

You will require **api_id** and **api_hash** to access the Telegram API servers. To learn how to obtain them [click here][api_credentials].

## Install third party software

* Download **Python 3.10** installer from [https://www.python.org/downloads/](https://www.python.org/downloads/) and install it with adding to PATH.
* Download **Git** installer from [https://git-scm.com/download/win](https://git-scm.com/download/win) and install it.

## Choose architecture and initialize terminal

Before preparing libraries and running build commands, initialize the Visual Studio environment for your target architecture.
The default modern toolset from Visual Studio 2026 (`v145`) does not support Windows 7, so x86 and x64 builds must use `-vcvars_ver=14.44` (`v144.4`, based on `v143` with Windows 7 support).

For `win` (32-bit):

    %comspec% /k "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars32.bat" -vcvars_ver=14.44

For `win64` (64-bit):

    %comspec% /k "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.44

For `winarm` (ARM64, on an ARM64 host), initialize the toolset used for the application build, for example:

    %comspec% /k "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" arm64 -vcvars_ver=14.52

MSVC 19.52 before 19.52.37025 generates incorrect ARM64 FFmpeg code ([issue #31570](https://github.com/telegramdesktop/tdesktop/issues/31570)). With those versions, dependency preparation automatically selects the 14.51 toolset for FFmpeg alone. Install the 14.51 ARM64 build tools in the same Visual Studio instance; MSVC 19.51.36256 has been verified. Other dependencies and the application keep the terminal's selected toolset. Fixed 19.52 versions build FFmpeg with the selected toolset as well.

Run dependency preparation and accept the FFmpeg rebuild, then rebuild Telegram. The ARM64 FFmpeg cache includes the compiler actually used for FFmpeg, so an existing library built by the affected compiler is not reused automatically. Keep FFmpeg's default non-LTO compilation and use the newer toolset for the final application link when mixing compiler versions.

Run both `Clone source code and prepare libraries` and `Build the project` sections in the terminal initialized with one of the commands above.

## Clone source code and prepare libraries

In the initialized terminal, go to ***BuildPath*** and run

    git clone --recursive https://github.com/telegramdesktop/tdesktop.git
    tdesktop\Telegram\build\prepare\win.bat

## Build the project

Go to ***BuildPath*\\tdesktop\\Telegram** and run (using [your **api_id** and **api_hash**](#obtain-your-api-credentials)):

For `win` (32-bit):

    configure.bat -D TDESKTOP_API_ID=YOUR_API_ID -D TDESKTOP_API_HASH=YOUR_API_HASH

For `win64` (64-bit):

    configure.bat x64 -D TDESKTOP_API_ID=YOUR_API_ID -D TDESKTOP_API_HASH=YOUR_API_HASH

For `winarm` (ARM64):

    configure.bat arm -T version=%VCToolsVersion% -D TDESKTOP_API_ID=YOUR_API_ID -D TDESKTOP_API_HASH=YOUR_API_HASH

* Open ***BuildPath*\\tdesktop\\out\\Telegram.slnx** in Visual Studio 2026
* Select Telegram project and press Build > Build Telegram (Debug and Release configurations)
* The result Telegram.exe will be located in **D:\TBuild\tdesktop\out\Debug** (and **Release**)

### Qt Visual Studio Tools

For better debugging you may want to install Qt Visual Studio Tools:

* Open **Extensions** -> **Manage Extensions**
* Go to **Online** tab
* Search for **Qt**
* Install **Qt Visual Studio Tools** extension

[api_credentials]: api_credentials.md
