#pragma once

#include "menu_input.hpp"
#include "render_context.hpp"
#include "story_intro.hpp"
#include "story_state.hpp"

namespace whacker::app {

void render_story_intro_overlay(
    const RenderContext& context,
    const StoryRuntimeState& story_runtime,
    const StoryIntroState& story_intro_state,
    const ControlHintBindings& controls,
    StoryIntroKeyNameFn key_name_fn,
    StoryIntroSanitizeNameFn sanitize_name_fn);

}  // namespace whacker::app
