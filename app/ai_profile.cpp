#include "ai_profile.hpp"

namespace whacker::app::ai_internal {

float competence_from_skills(const whacker::progression::SkillState& skills) {
    const float sum = clampf(
        skills.edge + skills.power + skills.spin_inject,
        0.0f,
        whacker::progression::kSkillBudgetCap);
    return clamp01f(sum / whacker::progression::kSkillBudgetCap);
}

AiCapabilityProfile capability_profile_for(const float competence) {
    const float c = clamp01f(competence);
    const float curve = std::pow(c, 0.90f);
    return AiCapabilityProfile {
        .reaction_lag_s = lerpf(0.15f, 0.010f, curve),
        .target_quantization_step = lerpf(48.0f, 1.2f, curve),
        .intercept_jitter_amplitude = lerpf(38.0f, 2.4f, curve),
        .speed_scale = lerpf(0.28f, 0.84f, curve),
        .accel_scale = lerpf(0.26f, 0.82f, curve),
        .makeability_scale = lerpf(0.09f, 0.85f, curve),
    };
}

StyleMix fallback_mix_from_style(const AiStyle style) {
    switch (style) {
        case AiStyle::Power:
            return StyleMix {.power = 0.70f, .technical = 0.20f, .spin = 0.10f};
        case AiStyle::Technical:
            return StyleMix {.power = 0.15f, .technical = 0.70f, .spin = 0.15f};
        case AiStyle::Spin:
            return StyleMix {.power = 0.15f, .technical = 0.15f, .spin = 0.70f};
        case AiStyle::Maxed:
            return StyleMix {.power = 0.34f, .technical = 0.33f, .spin = 0.33f};
        case AiStyle::Balanced:
        default:
            return StyleMix {.power = 0.34f, .technical = 0.33f, .spin = 0.33f};
    }
}

StyleMix style_mix_from_skills(const RuntimeAiState& ai_state) {
    const float power = clamp01f(ai_state.skills.power);
    const float technical = clamp01f(ai_state.skills.edge);
    const float spin = clamp01f(ai_state.skills.spin_inject);
    const float total = power + technical + spin;
    if (total <= 1.0e-6f) {
        return fallback_mix_from_style(ai_state.style);
    }
    return StyleMix {
        .power = power / total,
        .technical = technical / total,
        .spin = spin / total};
}

IntentWeights intent_weights_from_mix(const StyleMix& mix) {
    const float stabilize = 0.20f + (0.60f * mix.technical);
    const float pressure = 0.20f + (0.60f * mix.power);
    const float spintrap = 0.20f + (0.60f * mix.spin);
    const float sum = stabilize + pressure + spintrap;
    return IntentWeights {
        .stabilize = stabilize / sum,
        .pressure = pressure / sum,
        .spintrap = spintrap / sum};
}

float intent_weight_for(const IntentWeights& weights, const AiIntent intent) {
    switch (intent) {
        case AiIntent::Pressure:
            return weights.pressure;
        case AiIntent::SpinTrap:
            return weights.spintrap;
        case AiIntent::Stabilize:
        default:
            return weights.stabilize;
    }
}

float style_recover_lane_y_actor(
    const whacker::sim::SimulationConfig& config,
    const AiStyle style,
    const float spin) {
    const float h = config.court_height;
    switch (style) {
        case AiStyle::Power:
            return 0.56f * h;
        case AiStyle::Spin:
            return spin >= 0.0f ? (0.60f * h) : (0.40f * h);
        case AiStyle::Technical:
        case AiStyle::Balanced:
        case AiStyle::Maxed:
        default:
            return 0.50f * h;
    }
}

}  // namespace whacker::app::ai_internal
