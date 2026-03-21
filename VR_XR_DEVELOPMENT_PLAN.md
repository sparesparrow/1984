# 1984: VR Simulation — Development & Porting Plan
## Meta Quest (Unreal Engine 5)

**Project**: Immersive serious game / educational simulation based on George Orwell's *Nineteen Eighty-Four*
**Target platform**: Meta Quest 2, Quest 3, Quest Pro
**Engine**: Unreal Engine 5 (UE5)
**Status**: Greenfield — initial planning
**Branch**: `claude/plan-vr-xr-port-pCeTo`

---

## 1. Project Vision

Players inhabit Airstrip One as Winston Smith — navigating daily life under the Party's perpetual surveillance. The simulation teaches lessons about authoritarianism, propaganda, psychological manipulation, and civil liberties through first-person presence. Each choice players make is observed, logged, and has consequences — making the VR medium uniquely powerful: you *feel* watched.

**Core pillars:**
- **Presence** — oppressive environments, telescreens, invisible surveillance
- **Agency vs. Compliance** — dual life mechanic (outer conformity, inner resistance)
- **Educational Depth** — Newspeak, doublethink, historical revisionism surfaced through gameplay
- **Consequence** — a "Suspicion Meter" that adapts NPC behaviour and story outcomes

---

## 2. Technology Stack

| Layer | Choice | Notes |
|---|---|---|
| Engine | Unreal Engine 5.3+ | Latest stable with Meta XR support |
| XR Plugin | Meta XR SDK (OpenXR) | Official Meta plugin for UE5 |
| Build Target | Android (ARM64) | Required for standalone Quest |
| Renderer | Mobile Forward Renderer | Nanite/Lumen unavailable on Quest |
| Lighting | Baked + Dynamic (Stationary) | Pre-computed for performance |
| Scripting | Blueprints + C++ | Blueprints for rapid iteration, C++ for performance-critical systems |
| Version Control | Git LFS | Large binary assets (textures, audio, meshes) |
| CI/CD | GitHub Actions + UE automation | Automated builds and packaging |

---

## 3. Repository Structure (Target)

```
1984/
├── .github/
│   └── workflows/
│       ├── main.yml                  # CI: build validation
│       └── package-quest.yml         # CD: Android APK packaging
├── Source/
│   └── Project1984/
│       ├── Core/
│       │   ├── GameMode/             # 1984GameMode, 1984GameState
│       │   ├── Characters/           # WinstonCharacter, NPCBase
│       │   └── Systems/
│       │       ├── SurveillanceSystem.h/.cpp
│       │       ├── SuspicionComponent.h/.cpp
│       │       ├── NarrativeManager.h/.cpp
│       │       ├── NewSpeakProcessor.h/.cpp
│       │       └── ThoughtPoliceAI.h/.cpp
│       ├── VR/
│       │   ├── VRPawnBase.h/.cpp     # Quest VR pawn
│       │   ├── HandInteraction.h/.cpp
│       │   ├── HandTracking.h/.cpp   # Quest hand tracking
│       │   └── TelescreenComponent.h/.cpp
│       ├── UI/
│       │   ├── PropagandaHUD.h/.cpp  # World-space VR UI
│       │   └── DoublethinkWidget.h/.cpp
│       └── Audio/
│           └── SurveillanceAudioManager.h/.cpp
├── Content/
│   ├── Environments/
│   │   ├── VictoryMansions/          # Winston's apartment block
│   │   ├── MinistryOfTruth/          # Work environment
│   │   ├── ProleDistricts/           # Free zone environments
│   │   └── RoomOneOhOne/             # Interrogation/climax
│   ├── Characters/
│   │   ├── Winston/
│   │   ├── OBrien/
│   │   ├── Julia/
│   │   └── NPCs/                     # Party members, proles, thought police
│   ├── Audio/
│   │   ├── Ambient/                  # Room tones, telescreen audio
│   │   ├── Propaganda/               # Two Minutes Hate, Party broadcasts
│   │   └── Music/                    # Underscore, "Under the Spreading Chestnut Tree"
│   ├── VFX/
│   │   └── Telescreens/              # Dynamic video playback
│   └── UI/
│       └── WorldSpace/               # VR-compatible widgets
├── Config/
│   └── DefaultEngine.ini             # OpenXR, Quest-specific settings
├── Plugins/
│   └── MetaXR/                       # Meta XR SDK (submodule or Fab asset)
├── docs/
│   ├── GDD.md                        # Game Design Document
│   ├── ARCHITECTURE.md               # Technical architecture
│   ├── QUEST_SETUP.md                # Developer setup guide
│   └── EDUCATIONAL_FRAMEWORK.md     # Learning objectives map
├── VR_XR_DEVELOPMENT_PLAN.md        # This file
├── README.md
└── 1984.uproject
```

