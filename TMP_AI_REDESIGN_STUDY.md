# AI Redesign Study (Behavior-First)

Date: 2026-02-19
Status: Draft for review before implementation changes

## Purpose
Define a full redesign of runtime AI behavior that is:
- mechanically expressive (style directly affects contact physics),
- clearly skill-scaled by total points,
- deterministic,
- stable in motion (no spring/wobble artifacts),
- testable with objective gates.

This document is intentionally specific about what the AI should do each decision cycle.

## Non-Negotiables (Locked)
1. Style directly affects paddle/ball contact physics.
2. Total skill budget cap remains `1.70`.
3. Total points represent ability; distribution represents style.
4. Style is not difficulty.
5. AI and player share the same physics and collision model.

## Observed Failure Modes To Eliminate
1. Frame-to-frame target jitter masquerading as weakness.
2. Near-identical gameplay between weak and elite tiers.
3. Difficulty tied to unstable servo behavior instead of tactical quality.
4. Lack of explicit shot placement and opponent-position prediction.
5. Tuning by constants without objective acceptance gates.

## Design Space: Mechanism Options

### Option A: Continuous per-frame steering with noise
- Strengths: simple to wire.
- Weaknesses: tends to produce wobble and fake-looking incompetence.
- Decision: reject.

### Option B: Tick-based tactical decisions + stable motor controller
- Strengths: clean reaction-delay model, deterministic, expressive.
- Weaknesses: requires deliberate decision-state bookkeeping.
- Decision: recommend.

### Option C: State-machine only (recover/track/strike templates)
- Strengths: readable and robust.
- Weaknesses: can become rigid and repetitive.
- Decision: useful as scaffolding but insufficient alone.

### Option D: Heavy rollout/MPC each frame
- Strengths: can be highly effective.
- Weaknesses: expensive, harder to tune for broad skill ladder.
- Decision: use bounded rollout only at decision ticks.

## Recommended Architecture (Hybrid)
Use Option B with a light state machine and bounded shot rollout:

1. Decision Layer (tick-based)
- Run full planning every `N` frames (reaction cadence).
- Cache and commit command between decisions.

2. Perception Layer
- Estimate intercept and time-to-contact.
- Estimate opponent reachable band at shot arrival time.
- Inject deterministic, bounded perception error based on ability.

3. Tactical Layer
- Generate shot candidates (contact point + flick intent + style usage).
- Simulate candidate outcomes with actual game physics.
- Score candidates with placement, pressure, safety, and style terms.

4. Motor Layer
- Critically damped target follower.
- No per-frame random jitter input.
- Execution error applied at decision level, not actuator level.

## What The AI Should Actually Do (Per Decision Tick)
1. Compute competence:
- `c = clamp(total_points / 1.7, 0..1)`.

2. Determine cadence:
- decision interval `N(c)` frames.
- low `c` means slower updates; high `c` means fast updates.

3. Perceive incoming ball:
- estimate `t_contact`, `y_contact`.
- apply deterministic bounded errors to both estimates:
  - larger errors for low `c`, smaller for high `c`.

4. Build candidates:
- candidate tuple: `(contact_u, flick, style_commit)`.
- candidates must remain physically legal and executable.

5. Predict opponent response window:
- for each candidate, estimate arrival time on opponent side.
- compute opponent reachable interval `[y_min, y_max]` using opponent kinematic limits and reaction lag.

6. Score each candidate:
- `placement_score`: rewards landing outside or near boundary of opponent reachable interval.
- `pressure_score`: rewards reduced opponent response time and unfavorable ball states.
- `safety_score`: penalizes self-risk and poor recovery states.
- `style_score`: rewards candidate alignment with current style distribution.

7. Select and commit:
- choose max score candidate.
- cache resulting command until next decision tick.

8. Execute with stable motor:
- motor follows commanded target with fixed stable dynamics.
- no "wiggle" noise in actuator command.

## Shot Placement and Opponent Prediction (Explicit)
This is mandatory in the redesign.

### Opponent Reach Prediction
At candidate arrival time `t_arrive`:
1. Start with opponent current `(y, vy)`.
2. Apply reaction delay window before accel response starts.
3. Apply max accel and max speed bounds over remaining time.
4. Produce reachable interval `[y_min, y_max]`.

### Placement Value
For candidate landing `y_land`:
- high value when `y_land` is outside `[y_min, y_max]`.
- medium value when near interval edge.
- low value when centered in reachable band.

### Why This Matters
Without this step, AI only "hits hard" instead of "hits smart."

## Strength vs Style Separation

### Strength (total points) controls
1. Decision cadence (`N` frames).
2. Perception accuracy (`y`/`t` error bounds).
3. Candidate budget (planner breadth/depth).
4. Execution precision (contact/flick consistency).

### Style (edge/power/spin distribution) controls
1. Candidate preference in scoring.
2. Contact physics outcomes:
- `edge`: deflection angle profile from contact offset.
- `power`: post-contact pace/energy transfer.
- `spin`: spin injection/retention and curve exploitation.

## How It Should Work
1. `0.36` AI can rally, but makes slower and less accurate tactical decisions.
2. `1.70` AI anticipates lanes and places shots near/outside opponent reach.
3. Same total with different style distributions produces visibly different shot identities.
4. Paddle motion stays smooth and stable at all levels.

## How It Should Not Work
1. Weakness generated by shaking/wobbling paddles.
2. Style implemented only as labels with no contact-physics effect.
3. Per-frame random target perturbation as a "difficulty" lever.
4. Equal totals that do not produce roughly symmetric outcomes.
5. Strong totals that only increase speed but not placement quality.

