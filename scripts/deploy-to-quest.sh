#!/usr/bin/env bash
# deploy-to-quest.sh
# Install an APK to a connected Meta Quest headset via ADB, then launch.
#
# Usage:
#   ./scripts/deploy-to-quest.sh [path/to/app.apk]
#
# If no APK path is given, searches ./Build/Android/ for the most recent APK.
# Requires: ADB installed and Quest connected in developer mode over USB or Wi-Fi ADB.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
PACKAGE_NAME="com.yourorg.nineteeneightyfour"
BUILD_DIR="$PROJECT_ROOT/Build/Android"

error() { echo "[ERROR] $*" >&2; exit 1; }
info()  { echo "[INFO]  $*"; }

# -----------------------------------------------------------------------
# Find ADB
# -----------------------------------------------------------------------
command -v adb &>/dev/null || error "ADB not found. Run scripts/setup-android-sdk.sh or add platform-tools to PATH."

# -----------------------------------------------------------------------
# Find APK
# -----------------------------------------------------------------------
if [[ $# -ge 1 ]]; then
    APK="$1"
    [[ -f "$APK" ]] || error "APK not found: $APK"
else
    # Pick the most recently modified APK in the build directory
    APK=$(find "$BUILD_DIR" -name "*.apk" -printf '%T@ %p\n' 2>/dev/null \
            | sort -n | tail -1 | awk '{print $2}')
    [[ -n "$APK" ]] || error "No APK found in $BUILD_DIR — run 'make package' first."
fi

# -----------------------------------------------------------------------
# Check device
# -----------------------------------------------------------------------
info "ADB devices:"
adb devices

DEVICES=$(adb devices | grep -v "^List" | grep -v "^$" | grep -v "offline" | grep "device$" | wc -l)
[[ "$DEVICES" -gt 0 ]] || error "No Quest device detected.
  1. Enable developer mode: Meta Quest app → Headset Settings → Developer Mode
  2. Connect USB cable and accept the 'Allow USB debugging' prompt in the headset
  3. Or use Wi-Fi ADB: adb connect <quest-ip>:5555"

info ""
info "APK : $APK  ($(du -sh "$APK" | cut -f1))"
info "Device count: $DEVICES"

# -----------------------------------------------------------------------
# Check for OBB
# -----------------------------------------------------------------------
OBB=$(find "$BUILD_DIR" -name "*.obb" | head -1)

# -----------------------------------------------------------------------
# Install
# -----------------------------------------------------------------------
info "Installing APK..."
adb install -r "$APK"
info "APK installed."

if [[ -n "$OBB" ]]; then
    info "Pushing OBB data file..."
    OBB_FILENAME=$(basename "$OBB")
    adb shell mkdir -p "/sdcard/Android/obb/$PACKAGE_NAME/"
    adb push "$OBB" "/sdcard/Android/obb/$PACKAGE_NAME/$OBB_FILENAME"
    info "OBB pushed."
fi

# -----------------------------------------------------------------------
# Launch
# -----------------------------------------------------------------------
info "Launching 1984..."
adb shell am start -n "$PACKAGE_NAME/com.epicgames.ue4.GameActivity"
info ""
info "=== Deployment complete ==="
info "View logcat: adb logcat -s UE4"
