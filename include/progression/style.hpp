#pragma once

#include "progression/skills.hpp"

namespace whacker::progression {

struct SkillWeights {
    float edge = 0.0f;
    float power = 0.0f;
    float spin_inject = 0.0f;
};

struct RivalStyleProfile {
    SkillWeights bias {1.0f, 1.0f, 1.0f};
    SkillWeights skill_floor {};
    SkillWeights skill_ceiling {1.0f, 1.0f, 1.0f};
    SkillWeights bias_floor {0.2f, 0.2f, 0.2f};
    SkillWeights bias_ceiling {3.0f, 3.0f, 3.0f};
    float adapt_gain = 0.18f;
};

void clamp_weights_non_negative(SkillWeights& weights);
void clamp_weights_unit(SkillWeights& weights);
float sum_weights(const SkillWeights& weights);
SkillWeights normalize_weights(const SkillWeights& weights);

void clamp_skills_to_profile(SkillState& skills, const RivalStyleProfile& profile);

SkillWeights compute_training_focus(const RivalStyleProfile& profile, const SkillState& skills);
SkillWeights compute_match_focus(const RivalStyleProfile& profile, const SkillState& skills, bool training_mode);

void apply_style_adaptation(
    RivalStyleProfile& profile,
    const SkillWeights& loss_signal);

}  // namespace whacker::progression
