#pragma once

#include <cstdint>

#include "ai_core.hpp"
#include "app_types.hpp"

namespace whacker::app {

AiDecision runtime_ai_decision_from_plan(const RuntimeAiPlanState& plan);

void write_runtime_ai_plan_from_decision(
    RuntimeAiState& ai_state,
    const AiDecision& decision,
    std::uint64_t state_signature,
    bool inbound);

}  // namespace whacker::app

