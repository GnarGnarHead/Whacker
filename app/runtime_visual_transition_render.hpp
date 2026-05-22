#pragma once

#include "render_context.hpp"
#include "runtime_visual_transition.hpp"

namespace whacker::app {

void render_visual_transition_overlay(
    const RenderContext& context,
    const RuntimeVisualTransitionState& transition);

void release_visual_transition_render_resources();

}  // namespace whacker::app
