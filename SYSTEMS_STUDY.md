# Systems Study: Style AI Pipeline (Whacker)

Date: 2026-02-15
Scope: Runtime style behavior from menu selection -> planner -> paddle control -> contact outcomes -> progression metrics.

## 1) End-to-End Flow (Current)
1. `app/main.cpp` menu selects `AiStyle` per side.
2. Runtime maps style to:
   - profile/skills (`make_style_profile`, `make_seed_skills`)
   - planner context (`PlannerStyleContext`) including `preset`.
3. `ai/planner.cpp` generates style-specific candidate sets and chooses `(contact_u, paddle_flick)`.
4. `app/main.cpp::ai_offensive_target_for_paddle(...)` applies additional style shaping to that decision, then:
   - computes `desired_center`
   - applies a flick offset to target center
   - applies feedforward velocity
5. `sim/physics.cpp` integrates paddle control law and collision response.

## 2) Verified Strengths
- Style selection is wired through to planner context (`preset` path exists).
- Planner has explicit style candidate spaces (`power/technical/spin/runner/balanced`).
- Spin/power mechanics are present in simulation (`speed_scalar_after_contact(...)`, spin injection by paddle velocity).

## 3) Core System Problems (Root Causes)

### A. Two competing style layers create unstable behavior
- Style is applied in planner *and again* in controller execution.
- Planner outputs are already style-biased; controller re-shapes the same intent again.
- Result: behavior can become over-constrained and incoherent near intercept.

Code:
- `ai/planner.cpp` style-specific candidate generation (`build_candidates(...)`).
- `app/main.cpp:792-808` style reshaping of `contact_u` and `flick_intent` after planner output.

### B. Flick intent is applied twice near contact
- Controller both:
  - shifts target center by `raw_flick_offset`, and
  - injects feedforward with `velocity_command`.
- These are two control channels pushing the same intent simultaneously.
- Under tight intercept windows this can destabilize tracking and cause misses.

Code:
- `app/main.cpp:826-833` (target offset + feedforward together).

### C. Replan cadence is too sparse for unstable trajectories
- Plan refresh happens primarily when `rally_hits` changes.
- Ball path changes continuously (spin curvature, wall interactions, opponent motion), but plan may stay stale too long.
- Stale `contact_u` + strong flick shaping near contact increases misses.

Code:
- `app/main.cpp:743` replan gate on `!has_plan || rally_hits changed`.

### D. Style influence is disabled in several common states
- If not incoming or intercept unreachable, style logic is mostly bypassed.
- This makes style identity visually disappear for large portions of play.

Code:
- `app/main.cpp:730-771` early returns.

### E. Validation harnesses are misaligned with runtime claim
- `tools/ai_duel.cpp` does not use style presets/context, so it cannot validate style archetypes.
- `tools/style_playtest.cpp` is better, but outputs focus/rating more than realized contact behavior (no direct style contract metrics).

### F. Tests verify planned intent, not realized contact behavior
- Current style test checks planner output bands, not actual contact outcomes after control + physics.
- Missing tests for realized `|u|`, paddle impact velocity, spin imparted, and movement duty cycle per style.

Code:
- `tests/sim_smoke.cpp::test_style_planner_signatures()`.

## 4) Why the User Experience Feels "No Change"
Even when planner outputs differ, the final visible behavior can converge because:
- controller clamps to reachable bounds,
- style can be bypassed on non-incoming/unreachable frames,
- duplicated control channels overcorrect and produce misses,
- no live telemetry confirms what style command was attempted vs achieved.

## 5) Repair Plan (Phased, Minimal Risk)

### Phase 1: Stabilize Execution Path (must do first)
1. Make planner the only style decision layer for shot intent.
2. Remove controller post-shaping of `contact_u`/`flick_intent` (except runner idle movement only).
3. Use one flick execution channel, not two:
   - keep feedforward OR target offset, not both.
4. Replan at short cadence when incoming (e.g., every 60 Hz tick or at timed intervals), not only per `rally_hits`.

Acceptance:
- No paddle miss regressions in 10k-step AI-vs-AI smoke run.
- Rally hit rate improves versus current baseline.

### Phase 2: Add Style Contracts (realized outcomes)
Add deterministic per-style metrics from *actual contacts*:
- Power: mean `|u| <= 0.30`, mean impact `|paddle_vy|` high.
- Technical: mean `|u| >= 0.70`.
- Spin: mean `|paddle_vy|` high + mean `|spin_after - spin_before|` high.
- Runner: movement duty cycle high even when ball not incoming.
- Balanced: metrics near middle band.

Acceptance:
- Contract tests pass across seeds.

### Phase 3: Unify Tooling
- Route app, playtest, and duel through one shared AI controller path.
- Make `ai_duel` style-aware so style regressions are easy to detect quickly.

## 6) Immediate Recommendation
Stop adding style heuristics until Phase 1 is complete. The current issue is architectural conflict in execution, not lack of style weights.

