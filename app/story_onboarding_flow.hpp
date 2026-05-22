#pragma once

#include "navigation.hpp"
#include "story_state.hpp"

namespace whacker::app {

[[nodiscard]] Screen route_after_completed_story_match(
    StoryRuntimeState& story_runtime,
    StoryMatchKind completed_kind);

}  // namespace whacker::app
