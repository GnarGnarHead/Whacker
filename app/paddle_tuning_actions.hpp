#pragma once

#include "menu_intent.hpp"
#include "paddle_tuning.hpp"
#include "story_state.hpp"
#include "ui_state.hpp"

namespace whacker::app {

struct PaddleTuningInputIntent {
    MenuIntent pressed {};
    MenuIntent held {};
    bool pause = false;
};

enum class PaddleTuningActionResult {
    None,
    Changed,
    Commit,
    Cancel
};

void begin_quick_paddle_tuning(
    PaddleTuningState& tuning_state,
    AppState return_state,
    PaddleTuningTarget target,
    const whacker::progression::SkillState& skills);

void begin_story_player_paddle_tuning(
    PaddleTuningState& tuning_state,
    const StoryCareerData& career);

PaddleTuningActionResult apply_paddle_tuning_action(
    PaddleTuningState& tuning_state,
    const PaddleTuningInputIntent& intent);

void commit_paddle_tuning_to_options(
    const PaddleTuningState& tuning_state,
    MatchOptions& options);

void commit_paddle_tuning_to_career(
    const PaddleTuningState& tuning_state,
    StoryCareerData& career);

}  // namespace whacker::app
