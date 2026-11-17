# Packaging CloudVeil Messenger for macOS

These steps start from a universal Release build (see [building-mac.md](building-mac.md)). The app is `out/Release/CloudVeil Messenger.app`.

macOS Gatekeeper blocks apps that aren't signed with a **Developer ID Application** certificate and notarized by Apple. An ad-hoc signed DMG shows an "app is damaged / can't be opened" warning on users' Macs, so signing and notarization are required for distribution.

## 1. Check the build

    APP="out/Release/CloudVeil Messenger.app"
    lipo -archs "$APP/Contents/MacOS/CloudVeil Messenger"      # expect: x86_64 arm64
    vtool -show-build "$APP/Contents/MacOS/CloudVeil Messenger" | grep minos

The DMG has always shipped the bundle as `CloudVeilMessenger.app` (no space), and the built bundle name has a space. Renaming it changes the path in `/Applications`, so existing users may end up with two copies. [CONFIRM which bundle name to ship.] If you rename the bundle, do it **before** signing.

## 2. Sign (Developer ID, hardened runtime)

List the signing identities available on this Mac:

    security find-identity -v -p codesigning

Sign nested code first, then the app, using the entitlements in the repository:

    IDENTITY="Developer ID Application: <Company name> (<TEAMID>)"
    find "$APP/Contents" \( -name "*.dylib" -o -name "*.framework" -o -perm -111 -type f \) \
        -not -path "*/MacOS/CloudVeil Messenger" -print0 |
        xargs -0 -I{} codesign --force --options runtime --timestamp --sign "$IDENTITY" "{}"
    codesign --force --options runtime --timestamp \
        --entitlements Telegram/Telegram/Telegram.entitlements \
        --sign "$IDENTITY" "$APP"
    codesign --verify --deep --strict --verbose=2 "$APP"

## 3. Create the DMG

    brew install create-dmg
    mkdir -p CVM && cp -R "$APP" CVM/ && cp <background image> CVM/bg.png   # 800x600 recommended
    create-dmg \
      --volname "CloudVeil Messenger" \
      --background "CVM/bg.png" \
      --window-pos 200 120 \
      --window-size 900 600 \
      --icon-size 160 \
      --icon "CloudVeil Messenger.app" 200 240 \
      --hide-extension "CloudVeil Messenger.app" \
      --app-drop-link 660 260 \
      "cvm_macos_7004004.dmg" \
      "CVM/"
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
