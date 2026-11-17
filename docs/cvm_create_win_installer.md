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

1. Install the Windows SDK's `signtool.exe` and the code-signing certificate (or its hardware token).
2. In the Inno Setup Compiler, open **Tools → Configure Sign Tools…**. Add a tool named `sha256` with this command:

       "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe" sign /fd sha256 /tr http://timestamp.digicert.com /td sha256 /a $f

3. Sign the app itself before building the installer:

       signtool sign /fd sha256 /tr http://timestamp.digicert.com /td sha256 /a out\Release\CloudVeilMessenger.exe

**Note:** the CloudVeil EV code-signing certificate used for the 5.14 installers expired on 2026-09-08. A renewed certificate is needed before a signed release. [CONFIRM certificate status]

## 3. Build the installer

Install [Inno Setup 6](https://jrsoftware.org/isdl.php), then either open `Telegram\build\setup.iss` in the Inno Setup Compiler and choose **Build → Compile**, or run:

    "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" Telegram\build\setup.iss

The installer is written to `out\Release\cloudveil_messenger_setup-x64.<version>.exe`.

## 4. Verify

    signtool verify /pa /v <installer>.exe

Install it on a clean machine (or over the previous version) and check that CloudVeil Messenger starts and syncs its settings.
