#pragma once

#include "navigation.hpp"
#include "story_runtime.hpp"
#include "story_scene.hpp"

namespace whacker::app {

inline StorySceneState materialize_story_scene_transition_target(
    const StorySceneState& current_story_scene_state,
    const StoryRuntimeState& story_runtime,
    const Screen target_screen) {
    if (target_screen != Screen::StoryScene) {
        return StorySceneState {};
    }
    if (!story_runtime.onboarding_scene_pending && !story_runtime.post_forfeit_scene_pending) {
        return current_story_scene_state;
    }
    StorySceneState materialized_scene {};
    begin_story_onboarding_scene(materialized_scene, story_runtime);
    return materialized_scene;
}

}  // namespace whacker::app
