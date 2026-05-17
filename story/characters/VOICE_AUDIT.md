# Voice Audit

## Scope
- Date: `2026-02-17`
- Purpose: verify character voice separation and vibe consistency without rewriting dialogue/story text.
- Source surfaces reviewed: `app/story_intro.cpp`, `app/story_scene.cpp`, `STORY_SCENE_DRAFTS.md`, character bios/writeups.

## Summary
- `pass`: Kai, Tix, Jolo, Issa, Benji (on paper), Coach Reyes (on paper), Aya (on paper).
- `partial`: runtime onboarding lines are not fully aligned with latest cast-vibe locks for Benji and some scene flavor variants.
- No dialogue or prose lines were rewritten in this audit pass.

## Character Voice Fingerprints
- Kai: approachable bridge voice, low-ego phrasing, practical encouragement.
- Aya: restrained and composed, specific observation, no theatrical language.
- Tix: procedural and evaluative, compact technical framing, low social cushioning.
- Jolo: pace-and-proof language, challenge-forward and physical.
- Benji: polite surface with undermining edge, opportunistic framing, deniable digs.
- Issa: socially warm and tension-lowering, practical support language.
- Coach Reyes: short directive lines, standards-first, no sermonizing.

## Runtime Alignment Check
- `app/story_intro.cpp`: Kai intro voice is coherent with role and distinct from others.
- `app/story_scene.cpp`: core onboarding sequence is structurally correct and character-assigned correctly.
- `app/story_scene.cpp`: Benji runtime line currently reads neutral (`TABLE'S OPEN.`), while current cast lock expects low-empathy opportunist edge.
- `app/story_scene.cpp`: performance/style callouts still skew toward generic onboarding voice rather than per-character contrast in all branches.

## Collision Risks
- Aya and Issa can converge into similar "supportive" tone if line-level vocabulary is not separated by cadence.
- Kai and Coach can converge into generic encouragement if imperative tone and sentence length are not kept distinct.
- Benji currently under-signals his unique vibe in runtime onboarding compared with locked profile.

## Cleanup Actions Completed (No Rewrites)
- Normalized character index status semantics in `story/characters/INDEX.md`.
- Aligned character workspace process order in `story/characters/README.md`.
- Logged this audit for approval-first rewrite planning.

## Rewrite Candidates Requiring Approval
- Benji onboarding base line in `app/story_scene.cpp`.
- Benji performance flavor line in `app/story_scene.cpp`.
- Optional tightening of Issa vs Aya support-tone separation in `app/story_scene.cpp`.
- Optional Story Hub naming cleanup (`Official Match`, `Advance Week`) to match current narrative framing in `app/app_runtime.cpp`.
