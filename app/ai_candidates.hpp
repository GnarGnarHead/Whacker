#pragma once

#include <array>

#include "ai_planner_internal.hpp"

namespace whacker::app::ai_internal {

struct CandidateGenerationResult {
    std::array<Candidate, 48> candidates {};
    int candidate_count = 0;
    float vmax = 0.0f;
    float spin_skill = 0.0f;
};

CandidateGenerationResult generate_scored_candidates(
    const whacker::sim::RallyState& state,
    const whacker::sim::SimulationConfig& config,
    const whacker::sim::PaddleState& self,
    const whacker::sim::PaddleState& opponent,
    const RuntimeAiState& ai_state,
    const AiCapabilityProfile& capability,
    const StyleMix& mix,
    const IntentWeights& weights,
    const PredictorResult& prediction,
    const ReachabilityEnvelope& envelope,
    float planned_intercept_y);

}  // namespace whacker::app::ai_internal
