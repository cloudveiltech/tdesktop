#!/usr/bin/env bash
# Configure a CloudVeil Messenger (tdesktop) macOS build.
# Telegram API credentials are read from the login Keychain
# (service "cvm-tdesktop-api", accounts "api_id" / "api_hash"),
# so they never live in scripts or shell history.
#
# Usage: Telegram/build/cvm/configure-mac.sh <tdesktop checkout> [extra cmake args...]
# Env:   DEVELOPER_DIR       Xcode to use (default: Xcode 16.4)
#        CVM_MACOS_TARGET    minimum macOS (default: 12.0)
set -euo pipefail

src="${1:?usage: $0 <tdesktop checkout> [extra cmake args]}"; shift
src="$(cd "$src" && pwd)"
export DEVELOPER_DIR="${DEVELOPER_DIR:-/Applications/Xcode-16-4.app/Contents/Developer}"
target="${CVM_MACOS_TARGET:-12.0}"

api_id="$(security find-generic-password -s cvm-tdesktop-api -a api_id -w)"
api_hash="$(security find-generic-password -s cvm-tdesktop-api -a api_hash -w)"

# Qt version as configure.py would pick it (override with QT=...).
export QT="${QT:-$(python3 -I -B -c "import sys, os; sys.path.insert(0, '$src/Telegram/build'); import qt_version; qt_version.resolve(''); print(os.environ['QT'])")}"

echo "Xcode:  $(xcodebuild -version | head -1) ($DEVELOPER_DIR)"
echo "Qt:     $QT"
echo "Source: $src  ->  $src/out   (macOS >= $target)"

mkdir -p "$src/out"
cd "$src/out"
cmake -GXcode \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_DEPLOYMENT_TARGET="$target" \
  -DTDESKTOP_API_ID="$api_id" \
  -DTDESKTOP_API_HASH="$api_hash" \
  -DDESKTOP_APP_DISABLE_AUTOUPDATE=ON \
  -DDESKTOP_APP_DISABLE_CRASH_REPORTS=ON \
  "$@" \
  ..
