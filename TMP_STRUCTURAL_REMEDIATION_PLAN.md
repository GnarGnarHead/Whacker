# Structural Remediation Plan (Release Quality)

Status: in progress  
Owner: Lead Developer (Codex)  
Scope: story runtime architecture, onboarding flow integrity, dialogue ownership, and test coverage

## Objective

Stabilize and simplify story systems before narrative expansion so the codebase is:

1. Coherent to read and maintain.
2. Resistant to state drift and narrative regressions.
3. Testable with fast, deterministic checks.
4. Ready for post-launch source release quality expectations.

## Principles

1. Correctness over speed.
2. Single ownership for prose and narrative decisions.
3. State transitions must be explicit and testable.
4. Remove dead paths and duplicate logic.
5. No behavior surprises (forfeit, routing, menu semantics).

## Audit Findings (Tracked)

`SR-01` Onboarding forfeit advances narrative unexpectedly.  
`SR-02` "Next Match" wording does not match actual mode semantics.  
`SR-03` Dead `SideSelect` intro phase remains in code.  
`SR-04` Duplicate style/performance classifiers in multiple files.  
`SR-05` Duplicate/unused table tennis completion logic in `story_match.cpp`.  
`SR-06` Runtime state stores raw dialogue string (`onboarding_aya_coaching_line`).  
`SR-07` Onboarding writes hub feedback that is typically not player-visible.  
`SR-08` `app_runtime.cpp` still owns too many transition concerns.  
`SR-09` No story/onboarding transition tests.

## Execution Plan

### Phase A - Flow Safety and Semantic Alignment

Goal: eliminate behavior that can produce narrative corruption or user confusion.

1. `SR-01` Fix onboarding forfeit route:
   - Onboarding forfeits must not advance to next onboarding step.
   - Route back to the correct pre-match scene and replay path.
2. `SR-02` Align wording with actual behavior:
   - Remove misleading "another club match" phrasing unless logic truly matches.
   - Keep UI labels and spoken/system copy semantically consistent.
3. `SR-07` Remove non-visible onboarding hub feedback writes:
   - Avoid writing hub feedback for transitions that route directly to `StoryScene`.

Acceptance:

1. Forfeit in `M01` returns to `S01` path; forfeit in `M02` returns to `S02` path.
2. No wording implies a match type different from runtime behavior.
3. Hub feedback only represents states that actually render in hub flow.

### Phase B - State and Logic Dedup

Goal: remove drift vectors and simplify reasoning.

1. `SR-03` Remove dead `SideSelect` path:
   - Remove enum case, state fields, input branch, and overlay branching not used by current design.
2. `SR-04` Extract shared classification functions:
   - Single implementation for style/performance classification.
3. `SR-05` Remove duplicate table tennis completion helper from `story_match.cpp`.
4. `SR-06` Replace runtime raw text storage:
   - Store structured cue fields for Aya post-match line.
   - Resolve final line through `story_text` at render/scene assembly time.

Acceptance:

1. No unused intro side-selection branch remains.
2. One canonical classifier implementation is used everywhere.
3. No duplicate game-complete logic in story match path.
4. `StoryRuntimeState` contains cues/hints, not authored prose.

### Phase C - Runtime Decomposition and Tests

Goal: reduce god-file risk and lock behavior with tests.

1. `SR-08` Extract onboarding transition decisions out of `app_runtime.cpp` into focused helpers.
2. `SR-09` Add non-GLFW story flow tests for:
   - onboarding routing decisions
   - forfeit behavior
   - classifier thresholds
   - key text/selector invariants

Acceptance:

1. `app_runtime.cpp` no longer directly encodes onboarding routing rules.
2. New tests fail if onboarding progression or match routing regresses.
3. `ctest` includes at least one story-flow focused test target.

## Progress Checklist

- [x] Phase A completed
- [x] Phase B completed
- [x] Phase C completed
- [x] Final structural audit re-run
- [x] Documentation update (`MEMORY.md` summary entry)

## Post-Implementation Notes

Resolved:

1. `SR-01` Onboarding forfeit no longer advances storyline; it routes back to prior scene path.
2. `SR-02` Next-match wording no longer claims a specific mismatched match type.
3. `SR-03` Dead `SideSelect` phase and state field removed.
4. `SR-04` Classifier logic centralized in `app/story_classification.cpp`.
5. `SR-05` Duplicate table-tennis complete helper removed from `story_match.cpp`.
6. `SR-06` Runtime no longer stores Aya prose; it stores structured cue fields.
7. `SR-07` Onboarding match paths no longer write non-visible hub feedback.
8. `SR-08` Onboarding route decisions extracted from `app_runtime.cpp` to `app/story_onboarding_flow.cpp`.
9. `SR-09` Added `tests/story_flow_smoke.cpp` for routing/classification regression checks.

Residual:

1. `app_runtime.cpp` remains large overall; onboarding routing logic was extracted, but full runtime decomposition is still a future task.

## Verification Commands

1. `cmake --build build -j`
2. `ctest --test-dir build --output-on-failure`
3. `rg -n "StoryIntroPhase::SideSelect|side_choice" app`
4. `rg -n "onboarding_aya_coaching_line" app`
5. `rg -n "classify_.*style_hint|classify_.*performance_hint" app`
