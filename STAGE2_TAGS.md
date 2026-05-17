# STAGE2_TAGS.md

## Purpose
Deterministic tag schema for the Stage 2 vertical slice, constrained by:
- `STAGE2_LOCK.md` (cap: 6 training tags + 6 official tags)
- `STAGE2_FORMULAS.md` (usage metrics and derived values)

This document defines:
- fixed tag vocabulary
- trigger thresholds
- per-match/block selection and tie-break rules

## 1) Vocabulary (Locked)
## 1.1 Training tags (6)
1. `GRINDER`
2. `SLACKER`
3. `TECHNICIAN`
4. `POWER_SPECIALIST`
5. `SPIN_SPECIALIST`
6. `RECKLESS`

## 1.2 Official/performance tags (6)
1. `UPSET_WIN`
2. `EXPECTED_LOSS`
3. `COMEBACK_WIN`
4. `CHOKE_LOSS`
5. `DOMINANT_VICTORY`
6. `NARROW_DEFEAT`

No additional tags are allowed in Season 1 slice.

## 2) Inputs and Derived Metrics
All values are deterministic from match summaries.

## 2.1 Training block inputs
For one training block (between official matches):
- `training_match_count`
- `training_minutes_total`
- `edge_usage_mean`
- `power_usage_mean`
- `spin_inject_usage_mean`
- `clean_contact_rate_mean`
- `high_edge_contact_rate_mean`
  - where `high_edge_contact_rate = count(abs(u) >= 0.75) / max(contacts, 1)`

## 2.2 Official match inputs
- `won` (bool)
- `score_for`, `score_against`
- `peak_deficit` (max opponent lead during match)
- `peak_lead` (max own lead during match)
- `expected_win_prob` from pre-match reputation rating model

Derived:
- `margin = abs(score_for - score_against)`
- `down_two_then_win = (peak_deficit >= 2) and won`
- `up_two_then_loss = (peak_lead >= 2) and (not won)`

## 3) Training Tag Triggers
Evaluate all six conditions, then resolve conflicts with Section 5 rules.

## 3.1 `GRINDER`
Trigger if:

```txt
training_match_count >= 8
OR
training_minutes_total >= 25
```

## 3.2 `SLACKER`
Trigger if:

```txt
training_match_count <= 2
AND
training_minutes_total <= 8
```

## 3.3 `TECHNICIAN`
Trigger if:

```txt
edge_usage_mean >= 0.62
AND
clean_contact_rate_mean >= 0.52
```

## 3.4 `POWER_SPECIALIST`
Trigger if:

```txt
power_usage_mean >= 0.62
AND
spin_inject_usage_mean <= 0.58
```

## 3.5 `SPIN_SPECIALIST`
Trigger if:

```txt
spin_inject_usage_mean >= 0.64
AND
power_usage_mean <= 0.60
```

## 3.6 `RECKLESS`
Trigger if:

```txt
high_edge_contact_rate_mean >= 0.48
AND
clean_contact_rate_mean <= 0.42
```

## 4) Official Tag Triggers
Evaluate all six conditions, then resolve with Section 5 rules.

## 4.1 `UPSET_WIN`
Trigger if:

```txt
won
AND
expected_win_prob <= 0.35
```

## 4.2 `EXPECTED_LOSS`
Trigger if:

```txt
not won
AND
expected_win_prob <= 0.45
```

## 4.3 `COMEBACK_WIN`
Trigger if:

```txt
down_two_then_win
```

## 4.4 `CHOKE_LOSS`
Trigger if:

```txt
up_two_then_loss
```

## 4.5 `DOMINANT_VICTORY`
Trigger if:

```txt
won
AND
margin >= 3
AND
score_against <= 2
```

## 4.6 `NARROW_DEFEAT`
Trigger if:

```txt
not won
AND
margin == 1
```

## 5) Selection and Conflict Rules
Rules are deterministic and required.

## 5.1 Training tag selection
- Emit at most `2` training tags per block.
- Priority order:
  1. `GRINDER` / `SLACKER` (work ethic axis)
  2. `TECHNICIAN` / `POWER_SPECIALIST` / `SPIN_SPECIALIST` (playstyle axis)
  3. `RECKLESS` (behavior overlay)

Conflict resolution:
- `GRINDER` and `SLACKER` are mutually exclusive.
  - if both trigger (boundary noise), choose by `training_match_count` distance:
    - prefer `GRINDER` if `training_match_count >= 5`
    - else `SLACKER`
- `TECHNICIAN`, `POWER_SPECIALIST`, `SPIN_SPECIALIST` are mutually exclusive.
  - choose the highest margin over threshold:

```txt
tech_score  = min(edge_usage_mean - 0.62, clean_contact_rate_mean - 0.52)
power_score = min(power_usage_mean - 0.62, 0.58 - spin_inject_usage_mean)
spin_score  = min(spin_inject_usage_mean - 0.64, 0.60 - power_usage_mean)
```

Pick max of `tech_score`, `power_score`, `spin_score` among triggered candidates.
- `RECKLESS` can co-exist with one work-ethic tag or one style tag, but never creates more than 2 total tags.

## 5.2 Official tag selection
- Emit at most `3` official tags per match.
- Priority tiers:
  1. `COMEBACK_WIN` / `CHOKE_LOSS`
  2. `UPSET_WIN` / `EXPECTED_LOSS`
  3. `DOMINANT_VICTORY` / `NARROW_DEFEAT`

Conflict resolution:
- `COMEBACK_WIN` and `CHOKE_LOSS` are mutually exclusive by outcome.
- `UPSET_WIN` and `EXPECTED_LOSS` are mutually exclusive by outcome.
- `DOMINANT_VICTORY` and `NARROW_DEFEAT` are mutually exclusive by outcome.
- If multiple tags in same tier trigger (should not happen by definitions), tie-break by lexical tag name for deterministic fallback.

## 6) Data Contract (Implementation-Facing)
## 6.1 Enums
```txt
enum class TrainingTag {
  GRINDER,
  SLACKER,
  TECHNICIAN,
  POWER_SPECIALIST,
  SPIN_SPECIALIST,
  RECKLESS
}

enum class OfficialTag {
  UPSET_WIN,
  EXPECTED_LOSS,
  COMEBACK_WIN,
  CHOKE_LOSS,
  DOMINANT_VICTORY,
  NARROW_DEFEAT
}
```

## 6.2 Emission record
```txt
struct EmittedTags {
  std::vector<TrainingTag> training_tags; // size <= 2
  std::vector<OfficialTag> official_tags; // size <= 3
}
```

Stored in deterministic order by Section 5 priority.

## 7) Validation Targets
1. Tag vocabulary count is exactly `6 + 6`.
2. Re-running the same match/block data emits identical tags and ordering.
3. Training conflict resolution never emits both `GRINDER` and `SLACKER`.
4. Style conflict resolution emits at most one of:
   - `TECHNICIAN`
   - `POWER_SPECIALIST`
   - `SPIN_SPECIALIST`
5. Official selection never emits more than 3 tags.
