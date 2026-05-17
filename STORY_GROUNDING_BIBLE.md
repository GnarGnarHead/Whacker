# Story Grounding Bible

## Scope
- Canonical grounding reference for story tone, setting assumptions, and social realism constraints.
- Does not duplicate graph architecture or cast profile values.

## Core Story Statement
`Whacker` story mode is a grounded Filipino high school sports drama where emotional conflict is expressed through table tennis play, not speeches. The arc is joining a club, meeting people, and slowly becoming a group.

## Locked Assumptions

### 1) Protagonist and Time Scale
- Player character is `16` (Grade 11 equivalent).
- Stage 2 vertical slice targets approximately `8 weeks` of school life (rough planning number, not a hard constraint).
- Arc shape is subtle: starts as "just something to do," ends with "this mattered more than I expected."

### 2) Setting Shape (Grounded, Not Tourist Postcard)
- Country: `Philippines`.
- Setting style: unnamed `mid-sized coastal city public high school` (urban-province middle ground).
- School team identity: `White Lions`.
- Environment cues are systemic: rain delays, commute pressure, crowded shared spaces, limited sports resources.

### 3) Tone Lock
- Grounded realism; no villains, no melodrama, no cartoon cruelty.
- Conflicts are small-scale but emotionally serious at age 16:
  - wanting to matter
  - fear of being average
  - rivalry without hatred
  - social embarrassment (`hiya`) and pride
- "Problems solved on the table" is mandatory narrative spine.

### 4) Narrative Presentation
- Dialogue is concise, contextual, and readable globally.
- Language approach: English-primary with light Taglish/Filipino cadence markers where context is clear.
- No cultural glossary dumps during core play; meaning should come from scene context.

### 5) Match-to-Story Constraint
- Emotional state must have a mechanical echo (style choice, shot selection, risk profile, rally behavior).
- Story reacts to:
  - skill/style expression
  - match outcomes and margins
  - training volume and consistency
- Losing remains a valid, respected branch.

### 6) Social Realism Guardrails
- Avoid stereotype performance (poverty tourism, token slang spam, one-note strict parents).
- Show class differences through behavior/logistics, not lectures.
- Adults are present but peripheral; peer dynamics remain central.

## Canonical Dependencies
- Cast and NPC profile values: `STORY_CAST_LOCK.md`
- Anchor role distribution: `STORY_ANCHOR_BEAT_MATRIX.md`
- Story flow architecture: `STORY_REACTIVITY_MODEL.md`

## Act Scaffold (1-3) with Match Flow Integration

### Act 1 - Entry and Belonging (Weeks 1-2)
- Narrative intent: establish social world and "do I belong here?"
- Match intent: baseline skill expression, early inconsistency, first style signals.
- Key beats:
  - Onboarding chain per `STORY_ONBOARDING_CONTRACT.md` (Aya first contact + two casual matches + Coach briefing + midweek `1967` bridge).
  - Name/identity lock.
  - Club placement match (`Anchor A`) as first post-onboarding mainline beat.
  - Early mainline match where result matters less than composure and effort.
- Social realism signals:
  - commute timing pressure
  - shared school spaces
  - minor sport invisibility

### Act 2 - Friction and Identity (Weeks 3-6)
- Narrative intent: player starts caring; peer frictions surface.
- Match intent: style polarization (power/technical/spin/blended identities become visible).
- Key beats:
  - Act 2 divergence is "which crew you gravitate toward" (see `STORY_REACTIVITY_MODEL.md` crew model).
  - Fractures are simple disagreements over behavior and small social frictions (not melodrama).
  - club tension over commitment and priorities
  - one fair, undeniable loss against a higher-level rival
  - training choices reshape both play and social perception
- Social realism signals:
  - exams and schedule compression
  - rain/weather disruptions
  - class and resource contrasts shown through daily logistics

### Act 3 - Respect and Continuation (Weeks 7-8)
- Narrative intent: maturity without grand speeches; relationships gain nuance.
- Match intent: demonstrate earned expression under pressure (not miracle outcomes).
- Key beats:
  - club-level high-stakes mainline run (school-scale, not national spectacle)
  - final major match can be win or loss, but must feel fair
  - ending beat emphasizes continuity ("practice again tomorrow" energy)
- Social realism signals:
  - recognition at school scale (assembly, peers, coach/family acknowledgment)
  - unresolved life pressures still exist, but player has changed

## Implementation Notes
- Keep scene writing short, specific, and tied to observed match tags.
- Use subculture flavor as character truth, not decoration.
- Maintain deterministic narrative selection from authored content.
- Expand cast only after the locked core cast is represented in prototype scenes.

## Done Criteria
- Setting assumptions are fixed enough to write scenes without re-litigating world basics.
- Every major NPC concept maps to both:
  - a social reality anchor
  - a gameplay style identity
- Act scaffold can be implemented using the current campaign loop.
