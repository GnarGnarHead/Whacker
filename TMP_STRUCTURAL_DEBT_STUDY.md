# Structural Debt Study

Date: 2026-02-19
Scope: runtime/story orchestration, state ownership, and smoke-test architecture.
Goal: identify high-payoff structural cleanup with low gameplay risk.

## Summary

The main debt is not dead code. It is orchestration concentration and state plumbing:

1. very large runtime/story entrypoints with high parameter counts,
2. shared mutable state spread across many modules,
3. test harnesses tightly coupled to internals.

This is maintainability debt that will slow future story development unless reduced.

## Objective Hotspots

### Largest Files (LOC)

- `tests/runtime_input_phase_smoke.cpp` (~3027)
- `tests/sim_smoke.cpp` (~1627)
- `tests/story_integration_smoke.cpp` (~1104)
- `tests/runtime_handlers_smoke.cpp` (~1086)
- `tests/story_menu_continue_smoke.cpp` (~823)
- `app/audio_engine.cpp` (~920)
- `app/story_text.cpp` (~448)
- `app/menu_overlay.cpp` (~402)
- `app/runtime_step_phase.cpp` (~399)
- `app/story_overlays.cpp` (~396)

### Large Signatures (Parameter Count)

- `handle_runtime_input_phase(...)` (24): `app/runtime_input_phase.hpp:22`
- `handle_runtime_step_phase(...)` (20): `app/runtime_step_phase.hpp:21`
- `update_window_title(...)` (20): `app/window_title.hpp:21`
- `render_runtime_frame(...)` (19): `app/runtime_render_phase.hpp:20`
- `handle_runtime_pause_input(...)` (16): `app/runtime_pause.hpp:23`
- `handle_story_intro_input(...)` (15): `app/story_flow.hpp:25`
- `handle_story_menu_input(...)` (14): `app/story_flow.hpp:42`

### Include-Heavy Units

- `app/app_runtime.cpp` (28 includes): `app/app_runtime.cpp:1`
- `app/runtime_input_phase.cpp` (15 includes): `app/runtime_input_phase.cpp:1`
- `app/story_match.cpp` (13 includes): `app/story_match.cpp:1`

## Ranked Debt Backlog

## 1) Runtime Input Phase Is Overloaded
- Evidence: single function branching all UI states and side effects in one chain (`app/runtime_input_phase.cpp:49`, `app/runtime_input_phase.cpp:117`, `app/runtime_input_phase.cpp:310`).
- Risk if untouched: unrelated regressions when touching one branch; signature churn propagates everywhere.
- Refactor direction: keep current public entrypoint, internally split per-state handlers (`MainMenu`, `OptionsMenu`, `QuickMatchSetup`, `StoryMenu`, `StoryIntro`, `StoryScene`, `StoryHub`, `Paused`) with a tiny shared side-effect emitter.

## 2) Runtime Step Phase Mixes Too Many Concerns
- Evidence: narration typing, intro gameplay, active gameplay, AI overrides, scoring transitions, and audio in one loop (`app/runtime_step_phase.cpp:18`, `app/runtime_step_phase.cpp:95`, `app/runtime_step_phase.cpp:282`).
- Risk if untouched: story additions become high-risk edits in core simulation step path.
- Refactor direction: extract `step_intro`, `step_story_scene`, `step_playing` helpers with explicit contracts; leave accumulator loop as a dispatcher.

## 3) App Loop Owns Too Much Wiring
- Evidence: loop constructs and threads nearly all runtime state and subsystems (`app/app_runtime.cpp:37`, `app/app_runtime.cpp:105`, `app/app_runtime.cpp:137`, `app/app_runtime.cpp:190`).
- Risk if untouched: every new subsystem increases signature pressure and coordination bugs.
- Refactor direction: introduce a `RuntimeContext` aggregate and phase adapters that take context subsets.

## 4) Story Flow Handlers Are Branch-Heavy and Sticky
- Evidence: deeply nested phase handling and modal transitions in one function (`app/story_flow.cpp:15`, `app/story_flow.cpp:72`, `app/story_flow.cpp:112`, `app/story_flow.cpp:180`).
- Risk if untouched: onboarding/intro expansion causes fragile flag interactions.
- Refactor direction: phase-specific handlers (`Invite`, `BetweenBalls`, `NameEntry`, `RivalIntro`) and small transition helpers.

## 5) Story Reset/Initialization Is Duplicated and Wide
- Evidence: large field-by-field reset blocks (`app/story_runtime.cpp:46`, `app/story_runtime.cpp:83`, `app/story_runtime.cpp:123`).
- Risk if untouched: stale-field bugs as state structs grow.
- Refactor direction: shared reset/default helper functions for `StoryIntroState`, `StoryRuntimeState` onboarding flags, and story-hub feedback.

## 6) Shared Mutable State Lacks Guarded Invariants
- Evidence: broad mutable structs used across input/step/render/story (`app/ui_state.hpp:7`, `app/story_state.hpp:44`).
- Risk if untouched: invalid combinations of `AppState`, story phase, and pending flags.
- Refactor direction: introduce invariant helpers and narrow mutator APIs for high-risk transitions.

## 7) Runtime Input Smoke Tests Are Over-Coupled to Internals
- Evidence: very large global stub/counter surface and repetitive setup (`tests/runtime_input_phase_smoke.cpp:14`, `tests/runtime_input_phase_smoke.cpp:110`, `tests/runtime_input_phase_smoke.cpp:266`).
- Risk if untouched: high maintenance cost for safe refactors.
- Refactor direction: add a shared `TestInputPhaseContext` fixture + event-log assertions instead of raw call counters where feasible.

## 8) Story Integration Smoke Has Boilerplate/Stub Drift
- Evidence: repeated runtime construction and global input stubs (`tests/story_integration_smoke.cpp:23`, `tests/story_integration_smoke.cpp:44`, `tests/story_integration_smoke.cpp:1041`).
- Risk if untouched: integration coverage becomes expensive to evolve.
- Refactor direction: shared story fixture builder and reusable input simulator helpers.

## Do Not Touch Yet

- `render_runtime_frame(...)` is already a thin app-state render dispatcher with concrete GL coupling (`app/runtime_render_phase.cpp:11`). Low payoff now; higher visual-regression risk.

## Suggested Execution Order (Small, Safe Slices)

1. Test harness groundwork:
   - add reusable fixture builders for runtime input and story integration smokes,
   - reduce duplicated setup first, no behavior changes.
2. Runtime input internal split:
   - extract per-state handlers behind current public function.
3. Runtime step internal split:
   - extract intro/scene/playing step helpers.
4. Story flow phase split:
   - phase handlers + transition helpers.
5. Story reset consolidation:
   - canonical reset/default helper functions.
6. Invariant hardening:
   - targeted runtime/story invariant checks and focused regression tests.

## Validation Standard For Each Slice

- Add tests first for the touched boundary.
- Keep runtime behavior unchanged unless explicitly intended.
- Run full verification:
  - `cmake --build build -j`
  - `ctest --test-dir build --output-on-failure`
  - `cmake --build /tmp/whacker_warnings8 -j 2>&1 | tee /tmp/whacker_warnings8.log`
  - `ctest --test-dir /tmp/whacker_warnings8 --output-on-failure`
  - `rg -n "warning:" /tmp/whacker_warnings8.log`
