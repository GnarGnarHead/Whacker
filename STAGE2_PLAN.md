# STAGE2_PLAN.md

## Planning Baseline
This plan is constrained by `STAGE2_LOCK.md`.
If any item conflicts with the lock, the lock wins.

## Phase 0 - Data Contracts First
### Goal
Define schemas before implementation to avoid refactors.

### Deliverables
- `progression` state structs:
  - `SkillState` (3 skills)
  - `IdentityState` (derived style signals)
  - `ReputationState` (5-state enum + counters)
- Tag enums capped to lock limits:
  - 6 training tags
  - 6 official tags
- Content schema for:
  - rival match spec (style + 3-skill profile)
  - scene card predicates

### Acceptance criteria
- All enums/structs compile.
- Serialization format is deterministic.
- Unit tests validate bounds and schema defaults.

## Phase 1 - Skill Metrics and Mapping
### Goal
Implement deterministic skill growth and physics mapping.

### Skill usage metrics (draft)
- `edge`: mean `abs(u)` over valid paddle contacts.
- `power`: mean `(1 - abs(u))` weighted by contact speed.
- `spin_inject`: normalized `abs(paddle_velocity_at_impact)`.

### Mapping hooks (draft)
- `edge` -> effective `theta_max` multiplier.
- `power` -> center-hit ramp bonus.
- `spin_inject` -> spin transfer coefficient and effective spin cap.

### Acceptance criteria
- Growth follows lock formula and stays in `[0,1]`.
- Zero usage produces near-zero growth.
- High usage produces diminishing returns.
- Deterministic replay yields identical skill deltas.

## Phase 2 - Rival Authoring and Routing (No Rival Progression)
### Goal
Rival variety and personality are authored per match without hidden learning/adaptation.

### Deliverables
- Rival match specs:
  - `AiStyle`
  - fixed skill profile (`edge`, `power`, `spin_inject`)
  - name/id wiring for scene predicates
- Season routing:
  - per-week training rival spec
  - per-week official rival spec

### Acceptance criteria
- Rival specs obey skill bounds and the skill budget cap.
- Weekly routing is deterministic and test-covered.
- No system writes back to rival skills after matches.

## Phase 3 - Reputation and Tag Pipeline
### Goal
Convert match outcomes into deterministic narrative signals.

### Deliverables
- Official match summary metrics:
  - margin, upset flag, streak direction, key event flags
- Reputation state transitions into:
  - `rising`, `falling`, `stagnant`, `surging`, `slumping`
- Tag emission:
  - training tags (6 max)
  - official tags (6 max)

### Acceptance criteria
- Same input match history always yields same reputation state.
- Tag count and vocabulary stay within caps.
- Transition tests cover all reputation states.

## Phase 4 - Narrative Card Engine (Curated)
### Goal
Select authored scenes based on identity + reputation + tags.

### Deliverables
- Scene card schema:
  - id, speaker, body, predicates, priority, cooldown
- Predicate evaluator for:
  - skill thresholds
  - reputation state
  - recent tags
  - rival id and match context
- Deterministic tie-breaker policy.

### Acceptance criteria
- Same state chooses same scene every run.
- Predicate conflicts resolve deterministically.
- Missing-card fallback scene always exists.

## Phase 5 - Season 1 Vertical Slice Integration
### Goal
Ship one end-to-end loop.

### Fixed spine
- Training block
- Official match
- Major-rival semifinal rematch inflection
- Reflection scene

### Acceptance criteria
- Playable loop from season start to reflection.
- At least two distinct identity paths produce different post-match scenes.
- At least two reputation trajectories produce different commentary tone.
- Deterministic integration test passes.

## Test Strategy
- Unit tests:
  - skill growth, mapping, bounds
  - reputation transitions
  - tag emission rules
  - scene predicate matching
- Simulation tests:
  - deterministic season replay
- Golden tests:
  - fixed seed season outputs stable tag + scene sequences

## Immediate Next Planning Outputs
1. `STAGE2_FORMULAS.md` (exact coefficients and normalization windows)
2. `STAGE2_TAGS.md` (final 6+6 vocabulary and trigger definitions)
3. `STAGE2_SEASON1_CONTENT.md` (coach/rivals and first scene-card set)
