#pragma once

#include <cstdio>
#include <string>

#include "story_state.hpp"

namespace whacker::app {

template <typename SaveCareerFn>
bool persist_story_career_with_feedback(
    const StoryCareerData& career,
    const SaveCareerFn save_career_fn,
    StoryHubState* story_hub_state = nullptr,
    std::string* out_error = nullptr) {
    if (out_error != nullptr) {
        out_error->clear();
    }
    if (save_career_fn == nullptr) {
        return true;
    }

    std::string save_error;
    const bool saved = save_career_fn(career, &save_error);
    if (saved) {
        return true;
    }

    if (save_error.empty()) {
        save_error = "Failed to save story progress.";
    }
    if (story_hub_state != nullptr) {
        story_hub_state->feedback_line_1 = "Save failed. Progress not written.";
        story_hub_state->feedback_line_2 = save_error;
    }
    std::fprintf(stderr, "[story-save] %s\n", save_error.c_str());
    if (out_error != nullptr) {
        *out_error = save_error;
    }
    return false;
}

}  // namespace whacker::app
