#include "story_text_week.hpp"

#include <cassert>

#include "story_text_week_01.hpp"

namespace whacker::app::story_text_week {

namespace {

constexpr std::string_view kWeek01NodeId = "club_week_01";

bool is_week_01_node(const std::string_view node_id) {
    return node_id == kWeek01NodeId;
}

}  // namespace

std::string_view scene_text(const std::string_view node_id, const SceneKey key) {
    if (!is_week_01_node(node_id)) {
        return {};
    }

    const std::string_view text = week_01_scene_text(key);
    assert(!text.empty() && "Week-01 scene key missing authored copy.");
    return text;
}

bool match_start_feedback(
    const std::string_view node_id,
    const StoryMatchKind match_kind,
    story_text::FeedbackLines& out_feedback) {
    if (!is_week_01_node(node_id)) {
        return false;
    }
    return week_01_match_start_feedback(match_kind, out_feedback);
}

}  // namespace whacker::app::story_text_week

