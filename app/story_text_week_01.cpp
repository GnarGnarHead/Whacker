#include "story_text_week_01.hpp"

#include "story_pack.hpp"

namespace whacker::app::story_text_week {

std::string_view week_01_scene_text(const SceneKey key) {
    return story_pack::week_01_scene_text(key);
}

bool week_01_match_start_feedback(const StoryMatchKind match_kind, story_text::FeedbackLines& out_feedback) {
    return story_pack::week_01_match_start_feedback(match_kind, out_feedback);
}

}  // namespace whacker::app::story_text_week
