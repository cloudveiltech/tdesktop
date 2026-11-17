## CloudVeil Messenger (CVM) build for Linux

This section describes how CloudVeil Messenger 7.4.x is built for Linux x86_64. The upstream Telegram instructions follow below for reference.

### Requirements

- Docker, [poetry](https://python-poetry.org), git and git-lfs.
- About **80 GB** of free disk space. Plan for **2 GB of RAM per parallel compile job**: default parallelism ran out of memory on a 62 GB machine.

### Prepare the build image

    git clone --recursive <CVM desktop repository URL> tdesktop
    ./tdesktop/Telegram/build/prepare/linux.sh

This builds the `tdesktop:centos_env` Docker image, with all libraries inside it.

### Build

The Telegram **api_id** and **api_hash** must stay private. Don't put them in scripts, shell history, or commits. One way is to read them into variables without echoing them:

    read -rs API_ID; read -rs API_HASH
    cd tdesktop
    docker run --rm -u $(id -u) \
        -v "$PWD:/usr/src/tdesktop" \
        -e CMAKE_BUILD_PARALLEL_LEVEL=8 \
        tdesktop:centos_env \
        /usr/src/tdesktop/Telegram/build/docker/centos_env/build.sh \
        -D TDESKTOP_API_ID="$API_ID" \
        -D TDESKTOP_API_HASH="$API_HASH" \
        -D DESKTOP_APP_DISABLE_AUTOUPDATE=ON \
        -D DESKTOP_APP_DISABLE_CRASH_REPORTS=ON

Set `CMAKE_BUILD_PARALLEL_LEVEL` to about RAM ÷ 8 GB; link-time optimization needs a lot of memory. The result is `out/Release/cloudveil-messenger`.

### Split off debug symbols

The unstripped binary is over 3 GB. Keep the symbols for crash analysis and ship the stripped binary (about 250 MB):

    cd out/Release
    objcopy --only-keep-debug cloudveil-messenger cloudveil-messenger.debug
    strip --strip-debug --strip-unneeded -o cloudveil-messenger.stripped cloudveil-messenger
    objcopy --add-gnu-debuglink=cloudveil-messenger.debug cloudveil-messenger.stripped
    chmod 755 cloudveil-messenger.stripped && mv -f cloudveil-messenger.stripped cloudveil-messenger

### Testing over remote desktop (xrdp + XFCE)

The app talks to `xdg-desktop-portal` at startup. Under xrdp sessions the portal often starts without the session environment, which leaves the app stuck with no window, and links don't open. To fix it:

- `~/.local/share/xdg-desktop-portal/portals/gtk.portal`: copy `/usr/share/xdg-desktop-portal/portals/gtk.portal` and set `UseIn=XFCE;gnome`.
- `~/.config/environment.d/60-cvm-portal.conf`: set `XDG_CURRENT_DESKTOP=XFCE`.
- Add an XFCE autostart entry that runs `dbus-update-activation-environment --systemd DISPLAY XAUTHORITY XDG_CURRENT_DESKTOP`, then `systemctl --user try-restart xdg-desktop-portal xdg-desktop-portal-gtk`.
- For audio over RDP on Debian 12, install `pipewire-module-xrdp` from bookworm-backports.

Run with a separate profile: `./cloudveil-messenger -workdir ~/cvm-test-profile/`.

---

## Build instructions for Linux using Docker

### Prepare folder

Choose a folder for the future build, for example **/home/user/TBuild**. It will be named ***BuildPath*** in the rest of this document. All commands will be launched from Terminal.

### Obtain your API credentials

You will require **api_id** and **api_hash** to access the Telegram API servers. To learn how to obtain them [click here][api_credentials].

### Clone source code and prepare libraries

Install [poetry](https://python-poetry.org), go to ***BuildPath*** and run

    git clone --recursive https://github.com/telegramdesktop/tdesktop.git
    ./tdesktop/Telegram/build/prepare/linux.sh

### Building the project

Go to ***BuildPath*/tdesktop** and run (using [your **api_id** and **api_hash**](#obtain-your-api-credentials))

    docker run --rm -it \
        -u $(id -u) \
        -v "$PWD:/usr/src/tdesktop" \
        tdesktop:centos_env \
        /usr/src/tdesktop/Telegram/build/docker/centos_env/build.sh \
        -D TDESKTOP_API_ID=YOUR_API_ID \
        -D TDESKTOP_API_HASH=YOUR_API_HASH

Or, to create a debug build, run (also using [your **api_id** and **api_hash**](#obtain-your-api-credentials))

    docker run --rm -it \
        -u $(id -u) \
        -v "$PWD:/usr/src/tdesktop" \
        -e CONFIG=Debug \
        tdesktop:centos_env \
        /usr/src/tdesktop/Telegram/build/docker/centos_env/build.sh \
        -D TDESKTOP_API_ID=YOUR_API_ID \
        -D TDESKTOP_API_HASH=YOUR_API_HASH

The built files will be in the `out` directory.

### Visual Studio Code integration

Ensure you've followed the instruction up to the [**Clone source code and prepare libraries**](#clone-source-code-and-prepare-libraries) step at least.

Open the repository in Visual Studio Code, install the [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension and add the following to `.vscode/settings.json` (using [your **api_id** and **api_hash**](#obtain-your-api-credentials)):

    {
        "cmake.configureSettings": {
            "TDESKTOP_API_ID": "YOUR_API_ID",
            "TDESKTOP_API_HASH": "YOUR_API_HASH"
        }
    }

After that, choose **Reopen in Container** via the menu triggered by the green button in bottom left corner and you're done.

![Quick actions Status bar item](https://code.visualstudio.com/assets/docs/devcontainers/containers/remote-dev-status-bar.png)

[api_credentials]: api_credentials.md