---

## 4. Core Systems Design

### 4.1 Surveillance System (`SurveillanceSystem`)

The heart of the simulation. Manages all surveillance entities and feeds the suspicion state.

```cpp
// SurveillanceSystem.h
UCLASS()
class USurveillanceSystem : public UGameInstanceSubsystem
{
    // Telescreens: line-of-sight detection zones
    // ThoughtPolice AI: patrol routes, vision cones
    // Citizen NPCs: can report player behaviour
    // GlobalSuspicionLevel: 0.0–1.0 float driving story branches
    float GlobalSuspicion;
    void RegisterSurveillanceActor(ISurveillanceSource* Source);
    void ReportIncident(ESuspicionEvent Event, float Weight);
    void UpdateStoryState();  // Branches narrative at thresholds
};
```

**Suspicion Events** (examples):
| Event | Weight |
|---|---|
| Writing in diary | +0.15 |
| Facial micro-expression (eye tracking) | +0.05 |
| Entering off-limits area | +0.25 |
| Attending Two Minutes Hate | -0.10 |
| Speaking Newspeak correctly | -0.08 |

### 4.2 VR Pawn (`VRPawnBase`)

Mobile-optimised VR pawn targeting Quest 3 (72/90Hz) and Quest 2 (72Hz).

- **Locomotion**: Teleport (default) + smooth locomotion toggle (comfort setting)
- **Hand Interaction**: Pickup, examine, read, write (physical diary interaction)
- **Hand Tracking**: Full skeletal hand tracking via Meta XR SDK
- **Comfort Settings**: Vignette on turn, seated/standing mode, snap turn

### 4.3 Narrative Manager

Scene-graph-based story system with consequence tracking:

```
Act I   → Ordinary life (introduction, tutorial)
Act II  → Growing doubt (diary, Julia, Brotherhood rumours)
Act III → Active resistance (meeting O'Brien, the book)
Act IV  → Capture and conditioning (Room 101 — most impactful VR moment)
Act V   → Resolution (multiple endings based on suspicion history)
```

### 4.4 Telescreen System

Every room has telescreens — interactive and threatening:
- Procedural propaganda video playback (looping montage content)
- Directional audio emanating from each screen
- Vision cone: telescreen "sees" player — triggers suspicion events
- Can be partially obscured (gameplay risk/reward)
- Quest Pro: eye-tracking determines if player is looking at telescreen

### 4.5 Newspeak Processor

Educational mechanic teaching Orwell's constructed language:
- Voice recognition (Meta Voice SDK) for spoken Newspeak phrases
- Written Newspeak via telekinetic letter-block puzzles
- "Translation" mini-games: Oldspeak ↔ Newspeak

### 4.6 Doublethink Mechanic

Unique VR mechanic: holding two contradictory beliefs simultaneously.

- Physical objects that are "both true and false" (duality puzzles)
- Bimanual interaction: left hand = Party truth, right hand = historical truth
- Cognitive dissonance scored to reinforce educational objectives

---

## 5. Unreal Engine 5 + Meta Quest Setup

### 5.1 Prerequisites

```bash
# Required installations:
# - Unreal Engine 5.3+ (from Epic Games Launcher)
# - Android SDK (API Level 32+)
# - Android NDK r25+
# - Meta XR SDK for UE5 (from Meta Developer Hub or Fab)
# - ADB tools for Quest sideloading
# - Git LFS for binary assets
```

### 5.2 UE5 Project Configuration (`DefaultEngine.ini`)

```ini
[/Script/Engine.RendererSettings]
r.MobileHDR=False
r.Mobile.EnableStaticAndCSMShadowReceivers=True

[/Script/Engine.Engine]
bDisableAILogging=True

[/Script/AndroidRuntimeSettings.AndroidRuntimeSettings]
PackageName=com.yourorg.nineteeneightyfour
StoreVersion=1
ApplicationDisplayName=1984
MinimumSDKVersion=29
TargetSDKVersion=32
bEnableAudio=True

[/Script/OpenXRHandTracking.OpenXRHandTrackingSettings]
bEnableHandTracking=True

[XRPlugin]
DefaultPlugin=OpenXR
```

