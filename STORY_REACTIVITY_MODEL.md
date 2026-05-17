# Story Reactivity Model

## Scope
- Defines story graph/reactivity architecture.
- Owns how `Next Match` and `Training` write/read flags.
- Does not own world grounding, cast tuning, or prose content.

## 1) Player Loop (UI Contract)
Two player actions are required for this phase:
1. `Next Match`
2. `Training`

Meaning:
- `Next Match` advances toward the next scheduled mainline match.
- `Training` is optional and can be repeated; it writes behavior/style/conflict flags.

Action visibility note:
- Availability is state-gated.
- Hidden actions should not be explained with placeholder messaging.

## 2) Mainline Progression Contract
The campaign has a fixed schedule of `mainline matches`.

- Mainline matches are the structural spine.
- A mainline match can be `ranked`, `personal`, or `club`.
- Participating in each scheduled mainline match advances the story.
- Wins/losses change response, not access.

## 2.1) Onboarding Exception (Pre-Hub)
- Before normal story hub loop begins, run the fixed onboarding chain in `STORY_ONBOARDING_CONTRACT.md`.
- During onboarding:
  - story hub is gated
  - `Training` is gated
  - sequence order is fixed; only flavor variants react to flags

## 3) Node Graph Model
Convergent/divergent logic applies to all node types, not only matches.

### 3.1 Node Types
1. `scene_node`
- conversation, reflection, social beat, disagreement, quiet slice-of-life

2. `match_node`
- mainline or inserted match

3. `decision_node`
- explicit player choice branch (if used)

4. `transition_node`
- routing/merge control node (usually invisible)

### 3.2 Graph Behavior
- `convergent_node`: merges many paths into one canonical spine beat.
- `divergent_node`: branches variants based on flags/history.
- A node may be unrelated to the upcoming match.

## 4) Reactive Insert System
Before a scheduled mainline match resolves, the graph may inject additional content.

Insert types:
1. `inserted_scene`
2. `inserted_personal_match`
3. `inserted_aftermath`

Use case:
- low training + serious teammate tension -> conflict scene -> optional personal match -> return to scheduled mainline

Rules:
- Inserts are conditional on flags.
- Inserts can be skipped if conditions are not met.
- Inserts never permanently block return to the mainline spine.

## 5) Event Flags and History
Gameplay and node outcomes write flags that later nodes read.

### 5.1 Flag Families
1. `commitment_flags`
- examples: `never_trains`, `steady_grind`, `dropoff_after_grind`, `late_lock_in`

2. `style_flags`
- examples: `power_lean`, `technical_lean`, `spin_lean`, `style_shift_recent`

3. `performance_flags`
- examples: `close_loss`, `dominant_win`, `comeback`, `collapse`, `slump`, `bounce_back`

4. `relationship_flags`
- examples: `trust_rising:kai`, `friction:jolo`, `respect:tix`, `closeness:aya`, `sync:benji`

5. `conflict_flags`
- examples: `effort_dispute`, `style_disagreement`, `not_taking_it_seriously`

### 5.2 Flag Lifetimes
- `instant`: valid for current node chain only.
- `short`: valid for current and next few nodes.
- `persistent`: valid season-wide unless explicitly resolved.

### 5.3 Flag Resolution
Some flags are resolved (not deleted) by writing a counterpart:
- `effort_dispute` -> `effort_dispute_resolved` or `effort_dispute_worsened`

This preserves history for later callbacks.

## 6) Node-Local Interpretation (Critical)
There is no single global weighting model for all nodes.

Each node defines its own interpretation profile:
- which flags it reads
- which history window it cares about
- which NPCs can appear
- what it writes after resolving

Examples:
- Coach node may prioritize commitment + adaptation.
- Rival node may prioritize style + pressure behavior.
- Friend conflict node may prioritize training consistency + relationship tension.

## 7) Relationship-Driven Interaction Routing
Who appears is driven by compatibility/friction with active flags.

For any candidate interaction node:
1. Evaluate active flags and unresolved conflicts.
2. Score candidate NPC interactions per node-local profile.
3. Select deterministic best candidate.
4. Apply deterministic tie-break (`priority`, then `node_id`).

This supports:
- "trained together a lot" -> serious match-focused conversation
- "didn't train" -> conflict conversation and possible additional match

## 7.1) Crew Model (Act 2 Divergence) (Locked)
Act 2 diverges by **which friend group you gravitate toward**, not by branching into separate week schedules.

Crews:
- `Grind/Systems`: Tix (+ Coach proximity). "Show me reps, show me details."
- `Heart/Social`: Aya + Kai (+ Issa glue). "Show up, don't spiral, keep it human."
- `Chaos/Talent`: Benji (+ Jolo energy). "Play by feel, shortcuts, swagger, friction with structure."

Notes:
- Crew affinity is tracked from a small set of signals (example: played Tix lunch match, training volume lately, recent performance tags, recent style expression).
- Default definition of `lately` for training volume is the last 2 story weeks: `training_used_last_week + training_used` (node-local thresholds decide banding).
- Fractures are primarily small disagreements over behavior and social frictions, resolved on the table, then converged back into the regionals arc.

## 8) Match Semantics in Story
Matches are tools for narrative resolution, not only ranking gates.

Match categories in story graph:
1. `mainline_ranked_match`
2. `mainline_personal_match`
3. `mainline_club_match`
4. `inserted_personal_match`

All use the same gameplay rules; only narrative stakes differ.

## 9) Determinism Contract
- Given the same save state + same inputs, node resolution is identical.
- No random scene selection at runtime.
- Graph traversal and tie-breaks are deterministic.
- Insert triggers are deterministic from active flags/history.

## 10) Authoring Constraints
- Keep the mainline spine finite and readable.
- Use divergence for expression, not branch explosion.
- Prefer short, composable inserts over giant one-off branches.
- Ensure every divergent chain has a convergent return path.

## 11) Implementation Order (Before Prose)
1. Define graph schema (`node_type`, conditions, writes, next links).
2. Define flag schema and lifetimes.
3. Define mainline schedule data.
4. Implement deterministic resolver (`Next Match` traversal + insert handling).
5. Implement `Training` flag writer hooks.
6. Dry-run simulated seasons and verify convergent returns.
7. Only then write scene text.

## Dependency References
- Product-level constraints: `STORY_MODE_PRINCIPLES.md`
- Setting/tone constraints: `STORY_GROUNDING_BIBLE.md`
- Cast and profile constraints: `STORY_CAST_LOCK.md`
