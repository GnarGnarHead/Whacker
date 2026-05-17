#pragma once

#include "ai_planner_internal.hpp"

namespace whacker::app::ai_internal {

float competence_from_skills(const whacker::progression::SkillState& skills);
AiCapabilityProfile capability_profile_for(float competence);
StyleMix fallback_mix_from_style(AiStyle style);
StyleMix style_mix_from_skills(const RuntimeAiState& ai_state);
IntentWeights intent_weights_from_mix(const StyleMix& mix);
float intent_weight_for(const IntentWeights& weights, AiIntent intent);
float style_recover_lane_y_actor(
    const whacker::sim::SimulationConfig& config,
    AiStyle style,
    float spin);

}  // namespace whacker::app::ai_internal
