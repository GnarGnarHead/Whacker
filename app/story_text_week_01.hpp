#pragma once

#include <string_view>

#include "story_text_week.hpp"

namespace whacker::app::story_text_week {

std::string_view week_01_scene_text(SceneKey key);
bool week_01_match_start_feedback(StoryMatchKind match_kind, story_text::FeedbackLines& out_feedback);

}  // namespace whacker::app::story_text_week

