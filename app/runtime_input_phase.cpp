#include "runtime_input_phase.hpp"

#ifdef WHACKER_HAS_GLFW

#include "runtime_input_phase_internal.hpp"
#include "runtime_story_save_cache.hpp"
#include "story_save.hpp"

namespace whacker::app {

namespace {

bool resolve_story_save_exists(RuntimeStorySaveExistsCache* story_save_cache) {
    if (story_save_cache != nullptr) {
        return resolve_runtime_story_save_exists_cached(*story_save_cache, story_save_exists);
    }
    return story_save_exists();
}

}  // namespace

void handle_runtime_input_phase(RuntimeInputPhaseArgs& args) {
    RuntimeInputPhaseContext context {
        args.window,
        args.edge_state,
        args.app_state,
        args.pause_return_state,
        args.ai_controls_player_paddle,
        args.options,
        args.controls,
        args.audio_settings,
        args.audio_engine,
        args.main_menu_state,
        args.options_menu_state,
        args.pause_menu_state,
        args.menu_state,
        args.paddle_tuning_state,
        args.story_menu_state,
        args.story_intro_state,
        args.story_scene_state,
        args.story_hub_state,
        args.story_runtime,
        args.authored_transition_request,
        args.match_flow,
        args.simulation,
        args.rng,
        args.story_official_games_to_win};

    handle_runtime_global_input(context, args.show_dev_info);

    if (args.menu_input_lockout > 0.0) {
        return;
    }

    switch (context.app_state) {
        case AppState::MainMenu:
            handle_main_menu_branch(context);
            break;
        case AppState::OptionsMenu:
            handle_options_menu_branch(context);
            break;
        case AppState::QuickMatchSetup:
            handle_quick_match_setup_branch(context);
            break;
        case AppState::StoryMenu:
            handle_story_menu_branch(context, resolve_story_save_exists(args.story_save_cache));
            break;
        case AppState::PaddleTuning:
            handle_paddle_tuning_branch(context);
            break;
        case AppState::StoryIntro:
            handle_story_intro_branch(context);
            break;
        case AppState::StoryScene:
            handle_story_scene_branch(context);
            break;
        case AppState::StoryHub:
            handle_story_hub_branch(context);
            break;
        case AppState::Paused:
            handle_paused_branch(context);
            break;
        default:
            break;
    }
}

void handle_runtime_input_phase(
    GLFWwindow* window,
    KeyEdgeState& edge_state,
    AppState& app_state,
    AppState& pause_return_state,
    bool& show_dev_info,
    bool& ai_controls_player_paddle,
    MatchOptions& options,
    ControlBindings& controls,
    AudioSettings& audio_settings,
    AudioEngine& audio_engine,
    MainMenuState& main_menu_state,
    OptionsMenuState& options_menu_state,
    PauseMenuState& pause_menu_state,
    MenuState& menu_state,
    PaddleTuningState& paddle_tuning_state,
    StoryMenuState& story_menu_state,
    StoryIntroState& story_intro_state,
    StorySceneState& story_scene_state,
    StoryHubState& story_hub_state,
    StoryRuntimeState& story_runtime,
    RuntimeAuthoredTransitionRequest& authored_transition_request,
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    std::mt19937_64& rng,
    const double menu_input_lockout,
    const int story_official_games_to_win,
    RuntimeStorySaveExistsCache* story_save_cache) {
    RuntimeInputPhaseArgs args {
        window,
        edge_state,
        app_state,
        pause_return_state,
        show_dev_info,
        ai_controls_player_paddle,
        options,
        controls,
        audio_settings,
        audio_engine,
        main_menu_state,
        options_menu_state,
        pause_menu_state,
        menu_state,
        paddle_tuning_state,
        story_menu_state,
        story_intro_state,
        story_scene_state,
        story_hub_state,
        story_runtime,
        authored_transition_request,
        match_flow,
        simulation,
        rng,
        menu_input_lockout,
        story_official_games_to_win,
        story_save_cache};
    handle_runtime_input_phase(args);
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
