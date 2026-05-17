#pragma once

#include <array>

#include "ai_planner_internal.hpp"

namespace whacker::app::ai_internal {

struct CandidateSelectionResult {
    bool has_winner = false;
    Candidate winner {};
    int scored_candidate_count = 0;
    int reachable_candidate_count = 0;
};

CandidateSelectionResult select_best_candidate(
    std::array<Candidate, 48>& candidates,
    int candidate_count,
    int max_candidates,
    std::uint32_t base_seed,
    std::uint64_t decision_counter);

int risk_level_from_term(float risk_term);

}  // namespace whacker::app::ai_internal
