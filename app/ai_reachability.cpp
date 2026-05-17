#include "ai_reachability.hpp"

namespace whacker::app::ai_internal {

ReachabilityEnvelope compute_reachability_envelope(
    const whacker::sim::PaddleState& paddle,
    const whacker::sim::SimulationConfig& config,
    const float horizon_s,
    const int max_steps,
    const float speed_scale,
    const float accel_scale) {
    const float horizon = std::max(horizon_s, whacker::sim::kFixedDt);
    const int steps = clampi(
        static_cast<int>(std::ceil(horizon / whacker::sim::kFixedDt)),
        1,
        max_steps);

    auto integrate = [&](const float desired_velocity) {
        float y = paddle.center_y;
        float v = paddle.velocity_y;
        const float a = config.paddle_accel * clampf(accel_scale, 0.05f, 1.0f);
        const float vmax = config.paddle_max_speed * clampf(speed_scale, 0.05f, 1.0f);
        const float min_y = config.paddle_half_height;
        const float max_y = config.court_height - config.paddle_half_height;

        for (int i = 0; i < steps; ++i) {
            const float dv = clampf(
                desired_velocity - v,
                -(a * whacker::sim::kFixedDt),
                a * whacker::sim::kFixedDt);
            v += dv;
            v = clampf(v, -vmax, vmax);
            y += v * whacker::sim::kFixedDt;
            if (y < min_y) {
                y = min_y;
                v = 0.0f;
            } else if (y > max_y) {
                y = max_y;
                v = 0.0f;
            }
        }
        return y;
    };

    const float min_y = integrate(-config.paddle_max_speed);
    const float max_y = integrate(config.paddle_max_speed);
    return ReachabilityEnvelope {
        .min_center_y = std::min(min_y, max_y),
        .max_center_y = std::max(min_y, max_y),
    };
}

}  // namespace whacker::app::ai_internal
