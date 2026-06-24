- Install `create-dmg` from `brew` if you don't have it already:

```bash
brew install create-dmg
```

- Collect the `.app` file from your build (for example, `~/TBuild/tdesktop/out/Release/Telegram.app`) and rename it to `CloudVeilMessenger.app`.
- Prepare a folder named `CVM` (or whatever you want your app to be called) and copy the built `.app` file into it.
- Also add a background image named `bg.png` (800x600px recommended) into the same folder.

```bash
create-dmg \
  --volname "CloudVeilMessenger" \
  --background "CVM/bg.png" \
  --window-pos 200 120 \
  --window-size 900 600 \
  --icon-size 160 \
  --icon "CloudVeilMessenger.app" 200 240 \
  --hide-extension "CloudVeilMessenger.app" \
  --app-drop-link 660 260 \
  "cvm_macos_4010009.dmg" \
  "CVM/"
```

Note: if you get an error, try using terminal app.
Make sure Grant Automation permission (most common cause on macOS 13+)

Go to System Settings → Privacy & Security → Automation, find your terminal app (Terminal, iTerm2, etc.) and enable the Finder toggle. Then re-run the command.