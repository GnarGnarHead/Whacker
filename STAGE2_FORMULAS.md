# STAGE2_FORMULAS.md

## Purpose
Exact numeric formulas for the Stage 2 vertical slice, constrained by `STAGE2_LOCK.md`.

This document defines:
- 3-skill usage metrics (`edge`, `power`, `spin_inject`)
- derived contact quality metrics used for tags (`clean_contact_rate`, `high_edge_contact_rate`)
- growth update coefficients + skill budget cap
- runtime mapping from skills -> paddle execution scales

## 1) Skill Set (Locked)
Skills (all `S in [0, 1]`):
1. `edge`
2. `power`
3. `spin_inject`

No decay in Stage 2 slice.

Skill budget cap:
- `edge + power + spin_inject <= 1.70`

## 2) Data Collected Per Match (Deterministic)
For each paddle contact `j = 1..M`:
- `u_j`: normalized contact offset in `[-1, 1]`
- `pv_j`: paddle vertical velocity at impact
- `speed_j`: ball speed at impact

From baseline sim config:
- `ball_base_speed`
- `paddle_max_speed`

## 3) Per-Contact Samples
All samples clamp into `[0, 1]` before aggregation.

Constants:
- `u_clean_threshold = 0.30`
- `pv_clean_threshold = 0.35 * paddle_max_speed`
- `speed_norm_window = 1.5 * ball_base_speed`
- `epsilon = 1e-6`

Per contact:

```txt
abs_u_j = clamp(abs(u_j), 0, 1)
center_j = 1 - abs_u_j

speed_norm_j = clamp((speed_j - ball_base_speed) / max(speed_norm_window, epsilon), 0, 1)
power_sample_j = center_j * (0.55 + 0.45 * speed_norm_j)

spin_inject_sample_j = clamp(abs(pv_j) / max(paddle_max_speed, epsilon), 0, 1)

high_edge_j = (abs_u_j >= 0.75)

clean_j = (abs_u_j <= u_clean_threshold) AND (abs(pv_j) <= pv_clean_threshold)
```

## 4) Usage Metrics (Normalized [0, 1])
If `M > 0`:

```txt
edge_usage        = mean_j(abs_u_j)
power_usage       = mean_j(power_sample_j)
spin_inject_usage = mean_j(spin_inject_sample_j)

exposure = M
clean_contact_rate = count(clean_j) / M
high_edge_contact_rate = count(high_edge_j) / M
```

Else all are `0`.

Notes:
- `exposure` is a non-negative multiplier (contact count) used in growth.
- `clean_contact_rate` and `high_edge_contact_rate` are tag inputs, not skills.

## 5) Skill Growth Update
For each skill `S` with usage metric `usage`:

```txt
S_next = clamp(S + (k_growth * usage * exposure * (1 - S)), 0, 1)
```

Default growth coefficients (vertical-slice tuning defaults):
- `k_edge_growth = 0.00120`
- `k_power_growth = 0.00110`
- `k_spin_inject_growth = 0.00125`

After updating all three skills, enforce budget cap:

```txt
sum = edge + power + spin_inject
if sum > 1.70:
  scale = 1.70 / sum
  edge *= scale
  power *= scale
  spin_inject *= scale
```

## 6) Skill-to-Physics Mapping (Runtime)
Skills are converted to paddle execution scales:
- `power_scale = 0.10 + 0.90 * power`
- `technical_scale = edge`
- `spin_scale = spin_inject`

These scales drive deterministic sim behavior:
- `power_scale` affects speed scalar gains on contact.
- `technical_scale` affects angle authority.
- `spin_scale` affects spin injection and transfer.

Implementation references:
- `app/play_control.cpp`
- `include/sim/math.hpp`
- `sim/physics.cpp`
- `progression/skills.cpp`

## 7) Determinism Invariants
- No randomness in usage, growth, or mapping.
- All values clamped to valid ranges.
- Budget cap enforced after growth.

## 8) Validation Targets
1. `usage` and derived rates stay in `[0, 1]`.
2. Re-running the same deterministic match yields identical usage metrics and skill deltas.
3. Skill budget cap is never violated.
