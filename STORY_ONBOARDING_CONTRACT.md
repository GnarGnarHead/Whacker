# Story Onboarding Contract (Locked)

## Status
- Locked on: `2026-02-17`
- Amended on: `2026-02-23` (midweek bridge: YouTube -> 1967 -> Tix choice)
- Purpose: prevent drift between agreed onboarding design and implemented flow.
- Scope: first-time story onboarding only (post-intro acceptance through story hub unlock).

## Canonical Role
- This is the only source of truth for onboarding sequence and gates.
- If implementation and other docs disagree, this contract wins.

## Non-Negotiable Sequence
After intro acceptance, onboarding proceeds in this fixed order:

Notation:
- `S##` = scene node
- `M##` = match node

1. `S01` Early Arrival Scene
- Player arrives early before full club activity.
- Player meets `Aya` first, one-on-one.
- Short chat establishes tone and first social signal.

2. `M01` Aya Friendly Match
- Casual warm-up match (not official standings).
- Purpose: let interaction be expressed through play immediately.
- Result affects flavor/line tone, not access.

3. `S02` Club Floor Introduction Scene
- Aya introduces player to Coach/club.
- Short, human introductions; no long speeches.
- Reactive callouts use intro + warm-up signals.

4. `M02` Entry Game (Casual Benchmark)
- Second casual in-club match.
- Purpose: establish how player reads in a group context.
- Still not official progression standings.

5. `S03` Coach Welcome + Training Brief
- Coach frames training matches as technique/style development.
- This moment unlocks training and starts the bridge into the story hub loop.

6. `S04` At-Home YouTube Scene (midweek bridge)
- Short solo beat that tees up the dream-match tone.
- No branching; keeps momentum without adding new cast.

7. `M03` Imagination `1967` Match (dream)
- A contained “imagination match” that does **not** count as a weekly official.
- Purpose: aspiration spark + style emphasis, not standings.

8. `S05` Tix Midweek Scene (choice)
- Short Tix beat that resolves the `1967` moment back into grounded club reality.
- Presents a binary choice for an optional lunch set.

9. `M04` Tix Lunch Match (optional)
- If accepted in `S05`, run one optional match with Tix.
- If declined, skip directly to hub.

## UI and Progression Gates
- Story Hub is not shown until onboarding is complete (`S05`, and `M04` only if chosen).
- `Training` action stays hidden/disabled until `S03` completes (it may unlock before the hub is shown).
- First post-onboarding `Next Match` target is `Club Placement Match`.

## Presentation Rules
- Only active match players wobble while speaking.
- Non-active speakers do not wobble.
- Dialogue during gameplay appears between balls only, never mid-rally.
- Text typing remains character-by-character for dialogue beats.

## Reactivity Hooks (Onboarding)
- Style read hooks: `power`, `technical`, `spin`, `balanced`.
- Performance read hooks: strong win, close game, clear loss.
- Commitment seed hooks: early discipline/readiness signals.
- Hooks alter line flavor and who reacts first, not sequence order.

## Determinism Rules
- Same save state + same inputs produces identical onboarding traversal.
- No random speaker/scene selection during onboarding.
- Tie-breaking between eligible variants is deterministic.

## Implementation Done Criteria
- All onboarding steps exist and execute in order (`S01..S05`, plus `M01..M03` and optional `M04`).
- `Aya` first-meet and first friendly match are both present.
- A second casual entry match exists before hub unlock.
- Training unlock is explicitly tied to Coach briefing completion.
- Midweek bridge exists and is wired:
  - `S04` At-home YouTube
  - `M03` Imagination `1967` match
  - `S05` Tix midweek scene with lunch yes/no choice
  - `M04` Tix lunch match runs only on “yes”
- No direct jump from intro completion to Story Hub.
