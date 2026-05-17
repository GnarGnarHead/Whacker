#pragma once

#include <string>

#include "story_state.hpp"

namespace whacker::app {

std::string sanitize_player_name(const std::string& raw_name);
void reset_story_career(StoryCareerData& career);

bool story_save_exists();
bool save_story_career(const StoryCareerData& career_in, std::string* error_message = nullptr);
bool load_story_career(StoryCareerData& out_career, std::string* error_message = nullptr);

}  // namespace whacker::app
