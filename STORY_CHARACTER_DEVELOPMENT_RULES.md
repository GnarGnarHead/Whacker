# Story Character Development Rules (Locked)

## Canonical Role
- This is the canonical standard for developing story characters in `Whacker`.
- Purpose is grounding and internal consistency, not immediate player-facing copy.
- If a bio/writeup conflicts with this doc, this doc wins.

## Scope
- Applies to all narrative characters used in story mode.
- Applies to bios, long-form grounding writeups, dialogue drafting, and scene behavior.
- Does not replace `STORY_CAST_LOCK.md` for cast roster/profile ownership.

## Core Principle
Characters are built from lived causality, not trait labels.

We do not work backward from a finished profile.
We discover the character by writing concrete moments and extracting truth from what they do.

Every meaningful character quality must answer:
1. Where did this come from?
2. What did it cost?
3. How does it show up under pressure?

## Voice Standard For Grounding Writeups
Use restrained close-third realism in past tense.

Requirements:
- Concrete first: behavior, routine, setting details, social context.
- Emotion through action and choice timing, not abstract explanation.
- No melodrama, no trope shorthand, no anime archetype language.
- No menu-style "pick a trait" design framing in final character text.

## Anti-Caricature Rules
- Never reduce a character to one skill style or one social role.
- Every character must include at least one internal contradiction.
- Cultural context must be specific and lived, not decorative.
- Family/class pressure should appear through logistics and expectations, not lectures.
- Adults should shape constraints, not replace peer-centered story energy.

## Two-Layer Character Artifacts (Required)
Each character has two files:
1. `bio` (canonical reference)
2. `writeup` (long-form grounding narrative)

## Story Trilogy Dossier Method (Required)
Every character writeup must follow the same three-story development sequence:
1. `Grounding Story 01` - early childhood snapshot (formation seed)
2. `Grounding Story 02` - early-adolescent pressure event (identity turn)
3. `Grounding Story 03` - family/public social event (values under scrutiny)

Process rule:
- Do not skip to scene dialogue before all three stories exist.
- Keep story scale human and specific; no epic biography summaries.
- Do not write present-age beliefs/worldview sections before stories 01/02/03 are complete.
- Do not pre-commit to a final personality and force scenes to match it.
- If a drafted scene reveals a better truth, update the profile to match the scene evidence.

Post-trilogy extraction:
- After stories 01/02/03 are complete, extract and add:
  - `<Character> at Present Age - Beliefs, Values, Place in the World`

### Bio Purpose
- Stable, implementation-ready facts and constraints.
- Used for scene authoring, node logic, and consistency checks.

### Writeup Purpose
- Internal narrative truth and depth.
- Used to avoid flat characterization and tonal drift.

## Bio Requirements
Each bio must include:
- canonical identity facts
- role in story function
- social presentation and private pressure
- pivotal life moments timeline
- what they care about (written as prose, not bullet labels)
- voice notes (dialogue cadence, word choice, emotional restraint)
- match behavior cues (how emotional state appears in play)
- relationship posture toward core cast

## Writeup Requirements
Each writeup must include:
- three short stories following the Story Trilogy Dossier Method
- coherent mini-story structure in each story (scene-like, causal, grounded)
- one unresolved internal tension that can drive future scenes
- one clear "public self vs private self" contrast
- post-trilogy present-age beliefs/values section (added only after stories are drafted)

## Canonical Ordering For New Character Development
1. Draft Grounding Story 01.
2. Draft Grounding Story 02.
3. Draft Grounding Story 03.
4. Extract present-age beliefs/values section from the completed stories.
5. Treat extracted beliefs as discovered evidence, not authored targets.
6. If later scenes contradict early assumptions, revise assumptions and keep causal continuity.
7. Extract stable facts into bio.
8. Map match behavior cues from personality truth.
9. Add dialogue voice constraints.
10. Cross-check with anti-caricature rules.
11. Only then draft scene lines.

## Quality Gate (Must Pass)
Before a character is considered usable:
- Their choices feel plausible without exposition.
- Their play behavior can be read as personality, not random AI variation.
- Their dialogue can be recognized without name tags.
- Their profile does not conflict with `STORY_CAST_LOCK.md`.

## Ownership and File Layout
- Rules: `STORY_CHARACTER_DEVELOPMENT_RULES.md`
- Character assets: `story/characters/`
- Bios: `story/characters/bios/`
- Writeups: `story/characters/writeups/`
- Index: `story/characters/INDEX.md`

## Dependencies
- Product constraints: `STORY_MODE_PRINCIPLES.md`
- Tone and social realism: `STORY_GROUNDING_BIBLE.md`
- Roster and NPC profile ownership: `STORY_CAST_LOCK.md`
- Anchor role constraints: `STORY_ANCHOR_BEAT_MATRIX.md`
