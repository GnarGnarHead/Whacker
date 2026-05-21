#include "play_control_human.hpp"

#include <algorithm>

namespace {

float clampf(const float value, const float lo, const float hi) {
    return std::max(lo, std::min(value, hi));
}

}  // namespace

namespace whacker::app {

void set_human_target(
    whacker::sim::PaddleState& paddle,
    const whacker::sim::SimulationConfig& config,
    const bool move_up,
    const bool move_down,
    const float dt) {
    const int direction = static_cast<int>(move_down) - static_cast<int>(move_up);
    set_human_axis_target(paddle, config, static_cast<float>(direction), dt);
}

void set_human_axis_target(
    whacker::sim::PaddleState& paddle,
    const whacker::sim::SimulationConfig& config,
    const float move_y,
    const float dt) {
    const float axis = clampf(move_y, -1.0f, 1.0f);
    const float min_y = config.paddle_half_height;
    const float max_y = config.court_height - config.paddle_half_height;
    const float max_speed = config.paddle_max_speed;

    if (axis == 0.0f) {
        paddle.target_y = clampf(paddle.center_y, min_y, max_y);
        paddle.feedforward_velocity_y = 0.0f;
        return;
    }

    const float next_target = paddle.target_y + axis * max_speed * dt;
    paddle.target_y = clampf(next_target, min_y, max_y);
    paddle.feedforward_velocity_y = axis * max_speed;
}

}  // namespace whacker::app
