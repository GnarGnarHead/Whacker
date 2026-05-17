#include "runtime_visual_transition.hpp"

#ifdef WHACKER_HAS_GLFW

#include <algorithm>
#include <cassert>

namespace whacker::app {

namespace {

constexpr float kMaxAdvanceStepSeconds = 0.25f;

float clamp_duration(const float duration_seconds) {
    return std::max(1.0e-4f, duration_seconds);
}

bool has_valid_story_scene(const StorySceneState* scene) {
    return scene != nullptr && scene->id != StorySceneId::None;
}

void initialize_transition(
    RuntimeVisualTransitionState& transition,
    const AppState from_state,
    const AppState to_state,
    const double now_seconds,
    const float duration_seconds) {
    transition.active = true;
    transition.swapped_to_target = false;
    transition.from_state = from_state;
    transition.to_state = to_state;
    transition.elapsed_seconds = 0.0f;
    transition.duration_seconds = clamp_duration(duration_seconds);
    transition.last_update_time_seconds = now_seconds;
    transition.has_last_update_time = true;
}

void apply_midpoint_swap(
    RuntimeVisualTransitionState& transition,
    AppState& app_state,
    StorySceneState& story_scene_state) {
    if (transition.swapped_to_target) {
        return;
    }
    transition.swapped_to_target = true;
    app_state = transition.to_state;
    if (transition.has_story_scene_swap && transition.to_state == AppState::StoryScene) {
        story_scene_state = transition.to_story_scene;
    }
}

void finalize_transition(
    RuntimeVisualTransitionState& transition,
    AppState& app_state,
    StorySceneState& story_scene_state) {
    apply_midpoint_swap(transition, app_state, story_scene_state);
    transition.active = false;
    transition.has_last_update_time = false;
}

}  // namespace

void clear_authored_transition_request(RuntimeAuthoredTransitionRequest& request) {
    request = RuntimeAuthoredTransitionRequest {};
}

TransitionArmResult arm_authored_star_wipe_transition(
    RuntimeAuthoredTransitionRequest& request,
    const AppState from_state,
    const StorySceneState* from_story_scene,
    const AppState to_state,
    const StorySceneState* to_story_scene,
    const float duration_seconds) {
    auto fail = [](const TransitionArmError error) {
        return TransitionArmResult {.armed = false, .error = error};
    };
    const bool state_changed = from_state != to_state;
    const bool has_valid_from_story_scene = has_valid_story_scene(from_story_scene);
    const bool has_valid_to_story_scene = has_valid_story_scene(to_story_scene);

    const bool from_story_scene_required = from_state == AppState::StoryScene;
    const bool to_story_scene_required = to_state == AppState::StoryScene;
    if ((from_story_scene_required && !has_valid_from_story_scene) ||
        (to_story_scene_required && !has_valid_to_story_scene)) {
#ifndef NDEBUG
        assert(false && "Authored star wipe missing required story scene endpoint.");
#endif
        return fail(TransitionArmError::MissingRequiredStoryScene);
    }

    if (!state_changed) {
        const bool valid_scene_swap =
            from_state == AppState::StoryScene &&
            has_valid_from_story_scene &&
            has_valid_to_story_scene &&
            from_story_scene->id != to_story_scene->id;
        if (!valid_scene_swap) {
#ifndef NDEBUG
            assert(false && "Authored star wipe requires a state or story scene delta.");
#endif
            return fail(TransitionArmError::NoTransitionDelta);
        }
    }

    if (request.armed) {
#ifndef NDEBUG
        assert(false && "Overwriting an armed authored transition request.");
#endif
        return fail(TransitionArmError::RequestAlreadyArmed);
    }
    request.armed = true;
    request.from_state = from_state;
    request.to_state = to_state;
    request.has_from_story_scene = from_story_scene_required;
    request.has_to_story_scene = to_story_scene_required;
    request.from_story_scene = from_story_scene_required ? *from_story_scene : StorySceneState {};
    request.to_story_scene = to_story_scene_required ? *to_story_scene : StorySceneState {};
    request.duration_seconds = duration_seconds <= 0.0f
        ? kRuntimeVisualTransitionDurationSeconds
        : duration_seconds;
    return TransitionArmResult {.armed = true, .error = TransitionArmError::None};
}

void begin_visual_transition_for_state_change(
    RuntimeVisualTransitionState& transition,
    const AppState from_state,
    const AppState to_state,
    const double now_seconds,
    const float duration_seconds) {
    initialize_transition(
        transition,
        from_state,
        to_state,
        now_seconds,
        duration_seconds <= 0.0f ? kRuntimeVisualTransitionDurationSeconds : duration_seconds);
    transition.has_story_scene_swap = false;
    transition.from_story_scene = StorySceneState {};
    transition.to_story_scene = StorySceneState {};
}

void begin_visual_transition_for_scene_change(
    RuntimeVisualTransitionState& transition,
    const StorySceneState& from_scene,
    const StorySceneState& to_scene,
    const double now_seconds,
    const float duration_seconds) {
    initialize_transition(
        transition,
        AppState::StoryScene,
        AppState::StoryScene,
        now_seconds,
        duration_seconds <= 0.0f ? kRuntimeVisualTransitionDurationSeconds : duration_seconds);
    transition.has_story_scene_swap = true;
    transition.from_story_scene = from_scene;
    transition.to_story_scene = to_scene;
}

void begin_visual_transition_for_authored_request(
    RuntimeVisualTransitionState& transition,
    const RuntimeAuthoredTransitionRequest& request,
    const double now_seconds) {
    initialize_transition(
        transition,
        request.from_state,
        request.to_state,
        now_seconds,
        request.duration_seconds <= 0.0f ? kRuntimeVisualTransitionDurationSeconds : request.duration_seconds);
    transition.has_story_scene_swap = request.has_to_story_scene && request.to_state == AppState::StoryScene;
    transition.from_story_scene = request.has_from_story_scene ? request.from_story_scene : StorySceneState {};
    transition.to_story_scene = request.has_to_story_scene ? request.to_story_scene : StorySceneState {};
}

void advance_visual_transition(
    RuntimeVisualTransitionState& transition,
    AppState& app_state,
    StorySceneState& story_scene_state,
    const double now_seconds) {
    if (!transition.active) {
        return;
    }

    float advance_seconds = 0.0f;
    if (transition.has_last_update_time) {
        advance_seconds = static_cast<float>(now_seconds - transition.last_update_time_seconds);
    }
    transition.last_update_time_seconds = now_seconds;
    transition.has_last_update_time = true;

    if (advance_seconds < 0.0f) {
        advance_seconds = 0.0f;
    } else if (advance_seconds > kMaxAdvanceStepSeconds) {
        advance_seconds = kMaxAdvanceStepSeconds;
    }

    transition.elapsed_seconds += advance_seconds;
    const float progress = visual_transition_progress(transition);
    if (progress >= 0.5f) {
        apply_midpoint_swap(transition, app_state, story_scene_state);
    }
    if (progress >= 1.0f) {
        finalize_transition(transition, app_state, story_scene_state);
    }
}

float visual_transition_progress(const RuntimeVisualTransitionState& transition) {
    if (!transition.active) {
        return 1.0f;
    }
    const float safe_duration = clamp_duration(transition.duration_seconds);
    return std::clamp(transition.elapsed_seconds / safe_duration, 0.0f, 1.0f);
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
