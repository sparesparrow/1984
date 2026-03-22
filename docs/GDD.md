# 1984: VR Simulation — Game Design Document

## Overview

**Title**: 1984
**Genre**: Immersive Simulation / Educational Serious Game
**Platform**: Meta Quest 2, Quest 3, Quest Pro
**Engine**: Unreal Engine 5.3+
**Perspective**: First-person VR (you ARE Winston Smith)

## Core Concept

Players inhabit Airstrip One as Winston Smith, navigating daily life under the Party's perpetual surveillance. Every choice is observed, logged, and has consequences. The VR medium makes surveillance *felt*, not just understood.

## Design Pillars

1. **Presence** — Oppressive environments, telescreens, invisible surveillance
2. **Agency vs. Compliance** — Dual life mechanic (outer conformity, inner resistance)
3. **Educational Depth** — Newspeak, doublethink, historical revisionism through gameplay
4. **Consequence** — Suspicion Meter adapts NPC behavior and story outcomes

---

## Narrative Structure (Five Acts)

### Act I — Ordinary Life (Tutorial)

**Setting**: Victory Mansions, Ministry of Truth
**Duration**: ~30 minutes

- **Scene 1.1 — Morning Routine**: Wake in Winston's flat. Telescreen broadcasts morning exercises. Player learns VR controls through physical exercises.
- **Scene 1.2 — Commute**: Walk through Victory Mansions corridors. Observe Party slogans, Big Brother posters. Learn about NPCs and surveillance.
- **Scene 1.3 — Ministry of Truth**: Arrive at Records Department. Learn Winston's job: rewriting history. First Newspeak tutorial at the work terminal.
- **Scene 1.4 — Two Minutes Hate**: Mandatory participation event. Introduction to crowd dynamics and the suspicion system.

### Act II — Growing Doubt

**Setting**: Victory Mansions, Prole Districts, Ministry of Truth
**Duration**: ~45 minutes

- **Scene 2.1 — The Diary**: Winston acquires the forbidden diary. First high-risk action (+0.15 suspicion). Physical writing mechanic with VR pen.
- **Scene 2.2 — The Prole District**: Exploration of the "free" zones. Contrast with Party areas. Meeting Mr. Charrington.
- **Scene 2.3 — Julia's Note**: Receive the secret note "I love you." Trust decision: engage or report?
- **Scene 2.4 — Brotherhood Rumours**: Overhear whispers about resistance. Choose to investigate or ignore.

### Act III — Active Resistance

**Setting**: Mr. Charrington's room, Ministry of Truth, O'Brien's flat
**Duration**: ~45 minutes

- **Scene 3.1 — Secret Meetings**: Meet Julia in the room above Mr. Charrington's shop. Intimacy in a surveillance state.
- **Scene 3.2 — O'Brien's Invitation**: The pivotal meeting. O'Brien presents the Brotherhood. Player decides: join or refuse.
- **Scene 3.3 — Goldstein's Book**: Read excerpts from "The Theory and Practice of Oligarchical Collectivism." Key educational content delivered through gameplay.
- **Scene 3.4 — The Betrayal**: Mr. Charrington revealed as Thought Police. Arrest sequence.

### Act IV — Capture and Conditioning (Room 101)

**Setting**: Ministry of Love, Room 101
**Duration**: ~30 minutes

- **Scene 4.1 — Ministry of Love**: Imprisonment. Isolation. Time distortion.
- **Scene 4.2 — Interrogation**: O'Brien's questioning. The doublethink mechanic at its most intense. "How many fingers, Winston?"
- **Scene 4.3 — Room 101**: The most impactful VR moment. Player faces their selected fear. Bimanual interaction: choose betrayal or endurance.

### Act V — Resolution

**Setting**: Chestnut Tree Café, Victory Square
**Duration**: ~15 minutes

- **Scene 5.1 — Aftermath**: Based on player's cumulative decisions and suspicion history.
- **Scene 5.2 — Ending A (Total Compliance)**: Winston loves Big Brother. Player sees themselves conform.
- **Scene 5.3 — Ending B (Partial Resistance)**: Winston outwardly complies but retains inner doubt.
- **Scene 5.4 — Ending C (Martyrdom)**: Winston maintains resistance to the end. Rarest ending.
- **Scene 5.5 — Educational Debrief**: Break the fourth wall. Present real-world connections, reflection prompts, decision log export.

---

## Core Mechanics

### Suspicion System
- Global meter: 0.0 (unsuspected) to 1.0 (imminent capture)
- Every player action is observed and scored
- Thresholds trigger escalating NPC behavior and narrative branches
- See `SurveillanceSystem.h` for implementation details

### Doublethink Mechanic
- Bimanual VR interaction: left hand = Party truth, right hand = historical truth
- Cognitive dissonance scoring for educational assessment
- Physical objects that are "both true and false"

### Newspeak Language System
- Voice recognition for spoken Newspeak (Meta Voice SDK)
- Written translation puzzles at Ministry of Truth desk
- Oldspeak ↔ Newspeak translation mini-games

### Telescreen Surveillance
- Every room has active telescreens
- Vision cones detect player actions
- Can be partially obscured (high risk/reward)
- Dynamic propaganda content

---

## Performance Targets

| Metric | Quest 2 | Quest 3 |
|---|---|---|
| Frame rate | 72 Hz | 90 Hz |
| Draw calls | < 200 | < 400 |
| Triangle budget | < 500K | < 1M |
| Memory | < 4 GB | < 8 GB |

---

## Content Rating

Expected: **PEGI 16+ / ESRB Teen**
Content warnings: Psychological themes, authoritarian violence, surveillance, torture (Room 101).
Comfort features: Pause/exit at any time, content warnings before intense scenes, seated mode.

---

*This document is a living design reference. Scene details will be refined during development.*
