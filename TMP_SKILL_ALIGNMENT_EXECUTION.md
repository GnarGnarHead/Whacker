# Temporary Plan: Skill-Physics Alignment (Execution First)

Date: 2026-02-16
Owner: Lead Developer
Status: Draft for implementation

## Goal
Align gameplay to this contract:

1. `SimulationConfig` values are hard maximums.
2. Per-player skills in `[0, 1]` scale down from those maximums.
3. Ball speed gain is contact-driven by the striker, not rally age.
4. Style picks intent; skills determine how well that intent executes.
5. P1 and P2 apply separate skill effects on their own contacts.

## Current Misalignment Summary

1. Power/speed still comes from `ramp_rate` contact ramp, not strictly player power execution.
   - `sim/physics.cpp`
   - `include/sim/math.hpp`
2. Skill effect scaling exists but is not wired into runtime sim.
   - `progression/skills.cpp` (`compute_skill_effect_scales`)
3. Collision/spin/paddle integration use global config only (not striker/defender skill execution).
   - `sim/collision.cpp`
   - `sim/spin.cpp`
   - `sim/physics.cpp`
4. AI evaluator/planner simulate with global config and must be updated to match runtime physics.
   - `ai/evaluator.cpp`
   - `ai/planner.cpp`

## Skill Semantics (Canonical for This Pass)

Use four execution meanings:

1. `runner` -> paddle speed/accel authority
2. `power` -> energy added on center-biased contact
3. `technical` -> angle authority from contact point
4. `spin` -> spin transfer from paddle velocity

Mapping from current stored fields:

1. `runner = paddle_control`
2. `power = power`
3. `technical = edge`
4. `spin = spin_inject`

Note:
- `spin_control` remains in progression/story for now, but is not part of this execution refactor.

## Step Study and Execution Plan

### Step 1: Add Per-Paddle Execution Context in Sim

Why:
- Runtime sim has no per-side execution scaling container today.

Changes:
1. Add a lightweight per-side execution struct in sim state (or simulation instance), e.g.:
   - `runner_scale`
   - `power_scale`
   - `technical_scale`
   - `spin_scale`
2. Add API for app layer to set left/right execution scales each frame.

Files:
- `include/sim/types.hpp`
- `include/sim/physics.hpp`
- `sim/physics.cpp`

Acceptance:
1. Left/right execution scales can differ at runtime.
2. No behavior change yet when all scales are `1.0`.

---

### Step 2: Wire Runtime Skills to Execution Scales

Why:
- App currently computes style/focus/plan, but never applies skill scaling to sim physics.

Changes:
1. Convert `RuntimeAiState.skills` to execution scales via canonical mapping.
2. Apply independently for left and right every update tick.
3. Keep human paddle support:
   - option A: humans use full `1.0` execution
   - option B: later expose human skill profile

Files:
- `app/main.cpp`
- `progression/skills.*` (only helper mapping if needed)

Acceptance:
1. P1 and P2 skill values visibly produce different paddle/shot capability.
2. Swapping only one side's style/skills affects only that side.

---

### Step 3: Rework Live Physics to Player-Driven Power

Why:
- Current power path is still `ramp_rate` based and not directly a striker skill execution model.

Changes:
1. Replace `speed_scalar_after_contact(...)` usage in runtime contact resolution with striker-driven energy injection.
2. Keep center contact as power multiplier, but gate by striker `power_scale`.
3. Apply `technical_scale` to effective launch angle.
4. Apply `spin_scale` to spin transfer (`k_take * paddle_velocity` path).
5. Apply `runner_scale` to paddle speed and accel limits.
6. Keep explicit speed clamp for stability/determinism.

Files:
- `include/sim/math.hpp`
- `sim/physics.cpp`
- `sim/collision.cpp`
- `sim/spin.cpp`
- `include/sim/config.hpp`
- `config/default.json`

Acceptance:
1. No implicit rally-age speed growth remains.
2. Center + high paddle velocity + high power increases speed more than low power.
3. Same flick with low vs high spin yields different spin delta.
4. Same contact `u` with low vs high technical yields different launch angle.

---

### Step 4: Make AI Simulation Match Runtime Physics Exactly

Why:
- Planner/evaluator use physics approximations with global config; mismatch causes incoherent choices.

Changes:
1. Pass striker/opponent execution scales into evaluator/planner simulation helpers.
2. Replace direct global-config formulas with shared math used by runtime.
3. Ensure fast-scoring path and full path both use same execution scaling.

Files:
- `ai/evaluator.cpp`
- `ai/planner.cpp`
- `include/ai/evaluator.hpp`
- `include/ai/planner.hpp`

Acceptance:
1. AI predictions match realized outcomes materially better (contact and spin trend consistency).
2. No regressions in planner bounds and finite score tests.

---

### Step 5: Retune Style Candidate Grids After Physics Alignment

Why:
- Style behavior should be tuned only after execution model is trustworthy.

Changes:
1. Keep style as intent:
   - power prefers center + aggressive forward flick
   - technical prefers edge contact
   - spin prefers high signed flick near contact
   - runner keeps high movement duty cycle
   - balanced blends
2. Retune candidate sets and penalties with new execution physics.

Files:
- `ai/planner.cpp`
- `ai/controller.cpp`
- optional AI weights in `config/default.json`

Acceptance:
1. Style swaps produce clear, consistent behavioral signatures.
2. Balanced no longer collapses into extreme edge/flick behavior.

## Test Plan (Must Land With Refactor)

1. Add/replace smoke tests for skill-execution contract:
   - low vs high `power` speed gain at center contact
   - low vs high `spin` spin delta at equal paddle velocity
   - low vs high `technical` launch angle authority
   - low vs high `runner` reachability envelope
   - left/right independent skill application
2. Update old ramp-based assertions that assume `1 + ramp_rate`.
3. Keep determinism replay test passing.

Primary file:
- `tests/sim_smoke.cpp`

## Rollout Order

1. Step 1 + Step 2 (plumbing, no behavior change with unity scales)
2. Step 3 (runtime physics shift)
3. Step 4 (AI parity)
4. Step 5 (style retune)
5. Test pass and playtest sweep

## Out of Scope (This Pass)

1. Full progression schema rename/migration (`edge` -> `technical`, etc.)
2. Campaign narrative tuning
3. UI overhaul