## Probe Matrix (Multi-Perspective)

### Perspective 1: Player Experience
Probe:
- Can players read why they lost points?
- Does weak AI look novice (late/poor choices) rather than broken control?
Pass condition:
- losses feel attributable to tactical mistakes, not physics oddities.

### Perspective 2: Game Design
Probe:
- Does style produce distinct rally signatures at equal total?
Pass condition:
- measurable differences in contact offset, outgoing speed, and spin profiles.

### Perspective 3: Systems Engineering
Probe:
- Is behavior deterministic and reproducible?
Pass condition:
- identical seeds and inputs produce identical outcomes.

### Perspective 4: Performance
Probe:
- Does decision-tick planning stay within frame budget?
Pass condition:
- no frame spikes; bounded candidate counts per skill tier.

### Perspective 5: QA/Regression
Probe:
- Are acceptance metrics stable across seeds and side-swaps?
Pass condition:
- monotonic strength ladder and symmetric equal-total outcomes within tolerance.

## Objective Acceptance Gates
1. Monotonic strength curve:
- for increasing totals, aggregate win rate should not decrease across ladder tiers.

2. Equal-total parity:
- mirrored equal-total matchups converge toward ~50/50 over sufficient games.

3. Placement efficacy:
- higher totals should show increasing out-of-band placement against same opponent model.

4. Style fidelity at equal total:
- style-specific contact/shot metrics remain distinguishable.

5. Motion stability:
- no high-frequency sign-flip chatter in target commands near lock-on.

## Instrumentation Required
1. Per-decision log record:
- competence,
- decision tick interval,
- perceived intercept,
- chosen candidate,
- opponent reachable band,
- score breakdown.

2. Per-contact telemetry:
- contact `u`,
- outgoing speed scalar,
- spin delta,
- shot landing position.

3. Ladder summary output:
- win rate,
- point share,
- timeout rate,
- style-fidelity metrics,
- placement efficacy metrics.

## Implementation Boundaries (To Avoid Another Drift Cycle)
1. Build as `AI v2` parallel path first.
2. No in-place tuning churn to old behavior path during v2 bring-up.
3. Runtime switch only after gates pass.
4. Keep style-physics code as source of truth; AI picks intents only.

## Open Questions To Lock Before Coding
1. What exact novice baseline is acceptable (minimum average rally depth)?
2. What point-share target for `0.36 vs 1.70` feels right?
3. How much nonzero elite error is desirable for human-like variance?
4. Do we prefer more aggressive or more rally-heavy default pacing at equal totals?

## Scrutiny Findings (Red-Team Review)

### Critical
1. Acceptance gates are not numerically lockable yet.
- Current phrasing (`sufficient games`, `within tolerance`) can cause endless tuning drift.
- Required fix: lock sample size, confidence/tolerance bands, and timeout ceilings.

2. Deterministic error model is underspecified.
- \"Deterministic bounded error\" is not enough without a strict sampling key.
- Required fix: define counter-based sampling key exactly:
  - `(match_seed, rally_index, side, decision_index, channel)`.

3. Opponent reach prediction is too abstract.
- Kinematic interval language does not define how spin/curve/wall interactions are accounted for.
- Required fix: either:
  - use bounded forward integration for opponent reachability, or
  - explicitly declare kinematic approximation limits and fallback rules.

4. Style-collapse risk is unresolved.
- `style_score` can be dominated by `placement/pressure` at high competence, collapsing distinct styles.
- Required fix: lock a style-weight floor or style-commitment quota under high-pressure states.

### High
5. Tie/timeout degeneracy is not bounded.
- No explicit upper bound on ties/timeouts in evaluation gates.
- Required fix: add max tie-rate and max timeout-rate gates by matchup tier.

6. Migration/rollback control is missing.
- `AI v2` parallel path is stated, but no explicit runtime flag and rollback path are defined.
- Required fix: add an implementation contract:
  - `AI_V2` feature flag,
  - A/B harness output parity checks,
  - one-command rollback switch.

### Medium
7. Performance budget is not quantified.
- \"No frame spikes\" is non-testable without explicit thresholds.
- Required fix: lock per-tick planner timing budgets (median/p95/p99).

8. State coverage is implied but not explicit.
- Recover/track/strike behavior is mentioned, but serve/reset and non-incoming handling are not explicitly contracted.
- Required fix: add state transition table with guard conditions and outputs.

## Proposed Hard Gates (Initial Values, Editable)
1. Ladder totals: `{0.00, 0.20, 0.45, 0.75, 1.00, 1.35, 1.70}`.
2. Per pair evaluation: `>= 80` games, mirrored sides enabled.
3. Equal-total parity gate: win rate in `[45%, 55%]`.
4. Monotonic ladder gate: stronger total win rate `>= 58%` for adjacent tiers, `>= 70%` for gaps `>= 0.35`.
5. Timeout gate: `<= 20%` at equal totals, `<= 10%` for large-gap tiers.
6. Motion stability gate: target command sign-flip rate near lock-on below locked threshold (define from baseline telemetry pass).
7. Style fidelity gate: equal-total cross-style runs must preserve measurable separation in:
- mean `|u|`,
- outgoing speed scalar,
- spin delta.

## Recommended Next Step
1. Approve or edit this spec.
2. Freeze acceptance numbers.
3. Implement `AI v2` in isolated path with telemetry.
4. Evaluate with ladder probes before runtime switch.
