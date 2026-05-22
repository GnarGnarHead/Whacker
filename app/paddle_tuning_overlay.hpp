#pragma once

#include "paddle_tuning.hpp"
#include "render_context.hpp"

namespace whacker::app {

void render_paddle_tuning_overlay(const RenderContext& context, const PaddleTuningState& tuning_state);

}  // namespace whacker::app
