#include "ai_score.hpp"

#include <algorithm>

#include "ai_seed.hpp"

namespace whacker::app::ai_internal {

CandidateSelectionResult select_best_candidate(
    std::array<Candidate, 48>& candidates,
    int candidate_count,
    const int max_candidates,
    const std::uint32_t base_seed,
    const std::uint64_t decision_counter) {
    CandidateSelectionResult result {};
    if (candidate_count <= 0) {
        return result;
    }

    const int bounded_max = clampi(max_candidates, 1, 24);
    if (candidate_count > bounded_max) {
        std::stable_sort(
            candidates.begin(),
            candidates.begin() + candidate_count,
            [&](const Candidate& a, const Candidate& b) {
                if (std::abs(a.cheap_score - b.cheap_score) > kScoreEpsilon) {
                    return a.cheap_score > b.cheap_score;
                }
                if (a.id != b.id) {
                    return a.id < b.id;
                }
                const float ta = keyed_noise_u01(base_seed, decision_counter, kPhaseTieBreak, a.id, 0);
                const float tb = keyed_noise_u01(base_seed, decision_counter, kPhaseTieBreak, b.id, 0);
                return ta < tb;
            });
        candidate_count = bounded_max;
    }

    result.scored_candidate_count = candidate_count;
    result.reachable_candidate_count = candidate_count;

    int best_index = 0;
    float best_score = candidates[0].score;
    for (int i = 1; i < candidate_count; ++i) {
        const float score = candidates[static_cast<std::size_t>(i)].score;
        if (score > best_score + kScoreEpsilon) {
            best_index = i;
            best_score = score;
            continue;
        }
        if (std::abs(score - best_score) <= kScoreEpsilon) {
            const int id_a = candidates[static_cast<std::size_t>(i)].id;
            const int id_b = candidates[static_cast<std::size_t>(best_index)].id;
            if (id_a < id_b) {
                best_index = i;
                best_score = score;
            }
        }
    }

    result.has_winner = true;
    result.winner = candidates[static_cast<std::size_t>(best_index)];
    return result;
}

int risk_level_from_term(const float risk_term) {
    if (risk_term < 0.30f) {
        return 0;
    }
    if (risk_term < 0.60f) {
        return 1;
    }
    return 2;
}

}  // namespace whacker::app::ai_internal
