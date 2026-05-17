# AI Hitch Study (Static + Targeted Runtime Sampling)

Date: 2026-02-18
Scope: identify what actually causes the gameplay hitch under high-bounce rallies.
Method: static call-path analysis + reproducible `style_playtest` timing samples.

## Symptom

- Reported: hitch/lag spikes during long/high-bounce AI rallies.
- Context: most visible in strong-AI matchups (`maxed`, high-bounce tactical exchanges).

## Repro Timing Signal

Runs on current tree (`./build/style_playtest`, same `--steps 12000`, `--official-matches 2`, `--win 3`):

- `balanced vs balanced`:
  - wall time: `~3.26s`
  - decisions/point: `~279` / `~308`
  - timeout_rate: `1.000`
- `technical vs technical`:
  - wall time: `~2.61s`
  - decisions/point: `~304` / `~323`
  - timeout_rate: `1.000`
- `maxed vs balanced`:
  - wall time: `~7.13s`
  - decisions/point: `~223` / `~367`
  - timeout_rate: `1.000`
- `maxed vs maxed`:
  - wall time: `~12.7s`
  - decisions/point: `~785` / `~491`
  - timeout_rate: `1.000`

Interpretation: runtime cost explodes specifically with stronger/high-bounce planning paths.

## Primary Root Cause

### 1) Unbounded self-intercept simulation inside the fast planner

- `ai/planner.cpp:950` calls `predict_intercept(state, config, left_ai)` (no perception model).
- In `ai/intercept.cpp`, that path has:
  - `kMaxSteps = 4800` at 240 Hz (`~20s`) (`ai/intercept.cpp:182`)
  - no time horizon clamp when perception is null (`ai/intercept.cpp:183-186`)
  - full per-step physics + bounce handling in loop (`ai/intercept.cpp:191-202`)

Why this hurts:

- High-bounce trajectories can take a long simulated time before crossing the paddle plane.
- This runs synchronously in gameplay on the main thread, during AI target update.

### 2) Reachability solve scales linearly with that horizon

- After intercept prediction, planner computes self reachability with intercept horizon:
  - `ai/planner.cpp:956-960`
- `solve_paddle_reachability` integrates at fixed dt over full horizon:
  - `ai/intercept.cpp:122-128`

If intercept horizon inflates, this adds another long loop per plan decision.

## Secondary Amplifiers

### A) High replan cadence on strong styles

- Maxed timing aggressively reduces cadence/commitment:
  - `ai/controller.cpp:128-131`
  - effective cadence floor is very small (`~0.06s` scale).
- Planning window can reopen frequently:
  - `ai/controller.cpp:327-329`

So expensive planning happens often.

### B) Maxed planner workload is intentionally larger

- Dense maxed candidate grid:
  - `17 x 13` contacts/flicks (`ai/planner.cpp:245-252`)
- Larger refine beam for maxed:
  - `refine_limit ... maxed ? 8 : kOwnShotRefineBeam` (`ai/planner.cpp:999`)
- Rollout bounce depth for maxed:
  - `rollout_bounces = 7` (`ai/planner.cpp:636`)

Each replan is heavier even before the unbounded intercept issue.

### C) Main loop has no fixed-step budget cap per rendered frame

- App loop drains full accumulator each frame:
  - `while (accumulator >= whacker::sim::kFixedDt)` (`app/app_runtime.cpp:735`)
- No max-substeps cap exists in that loop.

When one update is expensive, the frame tries to catch up immediately, producing visible hitch bursts.

## Important Clarification

- Wall-spin reversal predates defender-horizon logic by commit chronology.
- The current hitch driver is not "wall-spin was added after defender horizon"; it is the current AI planner/intercept cost structure in combination with long trajectories.

## Additional Quality Findings (Non-hitch but high-standard concerns)

1. Physics logic duplication risk:
   - Similar contact/shot equations are implemented across runtime sim and AI evaluator/planner projections.
   - Files involved: `sim/physics.cpp`, `sim/collision.cpp`, `ai/evaluator.cpp`, `ai/planner.cpp`.
   - Risk: drift between "what AI predicts" and "what sim executes".

2. Timeout prevalence is extreme in playtest harness:
   - all sampled runs above reported `timeout_rate=1.000`.
   - This can mask true tactical effectiveness and confound AI tuning.

3. Planner memory churn:
   - per-plan temporary vectors (`candidates`, `scored`) rebuilt repeatedly (`ai/planner.cpp:948`, `ai/planner.cpp:965`).
   - Not the primary root cause, but it adds avoidable overhead.

## Remediation Order (for correctness + performance)

1. Bound fast-planner self-intercept horizon in `choose_contact_plan_fast` path.
2. Bound self reachability horizon used for pre-contact feasibility scoring.
3. Add per-frame max fixed-step budget to app loop to avoid catch-up hitch bursts.
4. Then tune cadence/workload knobs (maxed cadence, refine beam, rollout bounces) from measured budgets.
5. Consolidate shared shot/contact math into one authoritative utility path to reduce prediction drift.

