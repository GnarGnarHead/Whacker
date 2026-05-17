#pragma once

namespace whacker::app {

struct RuntimeLoopPolicy {
    int story_official_games_to_win = 3;
    double menu_input_lockout_seconds = 0.14;
    double max_sim_steps_per_frame = 8.0;
    double max_accumulated_sim_seconds = 0.0;
};

RuntimeLoopPolicy build_runtime_loop_policy();

}  // namespace whacker::app
