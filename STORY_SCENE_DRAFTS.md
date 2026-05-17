# Story Scene Drafts

## Scope
- Working prose drafts for authored scenes.
- Keep this file focused on dialogue and scene flow.
- Do not duplicate cast stats or system rules here; reference source docs.
- Onboarding sequence must follow `STORY_ONBOARDING_CONTRACT.md`.

## Onboarding Slice Draft Set

## Scene S01 - Early Arrival (Aya First Contact)
- Type: `scene_node`
- Position: immediately after intro acceptance
- Function: establish one-on-one human contact before full club social load
- Tone target: restrained, natural, no exposition dump

### Base Scene
Aya: "You're early."

Aya: "I'm Aya. You're {PLAYER_NAME}, right?"

Aya: "If you're up for it, we can warm up before everyone piles in."

### Variant Flavor Hooks
- If `intro_power_lean`: Aya -> "You hit heavy. Keep it controlled."
- If `intro_technical_lean`: Aya -> "You see angles quickly."
- If `intro_spin_lean`: Aya -> "That spin was loud."
- If `intro_tough_loss`: Aya -> "You stayed in the points. That's what matters."

### Exit (Convergent)
- Starts `M01` Aya friendly match.

## Match M01 - Aya Friendly Warm-Up
- Type: `match_node`
- Position: directly after `S01`
- Stakes: casual (`non-ranked`, `non-official`)
- Purpose: first relationship beat expressed through actual play
- Write hooks:
  - style read reinforcement
  - early composure/performance tone
  - Aya affinity seed

## Scene S02 - Club Floor Introduction
- Type: `scene_node`
- Position: after `M01`
- Function: Aya introduces player to coach and core club personalities
- Tone target: quick social texture, no speeches

### Base Scene
Aya: "Coach, this is {PLAYER_NAME}. We got a warm-up in."

Coach Reyes: "Reyes. You're {PLAYER_NAME}, right?"

Coach Reyes: "Welcome. Lets see what you can do."

Issa: "You're fine. First Friday is always weird."

Tix: "You improvise too much, but it's workable."

Jolo: "If you push pace, prove it twice."

Coach Reyes: "Benji. Table two."

Coach Reyes: "Careful, he puts spin on everything."

### Reactive Insert Lines (choose by flags)
Use 1 performance line + 1 style line as flavor swaps.

Performance variants:
- `intro_controlled_win`: Coach -> "You controlled points better than most first days."
- `intro_close_game`: Kai -> "Close rallies. You stayed in it."
- `intro_tough_loss`: Kai -> "You stayed in it. Keep that."

Style variants:
- `intro_power_lean`: Jolo -> "Center contact had bite."
- `intro_technical_lean`: Tix -> "Edge reads were sharp."
- `intro_spin_lean`: Issa -> "That spin jumps."
- `intro_balanced_lean`: Aya -> "You mix options well."

### Branch Flavor Hooks (single line swaps)
- If `low_commitment_seed`: Jolo line -> "If you're in, be in. Don't coast."
- If `high_commitment_seed`: Coach line -> "Saw you putting in reps. Keep that pace."
- If `high_tix_friction`: Tix line -> "I can already tell you won't listen to percentages."
- If `aya_affinity_open`: Aya line -> "If you want practice sets after class, ask me."

### Exit
- Starts `M02` Benji entry game.

## Match M02 - Benji Entry Game (Casual Benchmark)
- Type: `match_node`
- Position: after `S02`
- Stakes: casual (`non-ranked`, `non-official`)
- Purpose: establish club-context read on player style and consistency
- Write hooks:
  - club-first-impression summary
  - friction/respect seeds
  - readiness seed for placement

## Scene S03 - Coach Welcome and Training Brief
- Type: `scene_node`
- Position: after `M02`
- Function: convergent onboarding close and system unlock handoff

### Base Scene
Coach Reyes: "[Match-specific compliment about your game.]"

Coach Reyes: "You're in. Nothing fancy. Just keep showing up."

Coach Reyes: "Training tables are open any time. Play as many practice matches as you want."

Coach Reyes: "Use those reps to sharpen your game."

Coach Reyes: "When you're ready, take the next club placement match."

### Exit (Convergent)
- Onboarding complete.
- Unlock story hub actions:
  - `Next Match` -> `Club Placement Match`
  - `Training` -> available
