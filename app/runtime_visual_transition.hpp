#pragma once

#include <cstdint>

#include "story_scene.hpp"
#include "ui_state.hpp"

namespace whacker::app {

constexpr float kRuntimeVisualTransitionDurationSeconds = 1.5f;

struct RuntimeVisualTransitionState {
    bool active = false;
    bool swapped_to_target = false;
    AppState from_state = AppState::MainMenu;
    AppState to_state = AppState::MainMenu;
    float elapsed_seconds = 0.0f;
    float duration_seconds = kRuntimeVisualTransitionDurationSeconds;
    double last_update_time_seconds = 0.0;
    bool has_last_update_time = false;

    bool has_story_scene_swap = false;
    StorySceneState from_story_scene {};
    StorySceneState to_story_scene {};
};

struct RuntimeAuthoredTransitionRequest {
    bool armed = false;
    AppState from_state = AppState::MainMenu;
    AppState to_state = AppState::MainMenu;
    bool has_from_story_scene = false;
    bool has_to_story_scene = false;
    StorySceneState from_story_scene {};
    StorySceneState to_story_scene {};
    float duration_seconds = kRuntimeVisualTransitionDurationSeconds;
};

enum class TransitionArmError : std::uint8_t {
    None = 0,
    MissingRequiredStoryScene = 1,
    NoTransitionDelta = 2,
    RequestAlreadyArmed = 3,
};

struct TransitionArmResult {
    bool armed = false;
    TransitionArmError error = TransitionArmError::None;

    constexpr explicit operator bool() const { return armed; }
};

constexpr const char* transition_arm_error_label(const TransitionArmError error) {
    switch (error) {
        case TransitionArmError::MissingRequiredStoryScene:
            return "missing_required_story_scene";
        case TransitionArmError::NoTransitionDelta:
            return "no_transition_delta";
        case TransitionArmError::RequestAlreadyArmed:
            return "request_already_armed";
        case TransitionArmError::None:
        default:
            return "none";
    }
}

void clear_authored_transition_request(RuntimeAuthoredTransitionRequest& request);

[[nodiscard]] TransitionArmResult arm_authored_star_wipe_transition(
    RuntimeAuthoredTransitionRequest& request,
    AppState from_state,
    const StorySceneState* from_story_scene,
    AppState to_state,
    const StorySceneState* to_story_scene,
    float duration_seconds = kRuntimeVisualTransitionDurationSeconds);

void begin_visual_transition_for_state_change(
    RuntimeVisualTransitionState& transition,
    AppState from_state,
    AppState to_state,
    double now_seconds,
    float duration_seconds = kRuntimeVisualTransitionDurationSeconds);

void begin_visual_transition_for_scene_change(
    RuntimeVisualTransitionState& transition,
    const StorySceneState& from_scene,
    const StorySceneState& to_scene,
    double now_seconds,
    float duration_seconds = kRuntimeVisualTransitionDurationSeconds);

void begin_visual_transition_for_authored_request(
    RuntimeVisualTransitionState& transition,
    const RuntimeAuthoredTransitionRequest& request,
    double now_seconds);

void advance_visual_transition(
    RuntimeVisualTransitionState& transition,
    AppState& app_state,
    StorySceneState& story_scene_state,
    double now_seconds);

float visual_transition_progress(const RuntimeVisualTransitionState& transition);

}  // namespace whacker::app
