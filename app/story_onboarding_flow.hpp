#pragma once

#include "story_state.hpp"
#include "ui_state.hpp"

namespace whacker::app {

void route_after_completed_story_match(
    StoryRuntimeState& story_runtime,
    StoryMatchKind completed_kind,
    AppState& app_state);

}  // namespace whacker::app
