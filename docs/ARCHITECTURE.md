# 1984: VR Simulation — Technical Architecture

## System Overview

```
┌─────────────────────────────────────────────────────┐
│                   Game Instance                      │
│  ┌──────────────────┐  ┌─────────────────────────┐  │
│  │ SurveillanceSystem│  │   NarrativeManager      │  │
│  │  (Subsystem)      │  │   (Subsystem)           │  │
│  │                   │  │                          │  │
│  │ GlobalSuspicion   │  │ CurrentAct / Scene       │  │
│  │ SurveillanceActors│  │ DecisionHistory          │  │
│  │ ThresholdEvents   │←→│ EndingDetermination      │  │
│  └──────────────────┘  └─────────────────────────┘  │
│  ┌──────────────────┐  ┌─────────────────────────┐  │
│  │ NewSpeakProcessor │  │ SurveillanceAudioManager│  │
│  │  (Subsystem)      │  │  (Subsystem)            │  │
│  │                   │  │                          │  │
│  │ Dictionary        │  │ TensionLevel             │  │
│  │ VoiceRecognition  │  │ SpatialAudio             │  │
│  └──────────────────┘  └─────────────────────────┘  │
└─────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────┐
│                    World (Level)                      │
│  ┌──────────────┐  ┌────────────┐  ┌──────────────┐ │
│  │  VRPawnBase   │  │ NPCBase(s) │  │ Telescreens  │ │
│  │  (Player)     │  │            │  │ (Components) │ │
│  │              │  │ Loyalty     │  │              │ │
│  │ HandTracking  │  │ Detection  │  │ VisionCone   │ │
│  │ HandInteract  │  │ Patrol AI  │  │ Broadcast    │ │
│  │ Suspicion     │  │            │  │ Surveillance  │ │
│  │  Component    │  │ ThoughtPol │  │              │ │
│  └──────────────┘  └────────────┘  └──────────────┘ │
└─────────────────────────────────────────────────────┘
```

## Module Structure

### `Source/Project1984/`

| Directory | Purpose | Key Classes |
|---|---|---|
| `Core/GameMode/` | Session management, game rules | `Project1984GameMode`, `Project1984GameState` |
| `Core/Characters/` | Player and NPC characters | `WinstonCharacter`, `NPCBase` |
| `Core/Systems/` | Global game systems | `SurveillanceSystem`, `NarrativeManager`, `NewSpeakProcessor`, `ThoughtPoliceAI`, `SuspicionComponent` |
| `VR/` | VR-specific functionality | `VRPawnBase`, `HandInteraction`, `HandTracking`, `TelescreenComponent` |
| `UI/` | World-space VR UI | `PropagandaHUD`, `DoublethinkWidget` |
| `Audio/` | Spatial audio management | `SurveillanceAudioManager` |

## Core Systems

### 1. Surveillance System (`USurveillanceSystem`)

**Type**: `UGameInstanceSubsystem`
**Role**: Central hub for all surveillance mechanics.

- Manages a global suspicion float (0.0–1.0)
- Registered surveillance actors (telescreens, NPCs, Thought Police) report incidents
- Threshold crossings (0.3, 0.6, 0.8) trigger narrative events
- Broadcasts `OnSuspicionThresholdChanged` delegate

### 2. Narrative Manager (`UNarrativeManager`)

**Type**: `UGameInstanceSubsystem`
**Role**: Five-act story progression with branching.

- Scene-graph driven — each act contains multiple scenes
- Suspicion level can force act transitions (e.g., >0.8 → Act IV)
- Tracks decision history for multi-ending determination
- Three possible endings: Total Compliance, Partial Resistance, Martyrdom

### 3. Suspicion Component (`USuspicionComponent`)

**Type**: `UActorComponent`
**Role**: Per-actor suspicion tracking.

- Attached to Winston and surveillance-aware NPCs
- Reports events to the global `SurveillanceSystem`
- Natural suspicion decay when not under surveillance
- Blueprint-accessible for level designers

### 4. Thought Police AI (`AThoughtPoliceAI`)

**Type**: `AAIController`
**Role**: Autonomous patrol, observation, pursuit, and arrest.

- State machine: Patrol → Observe → Pursue → Arrest
- Vision cone detection with configurable angle and range
- Undercover mode (disguised as regular citizens)

### 5. Telescreen Component (`UTelescreenComponent`)

**Type**: `UActorComponent`
**Role**: Dual input/output surveillance device.

- Output: Propaganda broadcast, Two Minutes Hate, Party addresses
- Input: Vision cone surveillance, player detection
- Can be obscured by player (high risk)
- Spatial audio integration

## Data Flow

```
Player Action → SuspicionComponent → SurveillanceSystem → NarrativeManager
                                   → SurveillanceAudioManager (tension update)
                                   → ThoughtPoliceAI (behavior escalation)
                                   → TelescreenComponent (broadcast change)
```

## Platform Configuration

- **Renderer**: Mobile Forward (no Nanite/Lumen)
- **XR Runtime**: OpenXR with Meta XR extensions
- **Hand Tracking**: OpenXR Hand Tracking extension
- **Audio**: Spatial audio with ambisonic support
- **Build Target**: Android ARM64, API 29+

## Performance Budget

See `Config/DefaultEngine.ini` for renderer settings.
Target: 72Hz Quest 2, 90Hz Quest 3. Fixed Foveated Rendering Level 2 default.

---

*Architecture reflects the scaffold state. Implementation details will evolve during development.*
