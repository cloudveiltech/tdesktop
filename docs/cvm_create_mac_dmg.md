# Packaging CloudVeil Messenger for macOS

These steps start from a universal Release build (see [building-mac.md](building-mac.md)). The app is `out/Release/CloudVeil Messenger.app`.

macOS Gatekeeper blocks apps that aren't signed with a **Developer ID Application** certificate and notarized by Apple. An ad-hoc signed DMG shows an "app is damaged / can't be opened" warning on users' Macs, so signing and notarization are required for distribution.

## 1. Check the build

    APP="out/Release/CloudVeil Messenger.app"
    lipo -archs "$APP/Contents/MacOS/CloudVeil Messenger"      # expect: x86_64 arm64
    vtool -show-build "$APP/Contents/MacOS/CloudVeil Messenger" | grep minos

Ship the bundle as `CloudVeilMessenger.app` (no space), the name earlier DMGs used, so the new version replaces the old one in `/Applications`. The build produces `CloudVeil Messenger.app`, so copy it under the new name **before** signing:

    cp -c -R "out/Release/CloudVeil Messenger.app" CloudVeilMessenger.app
    APP=CloudVeilMessenger.app

## 2. Sign (Developer ID, hardened runtime)

List the signing identities available on this Mac:

    security find-identity -v -p codesigning

If the certificate is in a separate keychain (recommended), add `--keychain <path to keychain>` to each `codesign` command. Sign nested code first (the bundled Swift libraries in `Contents/Frameworks`), then the app, using the entitlements in the repository:

    IDENTITY="Developer ID Application: <Company name> (<TEAMID>)"
    find "$APP/Contents" \( -name "*.dylib" -o -name "*.framework" -o -perm -111 -type f \) \
        -not -path "*/MacOS/CloudVeil Messenger" -print0 |
        xargs -0 -I{} codesign --force --options runtime --timestamp --sign "$IDENTITY" "{}"
    codesign --force --options runtime --timestamp \
        --entitlements Telegram/Telegram/Telegram.entitlements \
        --sign "$IDENTITY" "$APP"
    codesign --verify --deep --strict --verbose=2 "$APP"

Notarize and staple the app itself before you package it (as in step 4, using a zip: `ditto -c -k --keepParent "$APP" app.zip`). That way the app also passes Gatekeeper when it's copied out of the DMG without a network connection.

## 3. Create the DMG

    brew install create-dmg
    mkdir -p stage && cp -c -R "$APP" stage/
    # bg.png: the 902x381 CloudVeil Messenger background (logo + arrow), as used since 5.x
    create-dmg \
      --volname "CloudVeil Messenger" \
      --volicon "$APP/Contents/Resources/Icon.icns" \
      --background bg.png \
      --window-pos 200 120 \
      --window-size 902 409 \
      --icon-size 128 \
      --icon "CloudVeilMessenger.app" 200 265 \
      --hide-extension "CloudVeilMessenger.app" \
      --app-drop-link 700 265 \
      --no-internet-enable \
      "cvm_macos_7004004.dmg" \
      stage/
    codesign --force --timestamp --sign "$IDENTITY" cvm_macos_7004004.dmg

The number in the file name is the CVM version code: major × 1,000,000 + minor × 1,000 + patch, so 7.4.4 is `7004004`.

If `create-dmg` fails while laying out the window, give your terminal app permission to control Finder: **System Settings → Privacy & Security → Automation**, then enable **Finder** under Terminal (or iTerm2, etc.). This is the most common cause on macOS 13 and later. Then run the command again.

## 4. Notarize and staple

Store the notary credentials once in the Keychain (use an app-specific password, never your Apple ID password):

    xcrun notarytool store-credentials cvm-notary --apple-id <Apple ID> --team-id <TEAMID>

Then submit the DMG and attach the approval ticket:

    xcrun notarytool submit cvm_macos_7004004.dmg --keychain-profile cvm-notary --wait
    xcrun stapler staple cvm_macos_7004004.dmg
    spctl --assess --type open --context context:primary-signature -v cvm_macos_7004004.dmg

If notarization fails, see what went wrong with `xcrun notarytool log <submission id> --keychain-profile cvm-notary`.
