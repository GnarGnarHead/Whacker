#include "progression/reputation.hpp"

#include <algorithm>
#include <cmath>

namespace whacker::progression {

namespace {

float clampf(const float value, const float lo, const float hi) {
    return std::clamp(value, lo, hi);
}

}  // namespace

float expected_win_probability(const float self_rating, const float opponent_rating) {
    const float exponent = (opponent_rating - self_rating) / 400.0f;
    const float denom = 1.0f + std::pow(10.0f, exponent);
    return 1.0f / std::max(denom, 1.0e-6f);
}

void apply_official_result(
    ReputationTracker& tracker,
    const OfficialResultInput& result) {
    const float expected = clampf(result.expected_win_prob, 0.0f, 1.0f);
    const float actual = result.won ? 1.0f : 0.0f;
    const float surprise = actual - expected;

    tracker.rating += 28.0f * surprise;
    tracker.momentum = clampf((0.70f * tracker.momentum) + surprise, -2.0f, 2.0f);

    if (result.won) {
        tracker.win_streak += 1;
        tracker.loss_streak = 0;
    } else {
        tracker.loss_streak += 1;
        tracker.win_streak = 0;
    }

    const int margin = std::max(0, result.margin);

    if ((tracker.win_streak >= 3) || ((tracker.momentum > 0.65f) && result.won && (margin >= 2))) {
        tracker.state = ReputationState::Surging;
    } else if ((tracker.loss_streak >= 3) || ((tracker.momentum < -0.65f) && !result.won && (margin >= 2))) {
        tracker.state = ReputationState::Slumping;
    } else if (tracker.momentum > 0.25f) {
        tracker.state = ReputationState::Rising;
    } else if (tracker.momentum < -0.25f) {
        tracker.state = ReputationState::Falling;
    } else {
        tracker.state = ReputationState::Stagnant;
    }
}

const char* to_string(const ReputationState state) {
    switch (state) {
        case ReputationState::Rising:
            return "rising";
        case ReputationState::Falling:
            return "falling";
        case ReputationState::Stagnant:
            return "stagnant";
        case ReputationState::Surging:
            return "surging";
        case ReputationState::Slumping:
            return "slumping";
    }
    return "stagnant";
}

}  // namespace whacker::progression
