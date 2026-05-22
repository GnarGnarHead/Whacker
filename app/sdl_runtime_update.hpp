#pragma once

#include "action_input.hpp"
#include "platform_sdl.hpp"
#include "render_context.hpp"
#include "sdl_runtime_state.hpp"
#include "sim/physics.hpp"

namespace whacker::app {

void update_runtime(
    const ActionInputFrame& input,
    const SdlEventFrame& events,
    const RenderContext& render_context,
    double frame_dt,
    SdlRuntimeState& runtime,
    whacker::sim::Simulation& simulation,
    SdlPlatform& platform);

}  // namespace whacker::app
