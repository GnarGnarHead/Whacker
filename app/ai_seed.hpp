#pragma once

#include "ai_core.hpp"

namespace whacker::app::ai_internal {

uint32_t make_noise_base(
    const whacker::sim::RallyState& state,
    AiStyle style,
    const whacker::progression::SkillState& skills);

float keyed_noise_u01(
    uint32_t base_seed,
    std::uint64_t decision_counter,
    int phase_id,
    int candidate_id,
    int draw_id);

uint64_t compute_state_signature_from_rally_state(const whacker::sim::RallyState& state, bool for_left_paddle);
uint64_t compute_state_signature_from_simulation(const whacker::sim::Simulation& simulation, bool for_left_paddle);

}  // namespace whacker::app::ai_internal
