#pragma once

#include <string>

#include "action_input.hpp"
#include "match_flow.hpp"
#include "sim/physics.hpp"
#include "story_runtime.hpp"
#include "story_scene.hpp"
#include "ui_state.hpp"

namespace whacker::app {

using StoryLoadCareerCallback = bool (*)(StoryCareerData&, std::string*);

struct StoryMenuControllerContext {
    StoryMenuState& menu;
    StoryRuntimeState& story_runtime;
    StoryHubState& story_hub;
    StoryIntroState& story_intro;
    StorySceneState& story_scene;
    MatchOptions& options;
    MatchFlowState& match_flow;
    whacker::sim::Simulation& simulation;
    std::string* feedback = nullptr;
};

enum class StoryMenuRoute {
    None,
    MainMenu,
    StoryIntro,
    StoryHub,
    StoryScene,
};

struct StoryMenuControllerEffects {
    bool play_move_sound = false;
    bool play_confirm_sound = false;
    StoryMenuRoute route = StoryMenuRoute::None;
};

StoryMenuControllerEffects update_story_menu_controller(
    StoryMenuControllerContext context,
    const ActionInputFrame& input,
    bool has_save,
    StoryLoadCareerCallback load_career_fn,
    StoryResetCareerFn reset_career_fn = nullptr);

}  // namespace whacker::app
