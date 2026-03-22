# 1984: VR Simulation

An immersive serious game / educational simulation based on George Orwell's *Nineteen Eighty-Four*, built for **Meta Quest 2, Quest 3, and Quest Pro** using **Unreal Engine 5**.

## Project Overview

Players inhabit Airstrip One as Winston Smith — navigating daily life under the Party's perpetual surveillance. The simulation teaches lessons about authoritarianism, propaganda, psychological manipulation, and civil liberties through first-person VR presence. Every choice is observed, logged, and consequential.

**Core pillars:**
- **Presence** — oppressive environments, telescreens, pervasive surveillance
- **Agency vs. Compliance** — dual life mechanic: outer conformity, inner resistance
- **Educational Depth** — Newspeak, doublethink, historical revisionism surfaced through gameplay
- **Consequence** — a Suspicion Meter that adapts NPC behaviour and story outcomes

## Technology Stack

| Component | Choice |
|---|---|
| Engine | Unreal Engine 5.3+ |
| XR | Meta XR SDK (OpenXR) |
| Build target | Android ARM64 (Meta Quest) |
| Renderer | Mobile Forward Renderer |
| Scripting | Blueprints + C++ |
| Assets | Git LFS |

## Development Plan

See [`VR_XR_DEVELOPMENT_PLAN.md`](./VR_XR_DEVELOPMENT_PLAN.md) for the full development roadmap, system architecture, performance targets, and educational framework.

## Getting Started

### Prerequisites

- Unreal Engine 5.3+ (Epic Games Launcher)
- Android SDK (API 32+) and NDK r25+
- Meta XR SDK for UE5 (Meta Developer Hub)
- ADB tools for Quest sideloading
- Git LFS (`git lfs install`)
- Meta Quest device enrolled in Developer Mode

### Setup

```bash
git clone <repo-url>
cd 1984
git lfs pull
# Open 1984.uproject in Unreal Engine
```

See [`docs/QUEST_SETUP.md`](./docs/QUEST_SETUP.md) for full device setup and sideloading instructions.

## Repository Structure

```
1984/
├── Source/          # C++ game code
├── Content/         # UE5 assets (via Git LFS)
├── Config/          # Engine and project configuration
├── Plugins/         # MetaXR SDK
├── docs/            # GDD, architecture docs, educator guide
└── VR_XR_DEVELOPMENT_PLAN.md
```

## License

MIT — see [LICENSE](./LICENSE)
