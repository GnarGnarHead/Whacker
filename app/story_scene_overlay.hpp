#pragma once

#include "render_context.hpp"

namespace whacker::app {

struct StorySceneState;

void render_story_scene_overlay(
    const RenderContext& context,
    const StorySceneState& scene_state);

}  // namespace whacker::app
