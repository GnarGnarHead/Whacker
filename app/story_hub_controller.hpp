#pragma once

#include <random>

#include "match_flow.hpp"
#include "menu_intent.hpp"
#include "paddle_tuning.hpp"
#include "sim/physics.hpp"
#include "story_runtime.hpp"
#include "story_scene.hpp"
#include "ui_state.hpp"

namespace whacker::app {

struct StoryHubControllerContext {
    StoryRuntimeState& story_runtime;
    StoryHubState& story_hub;
    StorySceneState& story_scene;
    PaddleTuningState& paddle_tuning;
    MatchOptions& options;
    MatchFlowState& match_flow;
    whacker::sim::Simulation& simulation;
    std::mt19937_64& rng;
};

enum class StoryHubRoute {
    None,
    MainMenu,
    StoryMenu,
    StoryScene,
    PaddleTuning,
    Playing,
};

struct StoryHubControllerEffects {
    bool play_move_sound = false;
    bool play_confirm_sound = false;
    StoryHubRoute route = StoryHubRoute::None;
};

StoryHubControllerEffects update_story_hub_controller(
    StoryHubControllerContext context,
    const MenuIntent& intent,
    StorySaveCareerCallback save_career_fn);

}  // namespace whacker::app
