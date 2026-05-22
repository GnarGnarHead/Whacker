#pragma once

#include <random>

#include "action_input.hpp"
#include "match_flow.hpp"
#include "paddle_tuning.hpp"
#include "sim/physics.hpp"
#include "story_runtime.hpp"
#include "story_scene.hpp"
#include "ui_state.hpp"

namespace whacker::app {

struct StoryHubControllerInput {
    bool move_up = false;
    bool move_down = false;
    bool confirm = false;
    bool back = false;
};

struct StoryHubControllerContext {
    StoryRuntimeState& story_runtime;
    StoryHubState& story_hub;
    StorySceneState& story_scene;
    PaddleTuningState& paddle_tuning;
    MatchOptions& options;
    MatchFlowState& match_flow;
    whacker::sim::Simulation& simulation;
    std::mt19937_64& rng;
    AppState& app_state;
};

struct StoryHubControllerEffects {
    bool play_move_sound = false;
    bool play_confirm_sound = false;
    bool enter_story_menu = false;
    bool return_to_main_menu = false;
};

StoryHubControllerEffects update_story_hub_controller(
    StoryHubControllerContext context,
    StoryHubControllerInput input,
    StorySaveCareerCallback save_career_fn);

StoryHubControllerEffects update_story_hub_controller_frame(
    StoryHubControllerContext context,
    const ActionInputFrame& input,
    StorySaveCareerCallback save_career_fn);

}  // namespace whacker::app
