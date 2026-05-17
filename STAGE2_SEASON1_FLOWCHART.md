# Stage 2 — Season 1 Planning Flowchart (Obsidian / Mermaid)

## Purpose
Visual planning aid for **structure only**:
- convergent spine
- optional training loop
- deterministic inserts (no branch explosion)

Canonical structure remains: `STAGE2_SEASON1_CONTENT.md`.

Note: “Weeks” here are nominal planning slots, not a hard calendar constraint.

Canvas version (draggable flowchart): `STAGE2_SEASON1_STRUCTURE.canvas`.

---

## 1) Macro Spine (Nominal 8 Week Slots + Anchors)

```mermaid
flowchart TD
  A0([Start Career]) --> A1[Intro Match (Kai)]
  A1 --> O1[Aya warm-up\n(scene + friendly match)]
  O1 --> O2[Club intro\n(Aya → Coach)]
  O2 --> O3[Entry benchmark match\n(Benji)]
  O3 --> O4[Coach brief\n(training unlock)\n+ Tix post-day feedback]
  O4 --> O5[At-home YouTube]
  O5 --> O6['67 dream match]
  O6 --> O7[Tix midweek invite\n(choice)]
  O7 -->|No| H1((Hub: Week 1))
  O7 -->|Yes| O8[Optional lunch match\n(Tix)]
  O8 --> O9[Post-game: thanks\n(classes)]
  O9 --> W1

  subgraph Act1[Act 1 — Entry + Belonging (Weeks 1–2)]
    H1 --> W1[Week 1 Official\nAnchor A: Placement]
    W1 --> H2((Hub: Week 2))
    H2 --> W2[Week 2 Official\nFirst Stakes]
  end

  subgraph Act2[Act 2 — Friction + Identity (Weeks 3–6)]
    W2 --> H3((Hub: Week 3))
    H3 --> W3[Week 3 Official\nStyle Visible]
    W3 --> H4((Hub: Week 4))
    H4 --> W4[Week 4 Official\nTrust + Friction]
    W4 --> H5((Hub: Week 5))
    H5 --> W5[Week 5 Official\nThe Wall]
    W5 --> H6((Hub: Week 6))
    H6 --> W6[Week 6 Official\nAnchor B: Club Semifinal\nMajor-rival Rematch]
  end

  subgraph Act3[Act 3 — Respect + Continuation (Weeks 7–8)]
    W6 --> H7((Hub: Week 7))
    H7 --> W7[Week 7 Official\nRegional Pressure]
    W7 --> H8((Hub: Week 8))
    H8 --> W8[Week 8 Official\nAnchor C: Regional Decider\n+ Quiet Aftermath]
  end

  W8 --> Z0([End: Continuation])
```

---

## 2) Weekly Loop (What “Hub → Training → Next Match → Advance” Means)

```mermaid
flowchart TD
  H((Story Hub\nWeek N)) -->|Optional| T[Training Match\nrepeatable]
  T --> H

  H --> O[Next Match (Official)\nWeek N mainline]

  O --> PM[Post-match feedback + card selection\n(tags + reputation + relationships)]
  PM -->|if Week N < 8| ADV[Advance Story\n(Week N → Week N+1)]
  ADV --> HN((Story Hub\nWeek N+1))
```

---

## 3) Insert Guardrail (Divergence Without Forking the Season)

```mermaid
flowchart TD
  PRE[Before Week N Official] --> CHECK{Insert trigger?\n(flags/tags)}
  CHECK -->|No| OFF[Week N Official]
  CHECK -->|Yes| S[Inserted Scene]
  S -->|Optional| PM[Inserted Personal Match]
  PM --> OFF
  S --> OFF
```
