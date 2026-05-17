#pragma once

#include <span>
#include <string_view>

#include "menu_sticker_pack.hpp"
#include "story_state.hpp"
#include "story_text.hpp"
#include "story_text_week.hpp"

namespace whacker::app {
struct StoryGraphNodeSpec;
}

namespace whacker::app::story_pack {

std::string_view week_01_scene_text(story_text_week::SceneKey key);

bool week_01_match_start_feedback(StoryMatchKind match_kind, story_text::FeedbackLines& out_feedback);

std::span<const StoryGraphNodeSpec> season1_hub_graph();
std::span<const MenuStickerSurfaceSpec> menu_sticker_surfaces();

}  // namespace whacker::app::story_pack
