#pragma once

#include "ai_planner_internal.hpp"

namespace whacker::app::ai_internal {

AiDecision make_recover_decision(
    const whacker::sim::Simulation& simulation,
    const RuntimeAiState& ai_state,
    float competence,
    const StyleMix& mix,
    const IntentWeights& weights);

AiDecision make_safe_intercept_decision(
    const whacker::sim::Simulation& simulation,
    const RuntimeAiState& ai_state,
    float intercept_time_s,
    float confidence,
    float competence,
    float intercept_y,
    const StyleMix& mix,
    const IntentWeights& weights,
    const AiCapabilityProfile& capability);

}  // namespace whacker::app::ai_internal
