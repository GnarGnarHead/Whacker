#include "runtime_step_phase.hpp"

#ifdef WHACKER_HAS_GLFW

#include <algorithm>

#include "runtime_step_phase_internal.hpp"

namespace whacker::app {

void handle_runtime_step_phase(RuntimeStepPhaseArgs& args) {
    constexpr double kFixedDtSeconds = static_cast<double>(whacker::sim::kFixedDt);

    RuntimeStepPhaseContext context {
        args.window,
        args.app_state,
        args.ai_controls_player_paddle,
        args.options,
        args.controls,
        args.story_runtime,
        args.story_hub_state,
        args.story_intro_state,
        args.story_scene_state,
        args.authored_transition_request,
        args.match_flow,
        args.left_ai_state,
        args.right_ai_state,
        args.simulation,
        args.rng,
        args.audio_engine,
        args.story_official_games_to_win};

    while (args.accumulator >= kFixedDtSeconds) {
        args.type_blip_cooldown = std::max(
            0.0f,
            args.type_blip_cooldown - static_cast<float>(whacker::sim::kFixedDt));
        if (context.app_state == AppState::Paused) {
            args.accumulator -= kFixedDtSeconds;
            continue;
        }

        RuntimeStepBranchOutcome outcome = RuntimeStepBranchOutcome::NeedsCommonSimulationStep;
        if (context.app_state == AppState::StoryIntro) {
            outcome = step_story_intro(
                context,
                args.step_input,
                args.now,
                args.type_blip_cooldown,
                args.type_blip_pattern_step);
        } else if (context.app_state == AppState::StoryScene) {
            outcome = step_story_scene(
                context,
                args.step_input,
                args.now,
                args.type_blip_cooldown,
                args.type_blip_pattern_step);
        } else if (context.app_state != AppState::Playing) {
            outcome = step_non_playing_ambient(context);
        } else {
            outcome = step_playing(context);
        }

        if (outcome == RuntimeStepBranchOutcome::SteppedInsideBranch) {
            args.accumulator -= kFixedDtSeconds;
            continue;
        }

        context.simulation.step(whacker::sim::kFixedDt);
        args.accumulator -= kFixedDtSeconds;
    }
}

void handle_runtime_step_phase(
    GLFWwindow* window,
    const double now,
    double& accumulator,
    const RuntimeStepInputSnapshot& step_input,
    AppState& app_state,
    const bool ai_controls_player_paddle,
    MatchOptions& options,
    const ControlBindings& controls,
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    StoryIntroState& story_intro_state,
    StorySceneState& story_scene_state,
    RuntimeAuthoredTransitionRequest& authored_transition_request,
    MatchFlowState& match_flow,
    RuntimeAiState& left_ai_state,
    RuntimeAiState& right_ai_state,
    whacker::sim::Simulation& simulation,
    std::mt19937_64& rng,
    AudioEngine& audio_engine,
    float& type_blip_cooldown,
    std::uint32_t& type_blip_pattern_step,
    const int story_official_games_to_win) {
    RuntimeStepPhaseArgs args {
        window,
        now,
        accumulator,
        step_input,
        app_state,
        ai_controls_player_paddle,
        options,
        controls,
        story_runtime,
        story_hub_state,
        story_intro_state,
        story_scene_state,
        authored_transition_request,
        match_flow,
        left_ai_state,
        right_ai_state,
        simulation,
        rng,
        audio_engine,
        type_blip_cooldown,
        type_blip_pattern_step,
        story_official_games_to_win};
    handle_runtime_step_phase(args);
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
