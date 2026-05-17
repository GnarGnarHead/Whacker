#pragma once

#include <cstdint>

namespace whacker::sim {

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
};

struct BallState {
    Vec2 position {};
    Vec2 velocity {};
    float spin = 0.0f;
    float speed_scalar = 1.0f;
};

struct PaddleState {
    float center_y = 0.0f;
    float velocity_y = 0.0f;
    float target_y = 0.0f;
    float feedforward_velocity_y = 0.0f;
    // Per-paddle execution scales in [0, 1]. Config values remain hard maximums.
    float power_scale = 1.0f;
    float technical_scale = 1.0f;
    float spin_scale = 1.0f;
};

struct RallyState {
    BallState ball {};
    PaddleState left {};
    PaddleState right {};
    std::uint64_t rally_hits = 0;
    int left_score = 0;
    int right_score = 0;
};

enum class ScoreEvent : std::uint8_t {
    None,
    LeftPlayerScored,
    RightPlayerScored
};

}  // namespace whacker::sim
