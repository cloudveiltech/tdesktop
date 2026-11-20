# Creating the CloudVeil Messenger Windows installer

These steps start from a Release build (see [building-win.md](building-win.md)). The app is `out\Release\CloudVeilMessenger.exe`, already named correctly, so no renaming is needed.

## 1. Check the installer script

Open `Telegram\build\setup.iss` and check the defines at the top:

- `MyAppVersion` / `MyAppVersionFull`: the release version (for example `7.4.4`).
- `MyAppExeName`: `CloudVeilMessenger.exe`.
- `ReleasePath`: `..\..\out\Release`, relative to `Telegram\build`.
- `MyAppId`: **never change this**. Windows uses it to recognize upgrades of an existing install.

## 2. Configure code signing

Windows SmartScreen warns about unsigned installers, so sign both the app and the installer. The installer script signs with a sign tool named `sha256` (`SignTool=sha256`).

1. Install the Windows SDK's `signtool.exe` (it comes with Visual Studio; the Developer Command Prompt puts it on `PATH`). Make sure the code-signing certificate is available: since 2023 it comes on a USB hardware token or through a cloud signing service. `/n` selects the certificate by its exact subject name.
2. In the Inno Setup Compiler, open **Tools → Configure Sign Tools…**. Add a tool named `sha256` with this command:

       "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe" sign /tr http://timestamp.digicert.com /td sha256 /fd sha256 /n "CloudVeil Technology, Inc." $f

3. Sign the app itself before building the installer:

       signtool sign /tr http://timestamp.digicert.com /td sha256 /fd sha256 /n "CloudVeil Technology, Inc." out\Release\CloudVeilMessenger.exe

**Note:** the CloudVeil EV code-signing certificate used for the 5.14 installers expired on 2026-09-08. A renewed certificate is needed before a signed release. [CONFIRM certificate status]

## 3. Build the installer

Install [Inno Setup 6](https://jrsoftware.org/isdl.php), then either open `Telegram\build\setup.iss` in the Inno Setup Compiler and choose **Build → Compile**, or run:

    "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" Telegram\build\setup.iss

The installer is written to `out\Release\cloudveil_messenger_setup-x64.<version>.exe`.

### Windows on ARM (arm64) installer

The arm64 app comes from the GitHub Actions workflow `cvm-win-arm64.yml` (artifact `CloudVeilMessenger-win-arm64-unsigned`). That artifact is the app itself, not an installer. Sign it, put it alone in a folder named `CloudVeilMessenger.exe`, then build the installer on any Windows PC (Inno Setup can package an arm64 app from an x64 machine):

    "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" /DMyBuildTarget=winarm /DReleasePath=C:\path\to\arm64-folder Telegram\build\setup.iss

The installer is written to that folder as `cloudveil_messenger_setup-arm64.<version>.exe`. It only runs on arm64 Windows. Unlike x64, it doesn't ship `d3dcompiler_47.dll`; upstream Telegram leaves it out for arm64 too, and Windows includes it.

Other `/D` options:

- `/DMyAppVersion=<version>`: override the version in the script, for example to package an older app for testing.
- `/DUnsigned`: build a test installer without a configured `sha256` sign tool. Don't ship it.

Both installers use the same `MyAppId`, so installing one over the other upgrades in place and keeps the user's data.

## 4. Verify

    signtool verify /pa /v <installer>.exe

Install it on a clean machine (or over the previous version) and check that CloudVeil Messenger starts and syncs its settings.
