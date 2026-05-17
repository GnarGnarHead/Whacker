# Style Strength Study (Power / Spin / Technical)

Date: 2026-02-16

## Goal
Evaluate why style mirrors felt weak, then find concrete physics presets that increase:
- power rally acceleration
- spin tactical influence
- technical shot threat

without destabilizing the deterministic control loop.

## Method
- Built a focused probe: `/tmp/style_strength_probe`
- Ran mirror matchups with the shared runtime controller path:
  - `power vs power`
  - `spin vs spin`
  - `technical vs technical`
- Measured:
  - `mean_speed` at paddle contact
  - `peak_speed`
  - `high_speed_rate` (`speed_scalar >= 1.2`)
  - `mean_spin`
  - `mean_ball_vy`
  - `mean_abs_u`

Baseline and top candidates were compared at `--steps 60000`.

## Baseline (Current Default)
Config:
- `tau_speed=5.0`
- `power_contact_boost=10.0`
- `theta_max_rad=1.05`
- `k_curve=60`
- `k_spin_rebound=14`
- `power_to_spin_coupling=0.85`
- `power_to_technical_coupling=0.55`
- `power_energy_spin_drag=0.40`
- `power_energy_technical_drag=0.24`

Results (60000 steps):
- `power`: `mean_speed=1.1366`, `peak_speed=1.1808`, `high_speed_rate=0.0000`
- `spin`: `mean_speed=1.0748`, `mean_spin=2.3268`, `high_speed_rate=0.0000`
- `technical`: `mean_speed=1.0006`, `mean_ball_vy=167.6099`, `mean_abs_u=0.9877`

Interpretation:
- Power and spin do not sustain high-speed contact pressure.
- Technical generates angle (`ball_vy`) but little extra rally lethality.

## Candidate Results

### Candidate S2 (Selected as Aggressive)
Config:
- `tau_speed=9.0`
- `power_contact_boost=16.0`
- `theta_max_rad=1.18`
- `k_curve=84`
- `k_spin_rebound=18`
- `power_to_spin_coupling=1.05`
- `power_to_technical_coupling=0.72`
- `power_energy_spin_drag=0.30`
- `power_energy_technical_drag=0.18`

Results (60000 steps):
- `power`: `mean_speed=1.3002`, `peak_speed=1.3728`, `high_speed_rate=0.9741`
- `spin`: `mean_speed=1.2700`, `mean_spin=2.4732`, `high_speed_rate=0.9388`
- `technical`: `mean_speed=1.0002`, `mean_ball_vy=184.2572`, `mean_abs_u=0.9996`

Interpretation:
- Large gain in sustained power and spin pressure.
- Technical keeps strong edge identity and sharper flight angles.

### Candidate S4 (Selected as Technical-lean)
Config:
- `tau_speed=8.0`
- `power_contact_boost=14.0`
- `theta_max_rad=1.22`
- `k_curve=95`
- `k_spin_rebound=20`
- `power_to_spin_coupling=1.10`
- `power_to_technical_coupling=0.80`
- `power_energy_spin_drag=0.28`
- `power_energy_technical_drag=0.15`

Results (60000 steps):
- `power`: `mean_speed=1.3082`, `peak_speed=1.4306`, `high_speed_rate=0.7632`
- `spin`: `mean_speed=1.1975`, `mean_spin=2.4197`, `high_speed_rate=0.8387`
- `technical`: `mean_speed=1.0002`, `mean_ball_vy=188.6137`, `mean_abs_u=0.9991`

Interpretation:
- Slightly lower spin speed than S2, but stronger technical angle pressure.

### Candidate S3 (Extreme Showcase)
Config:
- `tau_speed=11.0`
- `power_contact_boost=20.0`
- `theta_max_rad=1.24`
- `k_curve=100`
- `k_spin_rebound=22`
- `power_to_spin_coupling=1.20`
- `power_to_technical_coupling=0.85`
- `power_energy_spin_drag=0.24`
- `power_energy_technical_drag=0.14`

Results (30000 steps):
- `power`: `mean_speed=1.8695`, `peak_speed=3.5893`, `very_high_speed_rate=0.6974`
- `spin`: `mean_speed=1.4887`, `mean_spin=2.3636`, `very_high_speed_rate=0.5517`

Interpretation:
- Delivers the strongest style pressure.
- Best treated as a showcase/extreme preset, not baseline default.

## Output Presets Added
- `config/presets/style_strength_balanced.json`
- `config/presets/style_strength_aggressive.json`
- `config/presets/style_strength_extreme.json`

## Recommended Usage
- Start from `style_strength_balanced` for day-to-day playtests.
- Use `style_strength_aggressive` to validate deeper tactical expression quickly.
- Use `style_strength_extreme` only when stress-testing high-energy rallies.
