#include "app_runtime.hpp"

#include <algorithm>

#include "action_input.hpp"
#include "platform_sdl.hpp"
#include "sdl_runtime_render.hpp"
#include "sdl_runtime_state.hpp"
#include "sdl_runtime_update.hpp"

namespace whacker::app {

int run_app_loop(SdlPlatform& platform, whacker::sim::Simulation& simulation) {
    SdlRuntimeState runtime {};
    initialize_sdl_runtime_state(runtime);
    runtime.previous_time = platform.now_seconds();

    while (!platform.should_close()) {
        platform.poll_events();
        runtime.input.sample();
        const ActionInputFrame& input = runtime.input.frame();
        const SdlEventFrame& events = platform.event_frame();

        const double now = platform.now_seconds();
        const double frame_dt = std::clamp(now - runtime.previous_time, 0.0, 0.1);
        runtime.previous_time = now;

        const RenderContext render_context = platform.render_context();
        update_runtime(input, events, render_context, frame_dt, runtime, simulation, platform);
        render_runtime_frame(render_context, runtime, simulation);
        platform.swap_buffers();
    }

    shutdown_sdl_runtime_state(runtime);
    return 0;
}

}  // namespace whacker::app