## Assumption Audit (Static Only)

This section validates each remediation point as proven/partially-proven/ruled-out from code inspection only.

### Point 1: Bound fast-planner self-intercept horizon

Status: **Proven**

- `choose_contact_plan_fast` always computes self intercept once per plan (`ai/planner.cpp:950`).
- That call uses `predict_intercept(..., perception=nullptr)` and therefore gets no horizon cap (`ai/intercept.cpp:183-186`).
- The loop budget is `4800` fixed steps (`ai/intercept.cpp:182`), i.e. up to ~20 seconds simulated.
- Because `offensive_target_for_paddle` is called from the gameplay fixed-step loop (`app/app_runtime.cpp:965`, `app/play_control.cpp:153`, `app/play_control.cpp:165`), this work is on the main thread.

Conclusion: no assumption left here; this path is structurally unbounded relative to gameplay frame budget.

### Point 2: Bound self reachability horizon

Status: **Proven**

- After self intercept is predicted, planner calls `solve_paddle_reachability` with `self_intercept.time_to_intercept` (`ai/planner.cpp:956-960`).
- Reachability integrates over full horizon with fixed-dt loop (`ai/intercept.cpp:122-128`).
- No explicit clamp is applied before passing that horizon.

Conclusion: this cost scales directly with intercept horizon and is a deterministic secondary loop per plan.

### Point 3: Add per-frame max fixed-step budget

Status: **Partially Proven**

- Main loop drains `accumulator` with `while (accumulator >= kFixedDt)` and has no max-substep cap (`app/app_runtime.cpp:735`).
- Therefore, expensive updates can trigger catch-up bursts and visible hitching.

What static analysis cannot prove alone:

- the exact hitch magnitude on a given machine.

Conclusion: code structure supports hitch amplification; this is not ruled out and is likely contributory, but not sufficient alone to explain all latency.

### Point 4: Tune cadence/workload knobs after bounds

Status: **Proven as amplifier, not sole root**

- Maxed cadence/commitment are more aggressive than other styles (`ai/controller.cpp:128-131`).
- Maxed candidate and refinement workload is higher (`ai/planner.cpp:245-252`, `ai/planner.cpp:999`).
- Maxed own-shot rollout bounce depth is higher (`ai/planner.cpp:636`).

What is ruled out:

- Deep-search mode as a hitch source in normal gameplay. Gameplay path passes `deep_search=false` (`app/play_control.cpp:45`) and planner fast-path is used (`ai/planner.cpp:1059-1061`).

Conclusion: these knobs materially increase cost once baseline loops are bounded; they should be tuned second, not first.

### Point 5: Consolidate shared shot/contact math

Status: **Proven quality risk, not hitch root**

- Runtime sim and AI evaluator/planner implement overlapping contact/shot equations in separate code paths:
  - runtime: `sim/physics.cpp`, `sim/collision.cpp`
  - AI projection: `ai/evaluator.cpp`, `ai/planner.cpp`
- This is enough to create model drift risk.

What is ruled out:

- This duplication being the direct cause of the hitch spike itself.

Conclusion: keep as high-priority correctness/maintainability task, but treat as separate from performance root cause.

## Extra Issues Found During Static Pass

1. Planner allocates temporary vectors per replan (`ai/planner.cpp:948`, `ai/planner.cpp:965`).  
2. Timeout-heavy playtest defaults can hide tactical deltas (`tools/style_playtest.cpp:569-591`).  
3. Guard path only applies when perception is horizon-limited and ETA threshold is met (`ai/controller.cpp:313-325`), so it cannot bound the fast planner's unbounded self-intercept path.

## Implemented So Far

1. Bounded fast self-intercept horizon in fast planner path (`ai/planner.cpp`).  
2. Removed duplicate self-reachability integration by reusing intercept reachability (`ai/planner.cpp`).  
3. Bounded fast rollout bonus flight horizon (`ai/planner.cpp`, `ai/evaluator.cpp`, `include/ai/evaluator.hpp`).  
4. Added accumulator budget cap to avoid catch-up spirals (`app/app_runtime.cpp`).  
5. Added regression test for flight-horizon cap behavior (`tests/sim_smoke.cpp`).  

## Remaining High-Standard Work (Planned)

1. Add lightweight in-game/perf counters behind dev toggle (planning cost, worst-step time, plan calls per second).  
2. Reduce planner allocation churn (reuse buffers or stack-backed small candidate sets for fast mode).  
3. Separate "match quality" tuning from "performance" tuning:
   - performance target: stable frame pacing under maxed high-bounce rallies
   - gameplay target: reduce timeout-heavy outcomes without reintroducing hitching  
4. Consolidate duplicate shot/contact projection math between runtime and AI projection paths to reduce drift risk.  

This document now includes both analysis and implemented remediation notes.
