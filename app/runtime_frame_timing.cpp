#include "runtime_frame_timing.hpp"

#ifdef WHACKER_HAS_GLFW

#include <algorithm>

namespace whacker::app {

namespace {

constexpr double kMaxFrameDtSeconds = 0.1;

}  // namespace

double advance_runtime_frame_timing(
    RuntimeFrameTimingState& timing_state,
    const double now,
    const double max_accumulated_sim_seconds) {
    const double raw_frame_dt = now - timing_state.previous_time;
    timing_state.previous_time = now;

    const double frame_dt = std::clamp(raw_frame_dt, 0.0, kMaxFrameDtSeconds);
    timing_state.accumulator = std::min(timing_state.accumulator + frame_dt, max_accumulated_sim_seconds);
    timing_state.title_cooldown += frame_dt;
    timing_state.menu_input_lockout = std::max(0.0, timing_state.menu_input_lockout - frame_dt);
    return frame_dt;
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