### 5.3 Performance Targets

| Metric | Quest 2 | Quest 3 |
|---|---|---|
| Frame rate | 72 Hz | 90 Hz |
| Resolution | 1832×1920 per eye | 2064×2208 per eye |
| Draw calls | < 200 | < 400 |
| Triangle budget | < 500K | < 1M |
| Memory | < 4 GB | < 8 GB |
| App Spacewarp | Required | Optional |

### 5.4 Rendering Strategy

- **No Nanite** — use hand-crafted LODs (3–4 levels per mesh)
- **No Lumen** — pre-baked lightmaps + stationary lights for key dynamic sources
- **Texture streaming** — mip-maps, max 1K-2K textures for mobile
- **Occlusion culling** — aggressive, especially in tight corridor environments
- **App Spacewarp** — enable for Quest 2 to halve GPU load at 36Hz input
- **Foveated Rendering** — Fixed Foveated Rendering (FFR) level 2 as default

---

## 6. Development Roadmap

### Phase 0: Foundation (Weeks 1–4)
**Goal**: UE5 project running on Meta Quest hardware

- [ ] Initialize UE5 project (`1984.uproject`)
- [ ] Integrate Meta XR SDK plugin
- [ ] Configure Android build pipeline
- [ ] Basic VR pawn: teleport locomotion + controller input
- [ ] Hand tracking integration (Meta OpenXR Hand Tracking)
- [ ] Sideload and validate on Quest 2 and Quest 3
- [ ] Git LFS setup for binary assets
- [ ] GitHub Actions: Android packaging workflow

**Deliverable**: Empty room, playable on device, hand tracking functional

---

### Phase 1: Core Architecture (Weeks 5–10)
**Goal**: All major systems stubbed and individually testable

- [ ] `SurveillanceSystem` subsystem (suspicion tracking)
- [ ] `NarrativeManager` (act/scene graph)
- [ ] `TelescreenComponent` (vision cone + video playback)
- [ ] VR interaction framework (pickup, examine, read, write)
- [ ] Basic NPC AI (patrol, idle, report)
- [ ] Suspicion HUD (diegetic — visible on in-world Party posters)
- [ ] Spatial audio manager (ambisonic, telescreen directional audio)

**Deliverable**: Prototype grey-box room with working surveillance logic

---

### Phase 2: Environment Art (Weeks 11–20)
**Goal**: First playable level — Victory Mansions + Ministry of Truth

