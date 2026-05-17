# Misalignment Study (AI Style System)

Date: 2026-02-15

## Scope
Investigate why style selection appears non-functional in live play even though style presets exist.

Systems inspected:
- Planner style presets and candidate generation (`ai/planner.cpp`)
- Shared controller execution path (`ai/controller.cpp`)
- App runtime wiring (`app/main.cpp`)
- Playtest wiring (`tools/style_playtest.cpp`)
- Contact physics coupling (`sim/physics.cpp`, `sim/collision.cpp`)

## Diagnostic Method
A local diagnostics harness was run against current `build/libwhacker_core.a` to compare:
- Planned contact intent (`plan.contact_u`)
- Planned flick intent (`plan.paddle_flick`)
- Realized contact location at hit
- Realized paddle velocity at hit
- Spin after hit

Case shape:
- Left style under test vs right balanced
- 28,000 sim steps per style

## Key Results

### 1) Planner intent is style-distinct
Planned values are different per style:
- `Power`: `mean_abs_plan_u ~= 0.280` (center-biased)
- `Technical`: `mean_abs_plan_u ~= 1.000` (edge-biased)
- `Spin`: `mean_abs_plan_u ~= 0.320` (center-biased)

This confirms presets are reaching planner.

### 2) Execution does not realize planner intent (major mismatch)
Plan vs realized contact error is high for most styles:
- `Power`: `mean_abs_u_error ~= 0.670`
- `Spin`: `mean_abs_u_error ~= 0.680`
- `Balanced`: `mean_abs_u_error ~= 0.571`
- `Runner`: `mean_abs_u_error ~= 0.445`
- only `Technical` remains close (`~0.111`)

Observed contradiction:
- `Power` planned center (`0.280`) but realizes near-edge (`~0.900`).

### 3) Flick intent is almost not transferred to paddle velocity at impact
Planned flick magnitudes are large, but hit-time paddle velocity is tiny:
- `Power`: `mean_abs_plan_flick ~= 260`, `mean_abs_hit_vy ~= 3.2`
- `Spin`: `260` vs `13.8`
- `Balanced`: `260` vs `14.8`
- `Technical`: `98` vs `0.06`

Gap (`|planned_flick| - |hit_vy|`) remains massive in all styles.

### 4) Balanced style is not behaviorally balanced
`Balanced` plans extreme values:
- `mean_abs_plan_u ~= 1.000`
- `mean_abs_plan_flick ~= 260`

So balanced currently behaves as aggressive edge+max-flick, not mixed style.

## Root Causes (Ranked)

1. Controller execution model cannot express flick intent reliably
- Current controller uses a single feedforward velocity channel.
- Position-tracking term and acceleration limits dominate near contact.
- With no contact-offset actuation channel, flick commands mostly wash out before impact.
- File: `ai/controller.cpp`

2. Planner candidate sets are too extreme for controller feasibility
- Power/spin candidate sets are high-flick only (near max speed values).
- Planner picks values that are hard to realize under current control dynamics.
- File: `ai/planner.cpp`

3. Balanced candidate/evaluator interaction collapses to extremes
- Balanced path plus current evaluator weights prefers high-edge/high-flick outcomes.
- File: `ai/planner.cpp` + config weights in `config/default.json`

4. Style bypass paths still erase style in some rally states
- Non-incoming/unreachable intercept logic returns generic/passive behavior.
- This contributes to style flattening over time.
- File: `ai/controller.cpp`

5. Tooling still has extra controller duplication elsewhere
- `ai_duel`/`ai_sweep` still run independent controller code paths (not style controller).
- This can mask or contradict runtime behavior during diagnostics.
- Files: `tools/ai_duel.cpp`, `tools/ai_sweep.cpp`

## Misalignment Matrix

Expected vs observed:
- Power
  - Expected: center contact + strong impact velocity
  - Observed: near-edge contacts + very low hit velocity
- Spin
  - Expected: center contact + high spin injection
  - Observed: moderate contact match, but weak flick-to-velocity transfer
- Technical
  - Expected: edge contact + controlled flick
  - Observed: edge contact mostly works; flick still under-realized
- Balanced
  - Expected: mixed behavior
  - Observed: extreme planner outputs

## Conclusion
The primary failure is not style selection routing; it is style execution fidelity.

Style intent exists in planner output, but controller/physics coupling does not convert that intent into actual paddle-at-contact behavior.

## Recommended Next Fix Sequence
1. Re-introduce a second, bounded contact-phase actuation channel (target offset near impact) in addition to feedforward velocity.
2. Make planner candidate amplitudes style-specific but controller-feasible (reduce hard max-flick reliance).
3. Retune balanced candidate/evaluator path to avoid extreme default outputs.
4. Add realized-contact contract tests (plan-vs-hit error and flick-vs-hit-vy thresholds) as hard gates.
5. Migrate remaining tools (`ai_duel`, `ai_sweep`) to shared controller for consistent diagnostics.
