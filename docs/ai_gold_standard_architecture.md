# AI Gold-Standard Architecture

## Scope
This document captures the Milestone 1 AI decomposition for deterministic opponent planning.

## Public Entry Points
- `app/ai_core.hpp`
  - `AiDecision plan_ai_decision(...)`
  - `void apply_ai_decision(...)`
  - `uint64_t compute_ai_state_signature(const sim::RallyState&, bool)`
  - `uint64_t compute_ai_state_signature(const sim::Simulation&, bool)`

## Module Map
- `app/ai_core.cpp`
  - Planner coordinator only.
  - Stages: actor-frame transform -> perception/prediction -> candidate generation -> candidate scoring/selection -> decision finalize.
- `app/ai_frame.*`
  - Actor-frame mirror conversion and actor->world decision mapping.
- `app/ai_seed.*`
  - Deterministic keyed-noise and signature helpers.
- `app/ai_profile.*`
  - Competence curve, style mix, intent weights, style recover lane.
- `app/ai_predict.*`
  - Ball-forward intercept prediction in actor frame.
- `app/ai_reachability.*`
  - Paddle actuator-limited reach envelope.
- `app/ai_candidates.*`
  - Candidate lattice build, feasibility prune, term scoring, unique deterministic candidate IDs.
- `app/ai_score.*`
  - Candidate truncation, deterministic tie-break, winner selection, miss-risk level mapping.
- `app/ai_execute.*`
  - Decision->paddle command mapping and strike gating.
- `app/ai_decision_templates.*`
  - Recovery and safe-intercept fallback decision construction.

## Determinism Guarantees
- Candidate generation is deterministic for the same input state.
- Candidate IDs are unique and stable within each planning pass.
- Random-like variation uses keyed deterministic noise (`ai_seed`).
- Side parity is enforced by actor-frame planning and mirrored decision mapping.

## Planner Data Flow
1. Build actor-frame simulation (`ai_frame`).
2. Build style/capability profile (`ai_profile`).
3. Predict intercept (`ai_predict`).
4. Compute reach envelope (`ai_reachability`).
5. Generate and score candidate lattice (`ai_candidates`).
6. Select winner deterministically (`ai_score`).
7. Finalize decision fields and apply ambient adjustments (`ai_core`).
8. Apply commands to paddle (`ai_execute`).

## Scoring Terms (Normalized)
Each candidate carries:
- `make_term`
- `quality_term`
- `style_term`
- `risk_term`
- `motion_term`
- `score` and `cheap_score`

`ai_score` performs deterministic sort/truncate and winner pick with score epsilon and ID tie-break.

## Runtime Integration Notes
- Runtime replan logic in `app/play_control.cpp` currently uses rally-signature path for stability.
- Simulation-signature API exists for controlled future integration.

## Test Matrix (Milestone 1)
- Existing contracts:
  - `ai_core_smoke`
  - `ai_style_fidelity_smoke`
  - `ai_realized_spin_smoke`
  - `ai_competence_ladder_smoke`
  - `ai_determinism_trace_smoke`
  - `ai_side_parity_smoke`
  - `ai_canonical_symmetry_smoke`
- New contract:
  - `ai_candidate_id_smoke`

## Acceptance Snapshot (Milestone 1)
- `build`: all tests passing.
- `strict /tmp/whacker_warnings8`: all tests passing.
- warning scan: no `warning:` hits.
