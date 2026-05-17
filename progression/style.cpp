#include "progression/style.hpp"

#include <algorithm>

namespace whacker::progression {

namespace {

float clamp01(const float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

float clamp_non_negative(const float value) {
    return std::max(0.0f, value);
}

float clamp_range(const float value, const float lo, const float hi) {
    return std::clamp(value, lo, hi);
}

}  // namespace

void clamp_weights_non_negative(SkillWeights& weights) {
    weights.edge = clamp_non_negative(weights.edge);
    weights.power = clamp_non_negative(weights.power);
    weights.spin_inject = clamp_non_negative(weights.spin_inject);
}

void clamp_weights_unit(SkillWeights& weights) {
    weights.edge = clamp01(weights.edge);
    weights.power = clamp01(weights.power);
    weights.spin_inject = clamp01(weights.spin_inject);
}

float sum_weights(const SkillWeights& weights) {
    return std::max(
        0.0f,
        weights.edge + weights.power + weights.spin_inject);
}

SkillWeights normalize_weights(const SkillWeights& input) {
    SkillWeights weights = input;
    clamp_weights_non_negative(weights);
    const float sum = sum_weights(weights);
    if (sum <= 1.0e-6f) {
        return SkillWeights {1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f};
    }

    const float inv = 1.0f / sum;
    weights.edge *= inv;
    weights.power *= inv;
    weights.spin_inject *= inv;
    return weights;
}

void clamp_skills_to_profile(SkillState& skills, const RivalStyleProfile& profile) {
    clamp_skills(skills);
    skills.edge = clamp_range(skills.edge, clamp01(profile.skill_floor.edge), clamp01(profile.skill_ceiling.edge));
    skills.power = clamp_range(skills.power, clamp01(profile.skill_floor.power), clamp01(profile.skill_ceiling.power));
    skills.spin_inject = clamp_range(
        skills.spin_inject,
        clamp01(profile.skill_floor.spin_inject),
        clamp01(profile.skill_ceiling.spin_inject));
}

SkillWeights compute_training_focus(const RivalStyleProfile& profile, const SkillState& skills_in) {
    SkillState skills = skills_in;
    clamp_skills_to_profile(skills, profile);

    SkillWeights drive {};
    drive.edge = clamp_non_negative(profile.bias.edge) * std::max(0.0f, clamp01(profile.skill_ceiling.edge) - skills.edge);
    drive.power =
        clamp_non_negative(profile.bias.power) * std::max(0.0f, clamp01(profile.skill_ceiling.power) - skills.power);
    drive.spin_inject = clamp_non_negative(profile.bias.spin_inject) *
        std::max(0.0f, clamp01(profile.skill_ceiling.spin_inject) - skills.spin_inject);

    return normalize_weights(drive);
}

SkillWeights compute_match_focus(const RivalStyleProfile& profile, const SkillState& skills, const bool training_mode) {
    if (training_mode) {
        return compute_training_focus(profile, skills);
    }

    const SkillWeights identity = normalize_weights(profile.bias);
    const SkillWeights growth = compute_training_focus(profile, skills);
    SkillWeights mixed {};
    mixed.edge = (0.88f * identity.edge) + (0.12f * growth.edge);
    mixed.power = (0.88f * identity.power) + (0.12f * growth.power);
    mixed.spin_inject = (0.88f * identity.spin_inject) + (0.12f * growth.spin_inject);
    return normalize_weights(mixed);
}

void apply_style_adaptation(RivalStyleProfile& profile, const SkillWeights& loss_signal_in) {
    SkillWeights loss_signal = loss_signal_in;
    clamp_weights_unit(loss_signal);
    const float adapt_gain = std::clamp(profile.adapt_gain, 0.0f, 1.0f);

    profile.bias.edge = ((1.0f - adapt_gain) * profile.bias.edge) + (adapt_gain * loss_signal.edge);
    profile.bias.power = ((1.0f - adapt_gain) * profile.bias.power) + (adapt_gain * loss_signal.power);
    profile.bias.spin_inject = ((1.0f - adapt_gain) * profile.bias.spin_inject) + (adapt_gain * loss_signal.spin_inject);

    profile.bias.edge = clamp_range(
        profile.bias.edge,
        clamp_non_negative(profile.bias_floor.edge),
        std::max(clamp_non_negative(profile.bias_floor.edge), clamp_non_negative(profile.bias_ceiling.edge)));
    profile.bias.power = clamp_range(
        profile.bias.power,
        clamp_non_negative(profile.bias_floor.power),
        std::max(clamp_non_negative(profile.bias_floor.power), clamp_non_negative(profile.bias_ceiling.power)));
    profile.bias.spin_inject = clamp_range(
        profile.bias.spin_inject,
        clamp_non_negative(profile.bias_floor.spin_inject),
        std::max(clamp_non_negative(profile.bias_floor.spin_inject), clamp_non_negative(profile.bias_ceiling.spin_inject)));
}

}  // namespace whacker::progression
