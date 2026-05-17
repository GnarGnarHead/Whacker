# Whacker Codebase Gleam Plan (Study + Execution)

Date: 2026-02-18

## 1) Study Snapshot

- Scope scanned: `app/`, `ai/`, `sim/`, `progression/`, `include/`, `tests/`, `tools/`, `CMakeLists.txt`.
- Current code size: `18080` LOC across `102` C++/header files.
- Largest hotspots:
  - `app/app_runtime.cpp` (1191)
  - `ai/planner.cpp` (1222)
  - `app/audio_engine.cpp` (920)
  - `tests/sim_smoke.cpp` (1627)
- Baseline health:
  - Build: pass (`cmake --build build -j`)
  - Tests: pass (`ctest --test-dir build --output-on-failure`, 4/4)
  - Strict warning pass: `20` warnings with broader flags (`-Wshadow -Wconversion -Wsign-conversion -Wdouble-promotion`)

## 2) Core Findings (Structural, Not Cosmetic)

1. Runtime orchestration is over-concentrated.
- `app/app_runtime.cpp` mixes: input routing, pause policy, story transitions, match progression, audio triggering, typewriter timing, simulation stepping, and rendering dispatch.
- Result: high regression risk when touching any gameplay flow.

2. AI tactical logic is highly entangled and duplicates simulation-side logic.
- `ai/planner.cpp` and `ai/evaluator.cpp` both reconstruct contact outcomes and projection behavior with overlapping math.
- Result: drift risk between planner assumptions and actual simulation behavior.

3. Utility duplication exists in production paths.
- Local `clampf`/`trim_copy` variants appear across multiple app/progression files.
- Result: unnecessary divergence and friction when fixing bugs.

4. Story runtime state has duplication and sync overhead.
- Onboarding fields exist in both runtime and persisted career, bridged by copy helpers.
- Result: easy to miss sync points during transitions and save/load events.

5. Text/content ownership is split across multiple systems.
- Story script text lives in `app/story_text.cpp`, but menu labels and helper strings are in runtime/render files.
- Result: casing/font/content updates are error-prone and inconsistent.

6. Build hygiene is below product standard.
- `build_warnings/` artifacts are tracked in git (`371` tracked files), generating persistent noise.
- Warning policy only applies directly to `whacker_core`; app/tools/tests are less guarded.

7. Test topology is heavy in simulation smoke and lighter in app-flow invariants.
- Good physics coverage exists, but app state-machine behavior remains mostly smoke/integration-level.
- `match_end_flow_smoke` stubs some symbols, reducing full-path confidence.

8. Dead/unused subsystem present.
- `input/` module is compiled into `whacker_core`, but runtime path uses GLFW input directly and does not consume these commands.
- Result: maintenance overhead without runtime value.

9. Save/config parsing is handwritten and repeated.
- `sim/config_io.cpp`, `app/menu_settings.cpp`, `app/story_save.cpp` each use custom parse/trim logic.
- Result: parser inconsistency and avoidable complexity.

## 3) Target End-State (Gold Standard for This Project)

- Clear boundaries:
  - `sim`: canonical physics and contact outcomes.
  - `ai`: decision-making over canonical sim APIs, no duplicate physics logic.
  - `app`: thin orchestration layer, state-machine centric.
  - `story_text`: single source for narrative and UI-facing text copy where practical.
- One source of truth per concept:
  - transitions,
  - exit policy,
  - contact outcome model,
  - text catalog.
- Build/test discipline:
  - warnings applied consistently to all targets,
  - deterministic tests for critical transitions,
  - clean worktree (no generated artifacts tracked).

## 4) Execution Plan (Phased)

## Phase A - Build and Repository Hygiene (No Gameplay Behavior Change)

- Add/align ignore policy for generated build dirs (`build_warnings/` and similar local artifacts).
- Stop tracking generated files currently committed under `build_warnings/` (repo cleanup task).
- Apply warning policy consistently to app/tools/tests (not just `whacker_core`).
- Keep strict mode opt-in (`WHACKER_STRICT_WARNINGS`) to avoid blocking normal iteration.

Exit criteria:
- `git status` is not polluted by generated artifacts after normal local build.
- All targets compile under unified warning policy in non-strict mode.

## Phase B - Utility and Data-Path Dedup

- Consolidate shared helpers:
  - numeric clamp utility use (`sim::clampf` where appropriate),
  - one shared string trim helper for app serialization code.
- Remove repeated small helpers from app runtime/render files where not needed.
- Keep API-level behavior unchanged.

Exit criteria:
- duplicated helper implementations removed from app/progression save/menu paths.
- no behavior change in `ctest` baseline.

## Phase C - Runtime Decomposition (Highest Value)

- Split `app/app_runtime.cpp` into first-class units:
  - `app/runtime_loop.cpp` (frame loop only),
  - `app/runtime_input_dispatch.cpp`,
  - `app/runtime_update.cpp`,
  - `app/runtime_state_transitions.cpp`,
  - `app/runtime_audio_dispatch.cpp`.
- Centralize transition table/helpers so pause/forfeit/story routing is not distributed.
- Keep `run_app_loop()` as orchestrator, not logic sink.

Exit criteria:
- `app/app_runtime.cpp` reduced to a thin coordinator.
- transition logic tested with focused flow tests (see Phase E).

## Phase D - AI/Sim Boundary Hardening

- Introduce a canonical contact-outcome API used by planner/evaluator.
- Reduce duplicated shot projection math paths across `ai/planner.cpp` and `ai/evaluator.cpp`.
- Keep one place where contact->shot physics approximation is defined for AI.

Exit criteria:
- planner/evaluator share core projection model code.
- AI behavior stable in current playtests and style tools.

## Phase E - Story/Text/UI Coherence

- Create explicit text ownership rules:
  - menu labels in menu/story overlay modules,
  - narrative/dialogue only in `story_text`.
- Keep menu casing policy separate from dialogue casing policy.
- Ensure wrapping/scaling rules are centralized and reused between intro/scene overlays.

Exit criteria:
- text changes do not require hunting literals across runtime and render files.
- dialogue and menu casing policies are deterministic and documented.

## Phase F - Test Topology Upgrade

- Add focused tests for:
  - pause exit policy gates (intro ball gating, training stop, onboarding entry lock),
  - story transition sequences (Aya -> ClubIntro -> Benji -> CoachBrief),
  - save/load round-trip of new onboarding fields.
- Keep existing large `sim_smoke` but carve out critical deterministic regression tests by behavior area.

Exit criteria:
- transition regressions are caught by dedicated tests, not only manual playtest.

## 5) Recommended Sequence (Pragmatic)

1. Phase A
2. Phase C (runtime decomposition slice-by-slice)
3. Phase B (dedup while touching affected units)
4. Phase E (text coherence after runtime split)
5. Phase D (AI/sim boundary hardening)
6. Phase F (test expansion across all refactors)

Reasoning:
- Runtime decomposition and hygiene remove the largest current regression surface first.
- AI/sim hardening is safer once app flow churn is reduced.

## 6) Guardrails During Execution

- No behavior changes mixed into structural commits unless explicitly intended and tested.
- For each phase:
  - run build + test baseline,
  - make refactor slice,
  - rerun tests,
  - perform one manual smoke path for story/quick match.
- Avoid "big bang" rewrite; use vertical slices with stable checkpoints.

## 7) Immediate First Slice (Next Implementation Step)

- Start Phase A and the first C-slice:
  - build/repo hygiene cleanup,
  - extract pause/exit policy handling from `app/app_runtime.cpp` into dedicated runtime transition unit.
- This gives immediate maintainability gain with low gameplay risk.

