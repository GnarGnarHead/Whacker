# Temporary Refactor Plan: Duplicate System Removal

## Goal
Consolidate style/archetype definitions and AI contact execution into shared modules so app runtime and tooling run the same behavior path.

## Phase 1: Canonical Style Archetypes
- [x] Add `ai/style_archetype` module as single source of truth for:
  - style id + string conversion
  - planner preset
  - seed skills
  - style profile bias/ceilings
  - planner lambda tuning
- [x] Replace duplicated style tables in:
  - `app/main.cpp`
  - `tools/style_playtest.cpp`

## Phase 2: Shared AI Controller Path
- [x] Add `ai/controller` module with reusable per-paddle planning/execution state.
- [x] Move contact-plan -> intercept -> target/feedforward pipeline into shared code.
- [x] Use this shared controller in app runtime and style playtest tool.

## Phase 3: Remove Duplicate Execution Layers
- [x] Keep style intent in planner candidate generation/scoring.
- [x] Remove extra style-specific reshaping in runtime execution path.
- [x] Keep one flick execution channel (feedforward command).

## Phase 4: Contract Validation
- [x] Add/adjust tests for style signatures at realized output layer.
- [x] Build + run test suite.
- [x] Run style playtest spot checks for all archetypes.

## Exit Criteria
- No duplicated style profile/tuning tables across app/tooling.
- App and style_playtest share controller code.
- Build/test pass.
