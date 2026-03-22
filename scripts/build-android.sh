#!/usr/bin/env bash
# build-android.sh
# Wrapper around UE5 RunUAT to cook, build, stage, and package the 1984
# project for Meta Quest (Android ARM64).
#
# Usage:
#   ./scripts/build-android.sh [options]
#
# Options:
#   -c, --config    Build config: Development|Shipping|DebugGame  (default: Development)
#   -f, --flavor    Texture format: ASTC|Multi                    (default: ASTC)
#   -o, --output    Output directory                              (default: ./Build/Android)
#   -d, --deploy    Deploy via ADB after packaging                (flag)
#   -h, --help      Show this help
#
# Required environment variables:
#   UE_ROOT         — Unreal Engine installation root (e.g. ~/UnrealEngine)
#   ANDROID_SDK_ROOT
#   ANDROID_NDK_ROOT

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
UPROJECT="$PROJECT_ROOT/1984.uproject"

# -----------------------------------------------------------------------
# Defaults
# -----------------------------------------------------------------------
BUILD_CONFIG="Development"
COOK_FLAVOR="ASTC"
OUTPUT_DIR="$PROJECT_ROOT/Build/Android"
DEPLOY=false

# -----------------------------------------------------------------------
# Argument parsing
# -----------------------------------------------------------------------
while [[ $# -gt 0 ]]; do
    case "$1" in
        -c|--config)  BUILD_CONFIG="$2"; shift 2 ;;
        -f|--flavor)  COOK_FLAVOR="$2"; shift 2 ;;
        -o|--output)  OUTPUT_DIR="$2"; shift 2 ;;
        -d|--deploy)  DEPLOY=true; shift ;;
        -h|--help)
            grep '^#' "$0" | sed 's/^# \{0,2\}//'
            exit 0 ;;
        *) echo "[ERROR] Unknown argument: $1" >&2; exit 1 ;;
    esac
done

# -----------------------------------------------------------------------
# Validate environment
# -----------------------------------------------------------------------
error() { echo "[ERROR] $*" >&2; exit 1; }
info()  { echo "[INFO]  $*"; }

[[ -n "${UE_ROOT:-}" ]] || error "UE_ROOT is not set. Export it or add to your shell profile."
[[ -d "$UE_ROOT" ]]     || error "UE_ROOT directory not found: $UE_ROOT"
[[ -f "$UPROJECT" ]]    || error ".uproject not found at $UPROJECT — has the UE5 project been initialized?"

UAT="$UE_ROOT/Engine/Build/BatchFiles/RunUAT.sh"
[[ -f "$UAT" ]] || error "RunUAT.sh not found at $UAT"

case "$BUILD_CONFIG" in
    Development|Shipping|DebugGame) ;;
    *) error "Invalid build config: $BUILD_CONFIG (use Development|Shipping|DebugGame)" ;;
esac

case "$COOK_FLAVOR" in
    ASTC|Multi) ;;
    *) error "Invalid cook flavor: $COOK_FLAVOR (use ASTC|Multi)" ;;
esac

# -----------------------------------------------------------------------
# Build
# -----------------------------------------------------------------------
mkdir -p "$OUTPUT_DIR"

info "=== 1984 Android Build ==="
info "  Config  : $BUILD_CONFIG"
info "  Flavor  : $COOK_FLAVOR"
info "  Output  : $OUTPUT_DIR"
info "  UE root : $UE_ROOT"
info ""

START_TIME=$(date +%s)

"$UAT" BuildCookRun \
    -project="$UPROJECT" \
    -noP4 \
    -platform=Android \
    -clientconfig="$BUILD_CONFIG" \
    -cook \
    -build \
    -stage \
    -pak \
    -package \
    -cookflavor="$COOK_FLAVOR" \
    -archivedirectory="$OUTPUT_DIR" \
    -archive \
    -verbose

END_TIME=$(date +%s)
ELAPSED=$(( END_TIME - START_TIME ))
info "Build completed in ${ELAPSED}s"

# -----------------------------------------------------------------------
# List output
# -----------------------------------------------------------------------
info ""
info "=== Output files ==="
find "$OUTPUT_DIR" -name "*.apk" -o -name "*.obb" | while read -r f; do
    SIZE=$(du -sh "$f" | cut -f1)
    info "  $SIZE  $f"
done

# -----------------------------------------------------------------------
# Deploy
# -----------------------------------------------------------------------
if [[ "$DEPLOY" == true ]]; then
    info ""
    info "=== Deploying to Quest ==="
    command -v adb &>/dev/null || error "ADB not found — run scripts/setup-android-sdk.sh first."
    DEVICES=$(adb devices | grep -v "^List" | grep "device$" | wc -l)
    [[ "$DEVICES" -gt 0 ]] || error "No Quest device found via ADB. Enable USB debugging and connect the headset."

    APK=$(find "$OUTPUT_DIR" -name "*.apk" | head -1)
    [[ -n "$APK" ]] || error "No APK found in $OUTPUT_DIR"

    info "Installing: $APK"
    adb install -r "$APK"
    info "Launching application..."
    adb shell am start -n "com.yourorg.nineteeneightyfour/com.epicgames.ue4.GameActivity"
    info "Launched."
fi
