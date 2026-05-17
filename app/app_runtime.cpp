#include "app_runtime.hpp"

#ifdef WHACKER_HAS_GLFW

#include <random>

#include <GLFW/glfw3.h>

#include "menu_settings.hpp"
#include "runtime_frame_timing.hpp"
#include "runtime_loop_state.hpp"
#include "runtime_loop_policy.hpp"
#include "runtime_phase_drivers.hpp"
#include "runtime_render_drivers.hpp"
#include "runtime_story_save_cache.hpp"

namespace whacker::app {

int run_app_loop(GLFWwindow* window, whacker::sim::Simulation& simulation) {
    std::random_device random_device;
    RuntimeLoopState loop_state(random_device());
    initialize_runtime_loop_state(loop_state);
    const RuntimeLoopPolicy loop_policy = build_runtime_loop_policy();

    RuntimeFrameTimingState frame_timing {};
    frame_timing.previous_time = glfwGetTime();
    RuntimeUpdatePhaseContext update_phase_context = make_runtime_update_phase_context(window, loop_state, simulation);
    RuntimeRenderDriverContext render_phase_context = make_runtime_render_driver_context(window, loop_state, simulation);

    while (!glfwWindowShouldClose(window)) {
        const double now = glfwGetTime();
        advance_runtime_frame_timing(frame_timing, now, loop_policy.max_accumulated_sim_seconds);

        glfwPollEvents();
        RuntimeStorySaveExistsCache story_save_cache {};

        run_runtime_update_phases(
            update_phase_context,
            now,
            frame_timing.accumulator,
            frame_timing.menu_input_lockout,
            loop_policy.menu_input_lockout_seconds,
            loop_policy.story_official_games_to_win,
            &story_save_cache);

        run_runtime_render_phases(render_phase_context, frame_timing.title_cooldown, &story_save_cache);
        glfwSwapBuffers(window);
    }

    save_menu_settings(loop_state.options, loop_state.controls, loop_state.audio_settings);
    return 0;
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
