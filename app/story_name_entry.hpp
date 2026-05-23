#pragma once

#include <string>

#include "menu_intent.hpp"
#include "story_intro.hpp"

namespace whacker::app {

struct StoryNameTextInput {
    std::string text {};
    bool backspace_pressed = false;
};

struct StoryNameEntryEditResult {
    bool changed = false;
    bool consumed_back = false;
};

enum class StoryNameEntryConfirmResult {
    None,
    ConfirmationStarted,
    Accepted
};

void reset_story_name_entry_editor(StoryIntroState& story_intro_state);
void prepare_story_name_entry(StoryIntroState& story_intro_state);

std::string story_name_entry_display_text(const StoryIntroState& story_intro_state);

StoryNameEntryEditResult apply_story_name_entry_input(
    StoryIntroState& story_intro_state,
    const MenuIntent& intent,
    const StoryNameTextInput& text_input);

StoryNameEntryConfirmResult confirm_story_name_entry(
    StoryIntroState& story_intro_state,
    StoryIntroSanitizeNameFn sanitize_name_fn);

}  // namespace whacker::app
