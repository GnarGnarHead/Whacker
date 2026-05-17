#pragma once

#include <cstdint>

namespace whacker::progression {

enum class ReputationState : std::uint8_t {
    Rising,
    Falling,
    Stagnant,
    Surging,
    Slumping
};

struct ReputationTracker {
    float rating = 1000.0f;
    int win_streak = 0;
    int loss_streak = 0;
    float momentum = 0.0f;
    ReputationState state = ReputationState::Stagnant;
};

struct OfficialResultInput {
    bool won = false;
    int margin = 0;
    float expected_win_prob = 0.5f;
};

float expected_win_probability(float self_rating, float opponent_rating);

void apply_official_result(
    ReputationTracker& tracker,
    const OfficialResultInput& result);

const char* to_string(ReputationState state);

}  // namespace whacker::progression
