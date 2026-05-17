#include <cmath>
#include <cstdlib>

#include "play_control_human.hpp"

namespace {

bool approx_equal(const float a, const float b, const float eps = 1.0e-5f) {
    return std::fabs(a - b) <= eps;
}

void require(const bool condition) {
    if (!condition) {
        std::abort();
    }
}

void test_set_human_target_idle_tracks_center_and_zeroes_feedforward() {
    whacker::sim::Simulation simulation {};
    const auto& config = simulation.config();

    whacker::sim::PaddleState paddle {};
    paddle.center_y = 123.0f;
    paddle.target_y = 333.0f;
    paddle.feedforward_velocity_y = 99.0f;

    whacker::app::set_human_target(paddle, config, false, false, whacker::sim::kFixedDt);

    require(approx_equal(paddle.target_y, 123.0f));
    require(approx_equal(paddle.feedforward_velocity_y, 0.0f));
}

void test_set_human_target_moves_and_clamps() {
    whacker::sim::Simulation simulation {};
    const auto& config = simulation.config();
    const float min_y = config.paddle_half_height;
    const float max_y = config.court_height - config.paddle_half_height;

    whacker::sim::PaddleState paddle_up {};
    paddle_up.center_y = max_y - 1.0f;
    paddle_up.target_y = max_y - 1.0f;

    whacker::app::set_human_target(paddle_up, config, true, false, whacker::sim::kFixedDt);
    require(approx_equal(paddle_up.feedforward_velocity_y, -config.paddle_max_speed));
    require(paddle_up.target_y >= min_y);
    require(paddle_up.target_y <= max_y);

    whacker::sim::PaddleState paddle_down {};
    paddle_down.center_y = max_y;
    paddle_down.target_y = max_y;

    whacker::app::set_human_target(paddle_down, config, false, true, whacker::sim::kFixedDt);
    require(approx_equal(paddle_down.feedforward_velocity_y, config.paddle_max_speed));
    require(approx_equal(paddle_down.target_y, max_y));
}

}  // namespace

int main() {
    test_set_human_target_idle_tracks_center_and_zeroes_feedforward();
    test_set_human_target_moves_and_clamps();
    return 0;
}

