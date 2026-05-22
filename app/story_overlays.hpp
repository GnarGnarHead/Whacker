#pragma once

#include <string>

#include "render_context.hpp"
#include "story_state.hpp"
#include "ui_state.hpp"

namespace whacker::app {

using StoryRowNameFn = const char* (*)(int);
using StoryHubRowEnabledFn = bool (*)(StoryHubRow, const StoryCareerData&);
using StorySanitizeNameCallback = std::string (*)(const std::string&);

void render_story_menu_overlay(
    const RenderContext& context,
    const StoryMenuState& menu_state,
    bool has_save,
    StoryRowNameFn story_menu_row_name_fn,
    const std::string& status_message = {});

void render_story_hub_overlay(
    const RenderContext& context,
    const StoryRuntimeState& story_runtime,
    const StoryHubState& story_hub_state,
    StoryRowNameFn story_hub_row_name_fn,
    StoryHubRowEnabledFn story_hub_row_enabled_fn,
    StorySanitizeNameCallback sanitize_name_fn);

}  // namespace whacker::app
