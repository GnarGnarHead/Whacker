#pragma once

#ifdef WHACKER_HAS_GLFW

namespace whacker::app {

struct RuntimeFrameTimingState {
    double previous_time = 0.0;
    double accumulator = 0.0;
    double title_cooldown = 0.0;
    double menu_input_lockout = 0.0;
};

double advance_runtime_frame_timing(
    RuntimeFrameTimingState& timing_state,
    double now,
    double max_accumulated_sim_seconds);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
