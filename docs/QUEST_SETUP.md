# 1984: VR Simulation — Meta Quest Developer Setup Guide

## Prerequisites

### Software

| Tool | Version | Purpose |
|---|---|---|
| Unreal Engine | 5.3+ (LTS preferred) | Game engine |
| Android SDK | API Level 32+ | Quest build target |
| Android NDK | r25+ | Native compilation |
| Meta XR SDK | Latest | Quest VR support (OpenXR) |
| ADB | Latest | Device sideloading |
| Git LFS | 3.0+ | Binary asset versioning |
| Visual Studio 2022 | Latest | C++ compilation (Windows) |
| Xcode 14+ | Latest | C++ compilation (macOS, optional) |

### Hardware

- Meta Quest 2, Quest 3, or Quest Pro
- USB-C cable for tethered debugging
- PC meeting UE5 minimum requirements

## Setup Steps

### 1. Install Unreal Engine

1. Download Epic Games Launcher from [unrealengine.com](https://www.unrealengine.com/)
2. Install UE 5.3 or later (select Android support during install)
3. Verify installation: launch UE5 and create a blank project

### 2. Install Android SDK and NDK

```bash
# Via Android Studio or standalone SDK manager
sdkmanager "platforms;android-32" "build-tools;32.0.0"
sdkmanager "ndk;25.2.9519653"
```

Set environment variables:
```bash
export ANDROID_HOME=/path/to/android-sdk
export ANDROID_NDK_ROOT=/path/to/android-ndk
export PATH=$PATH:$ANDROID_HOME/platform-tools
```

### 3. Install Meta XR SDK

1. Download from [Meta Developer Hub](https://developer.meta.com/horizon/downloads/)
2. Extract to `Plugins/MetaXR/` in the project (or install via Fab marketplace)
3. Enable in UE5: Edit → Plugins → search "Meta XR" → Enable

### 4. Configure Quest for Development

1. Create a Meta developer account at [developer.meta.com](https://developer.meta.com/)
2. Enable Developer Mode on your Quest:
   - Open Meta Quest app on phone
   - Go to Devices → select your headset
   - Enable Developer Mode
3. Connect Quest via USB-C
4. Accept the USB debugging prompt on the headset
5. Verify: `adb devices` should list your Quest

### 5. Clone the Repository

```bash
git lfs install
git clone https://github.com/sparesparrow/1984.git
cd 1984
git checkout claude/plan-vr-xr-port-pCeTo
```

### 6. Open the Project

1. Launch UE5
2. Open `1984.uproject`
3. If prompted, allow UE5 to generate project files
4. Build the project (Build → Build Project1984)

### 7. Configure for Quest

1. Edit → Project Settings → Platforms → Android:
   - Minimum SDK: 29
   - Target SDK: 32
   - Package name: `com.yourorg.nineteeneightyfour`
2. Edit → Project Settings → Plugins → OpenXR:
   - Ensure OpenXR is the active XR runtime
3. Edit → Project Settings → Plugins → Meta XR:
   - Enable hand tracking
   - Enable passthrough (optional, for mixed reality features)

### 8. Deploy to Quest

```bash
# Using UE5 Launch (recommended)
# In UE5 Editor: Platforms → Android → Launch → Quest device

# Or via command line:
"${UE5_ROOT}/Engine/Build/BatchFiles/RunUAT.sh" \
  BuildCookRun \
  -project="$(pwd)/1984.uproject" \
  -platform=Android \
  -clientconfig=Development \
  -build -cook -stage -pak -deploy \
  -device=quest

# Or sideload manually:
adb install -r Build/Android/*.apk
```

## Troubleshooting

### "Device not found" in ADB
- Ensure USB debugging is enabled on Quest
- Try a different USB-C cable (data cable, not charge-only)
- Run `adb kill-server && adb start-server`

### Build fails with NDK errors
- Verify `ANDROID_NDK_ROOT` points to NDK r25+
- In UE5: Edit → Project Settings → Android → SDK/NDK paths

### OpenXR not detected
- Ensure Meta XR SDK plugin is enabled in UE5
- Check that `DefaultEngine.ini` has `[XRPlugin] DefaultPlugin=OpenXR`

### Hand tracking not working
- Verify Quest firmware is up to date
- Check: Settings → Movement Tracking → Hand and Body Tracking → Enabled
- Ensure `OpenXRHandTracking` plugin is enabled in `.uproject`

### Low frame rate on Quest 2
- Enable App Spacewarp in project settings
- Set Fixed Foveated Rendering to Level 2+
- Check draw call count in `stat unit` console command
- Target: <200 draw calls, <500K triangles

---

*Guide based on UE 5.3 + Meta XR SDK. Update paths and versions as the project evolves.*
