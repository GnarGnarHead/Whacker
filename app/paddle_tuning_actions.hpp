#pragma once

#include "action_input.hpp"
#include "paddle_tuning.hpp"
#include "story_state.hpp"
#include "ui_state.hpp"

namespace whacker::app {

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

PaddleTuningActionResult apply_paddle_tuning_action_frame(
    PaddleTuningState& tuning_state,
    const ActionInputFrame& input);

void commit_paddle_tuning_to_options(
    const PaddleTuningState& tuning_state,
    MatchOptions& options);

void commit_paddle_tuning_to_career(
    const PaddleTuningState& tuning_state,
    StoryCareerData& career);

}  // namespace whacker::app
