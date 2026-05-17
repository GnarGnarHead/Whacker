# STAGE2_LOCK.md

## Status
- Locked on: `2026-02-15`
- Amended on: `2026-02-23` (Season 1 structure + 3 skills + authored rivals)
- Purpose: canonical Stage 2 implementation contract
- Precedence: this document overrides ambiguous or conflicting details in `VISION2.md` and `theConversation.md`

## Credits
- Game Designer: `GNARGNARHEAD`
- Lead Developer: `Codex`

## Stage 2 Product Statement
`Whacker` Stage 2 is a deterministic Pong campaign where:
- training builds mechanical identity
- official matches build reputation
- curated text scenes react to identity + reputation + match events

The game remains mechanics-first and text-forward.

## Scope Lock (Frozen)
### In scope
- Deterministic Pong simulation remains core gameplay.
- Usage-based skill growth for the player character.
- Rivals are authored per match (fixed skill profiles; no progression/adaptation in Season 1 slice).
- Campaign loop with training blocks and official matches.
- Deterministic match/event tagging.
- Curated narrative card/scene selection driven by tags and reputation.
- Season 1 vertical slice with a small cast and constrained branching.

### Out of scope
- Runtime LLM-generated dialogue.
- Visual overhaul, character art, voice acting.
- Inventory, loot, broad RPG progression systems.
- Large-cast branching narrative expansion.
- Any system that bypasses fair mechanical constraints.

## Architecture Boundaries
- `sim/`: deterministic physics authority only.
- `ai/`: tactical decision systems under same mechanical rules as player.
- `progression/`: skills, identity signals, reputation state updates, tag generation.
- `campaign/`: season flow, match scheduling, rival routing hooks.
- `narrative/`: deterministic scene selection from authored content.
- `content/`: rival definitions, season graph, scene cards, tuning data.

No layer may directly violate deterministic match logic in `sim/`.

## Skills System (Reduced Set)
### Active skills (3 only)
1. `edge`
2. `power`
3. `spin_inject`

### Removed from Stage 2 slice
- `endurance` (deferred)
- `composure` (deferred)

### Skill domain and growth
- Range: `S in [0.0, 1.0]`
- Initial value: `0.0`
- No decay in Stage 2 slice
- Skill budget cap (sum of active skills): `<= 1.70`
- Growth rule:

```txt
S += usage_metric * exposure * growth_rate * (1 - S)
S = clamp(S, 0, 1)
```

### Mapping rule
Skills modify control surfaces and response mappings, not guaranteed outcomes.

## Fatigue and Pressure
- Deferred for the first Stage 2 slice.
- Not part of initial implementation lock.
- May be added in a later phase only after the vertical slice is validated.

## Identity, Reputation, and Tags
### System split (must remain separate)
- Identity: emergent mechanical style from skills + training behavior.
- Reputation: social/performance arc from official match outcomes.

### Reputation states (5 only)
1. `rising`
2. `falling`
3. `stagnant`
4. `surging`
5. `slumping`

### Tag budget caps
- Training tags: max `6`
- Official/performance tags: max `6`
- Reputation states: fixed `5` (above)

Tag vocabulary must stay within these caps for Season 1 slice.

## Rival Authoring Model (Season 1 Slice)
- Rivals do **not** run progression in Season 1 slice.
- Rival skill profiles are **authored per match** as fixed values:
  - `edge`
  - `power`
  - `spin_inject`
- Rival identity (technical/power/spin/balanced) is expressed through:
  - authored skill profile
  - authored `AiStyle`
  - deterministic tag-driven narrative reactions

Goal: match-to-match variety and personality without hidden learning/adaptation.

## Season 1 Narrative Spine (Locked)
- Core cast (Season 1 alpha) is defined in `STORY_CAST_LOCK.md`.
- Timescale note: “Weeks” are nominal slots (planning index), not a hard calendar constraint.
- Rival focus for Season 1:
  - major rival: `Jolo Marasigan`
  - minor rival: `Benji Santos`
- Inflection match (fixed):
  - major-rival rematch in club tournament semifinal
- Anchor beats (planning trace):
  - `Anchor A` club entry placement
  - `Anchor B` mid-arc conflict resolution match
  - `Anchor C` regional decider + quiet aftermath
- Branching style:
  - mostly linear structure
  - flavor/reactivity branches from identity + reputation + tags
  - limited hard branches only where necessary

Resolved Season 1 spine (acts + mainline slots) lives in `STAGE2_SEASON1_CONTENT.md`.

## Vertical Slice Contract
First end-to-end playable slice must include:
1. Training match flow
2. Skill growth update
3. Official match flow
4. Tag emission
5. Reputation state update
6. Curated scene selection + display

Flow target:

```txt
training -> progression update -> official match -> tags -> reputation -> scene
```

## Success Criteria (Stage 2 Slice)
- Different skill profiles produce visibly different match expression.
- Authored rival profiles produce visibly different shot style across matches.
- Same win/loss result can produce different scenes when identity/reputation differ.
- Season 1 slice is playable end-to-end without scope creep.
- Deterministic tests remain stable.

## Planning Guardrails
- Keep systems minimal and testable.
- Prefer data-driven content over code branching explosion.
- Expand only after slice validation through playtests.

## 48-Hour Execution Rule
- After implementation starts, no scope expansion for 48 hours.
- Only allow changes that fix blockers or contradictions to this lock.

## Planning Track (Next)
1. Define exact 3-skill usage metrics and mapping formulas.
2. Define capped tag schema (6 training + 6 official).
3. Define reputation state transition rules.
4. Define first 3 character profiles (coach, major rival, minor rival).
5. Define minimum scene-card schema and selection predicates.
6. Implement vertical slice in that order.
