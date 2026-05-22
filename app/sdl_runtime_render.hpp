#pragma once

#include "render_context.hpp"
#include "sdl_runtime_state.hpp"
#include "sim/physics.hpp"

namespace whacker::app {

void render_runtime_frame(
    const RenderContext& render_context,
    const SdlRuntimeState& runtime,
    const whacker::sim::Simulation& simulation);

}  // namespace whacker::app
