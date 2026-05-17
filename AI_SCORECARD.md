# AI Scorecard

This project now treats AI tuning as a measurable system, not feel-only tweaking.

## Goal

Every AI change should answer:

1. Did match effectiveness improve?
2. Did style identity become more coherent?
3. Did we increase timeout dependence or planner cost?

## Tool

Use `style_playtest` as the single scorecard entry point.

```bash
./build/style_playtest \
  --training-matches 1 \
  --official-matches 12 \
  --win 5 \
  --steps 18000 \
  --samples 11 \
  --left-style maxed \
  --right-style balanced
```

## Scorecard Metrics

Printed per side at the end of `style_playtest`:

- `win_rate`: official match wins / official matches
- `point_share`: points_for / (points_for + points_against)
- `timeout_rate`: fraction of official matches that ended on step cap
- `ppm`: total points per minute of simulated match time
- `decisions_per_point`: planner decisions needed per point scored
- `style_fidelity`: distance between observed usage mix and style target mix
- `overall`: weighted composite for quick comparisons

Also printed:

- `*_target`: normalized style target weights (`edge`, `power`, `spin_inject`)
- `*_observed`: normalized usage weights measured in official matches

## Interpretation

- Rising `win_rate` with falling `style_fidelity` means stronger but less coherent identity.
- High `style_fidelity` with low `win_rate` means coherent but ineffective policy.
- High `timeout_rate` means the policy is not converting pressure into points.
- Rising `decisions_per_point` without performance gain means wasted planner complexity.

## Tuning Rule

Do not accept an AI change from one matchup sample.

Run at least:

1. 10+ official matches for the intended pairing
2. One mirrored run (swap left/right styles)
3. At least one control run against `balanced`

Keep the change only if effectiveness and coherence both improve, with no major timeout regression.