- [ ] Modular architecture kit (brutalist Airstrip One aesthetic)
- [ ] Victory Mansions apartment (Winston's flat, shared corridors)
- [ ] Ministry of Truth: Records Department (Winston's desk, pneumatic tubes)
- [ ] Street exterior connecting locations
- [ ] Telescreen assets with dynamic content
- [ ] Character: Winston (mirror reflection + hands)
- [ ] Character: Party NPCs (patrol, observe)
- [ ] Propaganda art assets (posters, slogans, Big Brother imagery)

**Deliverable**: Walkable first act environment, authored lighting

---

### Phase 3: Gameplay Systems (Weeks 21–28)
**Goal**: Full gameplay loop, all mechanics functional

- [ ] Diary writing mechanic (physical VR interaction with book + pen)
- [ ] Newspeak language puzzle system
- [ ] Doublethink bimanual mechanic
- [ ] Two Minutes Hate scripted event (crowd NPC + player participation)
- [ ] Julia introduction: secret note passing mechanic
- [ ] O'Brien meeting: trust/risk choice system
- [ ] Full narrative branching via NarrativeManager
- [ ] Voice recognition for Newspeak (Meta Voice SDK)
- [ ] Quest Pro: eye tracking integration (gaze surveillance)

**Deliverable**: Complete Acts I–III playable

---

### Phase 4: Room 101 and Climax (Weeks 29–34)
**Goal**: Act IV — most impactful VR content

- [ ] Room 101 environment (psychological design, claustrophobia, fear)
- [ ] O'Brien character and interrogation system
- [ ] Phobia system (player-selected fear used against them)
- [ ] Multiple endings (total compliance, partial resistance, martyrdom)
- [ ] Educational debrief scene (break fourth wall, real-world context)

**Deliverable**: Full playthrough end-to-end

---

### Phase 5: Polish & Optimisation (Weeks 35–40)
**Goal**: Quest-ready performance and accessibility

- [ ] Performance profiling (RenderDoc, Unreal Insights, OVR Metrics Tool)
- [ ] LOD pass for all meshes
- [ ] Lightmap rebake at production quality
- [ ] App Spacewarp tuning
- [ ] Comfort settings UI (locomotion, vignette, seated mode)
- [ ] Audio mix pass (spatial, ducking, propagation)
- [ ] Localization framework (English first, other languages TBD)
- [ ] Accessibility: subtitle system, colorblind modes, controller remapping

**Deliverable**: Submission-ready build

---

### Phase 6: Meta Quest Store Submission (Weeks 41–44)
- [ ] Meta Quest Content Review compliance
- [ ] Age rating (PEGI/ESRB — likely 16+)
- [ ] Performance certification (OVR Metrics passing score)
- [ ] Horizon OS Store listing assets (screenshots, trailer, description)
- [ ] Educational institution partnership (schools, universities, museums)
- [ ] Press kit and outreach

---

## 7. Rust Template — Migration Decision

The current repository contains a **Rust template**. For an Unreal Engine project, the Rust code is not needed. Recommended migration:

**Option A — Replace entirely (Recommended)**
- Archive or remove Rust template files (`tests/`, `Makefile`, `devcontainer.json`)
- Replace with UE5 project structure
- Update `.gitignore` for Unreal (generated files, Binaries, Saved, Intermediate)
- Update GitHub Actions for Android packaging

**Option B — Hybrid (Advanced)**
- Keep Rust for backend/tooling (e.g., level data processing, asset pipeline scripts)
- UE5 as the game itself
- Communicate via file I/O or a local service

**Recommended: Option A** for simplicity unless specific Rust tooling is planned.

### `.gitignore` additions for UE5 + Android
```
# Unreal Engine
Binaries/
Build/
Intermediate/
Saved/
DerivedDataCache/
*.VC.db
*.opensdf
*.opendb
*.sdf
*.suo
*.xcodeproj
*.xcworkspace
*.DS_Store

# Android
*.apk
*.obb

# Visual Studio
.vs/
```

---

## 8. Key Risks & Mitigations

| Risk | Likelihood | Mitigation |
|---|---|---|
| Quest 2 performance limits VR presence | High | Forward renderer, App Spacewarp, aggressive LOD from day one |
| Room 101 content causes distress | Medium | Content warnings, pause/exit option at any time, debrief screen |
| Hand tracking reliability in dark environments | Medium | Controller fallback always available, scene lighting tuned for tracking |
| Meta Store content rejection (political themes) | Low-Medium | Early engagement with Meta developer relations, clear educational framing |
| Voice recognition accuracy in Newspeak | Medium | Phoneme-based matching with generous thresholds, visual keyboard fallback |
| Scope creep on content production | High | Modular architecture kit reuse, grey-box first, never block on art |

---

## 9. Educational Framework

**Learning Objectives:**

1. Understand mechanisms of totalitarian control (surveillance, propaganda, language)
2. Recognize doublethink and cognitive dissonance in real-world contexts
3. Experience the psychological cost of living under authoritarian systems
4. Connect Orwell's fiction to historical and contemporary examples

**Assessment integration:**
- Post-play reflection prompts (on-screen at debrief)
- Exportable "decision log" (JSON) for classroom discussion
- Educator guide with discussion questions mapped to learning objectives
- Tie-in with curricula: History, Political Science, Media Studies, Ethics

---

## 10. Immediate Next Steps (This Week)

1. **Decide on Unreal Engine version** — recommend UE 5.3 or 5.4 (LTS preferred)
2. **Create UE5 project** and configure for Android/Meta Quest
3. **Install Meta XR SDK** from [Meta Developer Hub](https://developer.meta.com/horizon/downloads/)
4. **Set up Git LFS** for the repository
5. **Update `.gitignore`** for Unreal + Android
6. **Enroll device in developer mode** on Meta Quest Developer Hub
7. **Validate sideloading pipeline** — blank UE5 project on device
8. **Draft Game Design Document** (`docs/GDD.md`) with scene-by-scene breakdown

---

*Plan authored: 2026-03-21 | Branch: `claude/plan-vr-xr-port-pCeTo`*
