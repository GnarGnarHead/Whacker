#include <cassert>
#include <cstdint>
#include <random>

#include <GLFW/glfw3.h>

#include "runtime_render_drivers.hpp"
#include "runtime_story_save_cache.hpp"

namespace {

int g_title_calls = 0;
int g_render_calls = 0;
int g_policy_calls = 0;
int g_transition_overlay_calls = 0;
whacker::app::AppState g_last_title_app_state = whacker::app::AppState::MainMenu;
whacker::app::AppState g_last_render_app_state = whacker::app::AppState::MainMenu;
bool g_last_render_pause_policy_present = false;
whacker::app::MatchExitPolicy g_last_render_pause_policy {};
whacker::app::RuntimeStorySaveExistsCache* g_last_render_story_save_cache = nullptr;
bool g_last_render_ai_controls_player_paddle = false;
bool g_last_transition_overlay_active = false;
const whacker::sim::Simulation* g_policy_seen_simulation = nullptr;
whacker::app::AppState g_policy_seen_app_state = whacker::app::AppState::MainMenu;
whacker::app::AppState g_policy_seen_pause_return_state = whacker::app::AppState::MainMenu;
const whacker::app::MatchFlowState* g_policy_seen_match_flow = nullptr;
const whacker::app::StoryRuntimeState* g_policy_seen_story_runtime = nullptr;
const whacker::app::StoryIntroState* g_policy_seen_story_intro = nullptr;
whacker::app::MatchExitPolicy g_stub_policy {};

void reset_stubs() {
    g_title_calls = 0;
    g_render_calls = 0;
    g_policy_calls = 0;
    g_transition_overlay_calls = 0;
    g_last_title_app_state = whacker::app::AppState::MainMenu;
    g_last_render_app_state = whacker::app::AppState::MainMenu;
    g_last_render_pause_policy_present = false;
    g_last_render_pause_policy = whacker::app::MatchExitPolicy {};
    g_last_render_story_save_cache = nullptr;
    g_last_render_ai_controls_player_paddle = false;
    g_last_transition_overlay_active = false;
    g_policy_seen_simulation = nullptr;
    g_policy_seen_app_state = whacker::app::AppState::MainMenu;
    g_policy_seen_pause_return_state = whacker::app::AppState::MainMenu;
    g_policy_seen_match_flow = nullptr;
    g_policy_seen_story_runtime = nullptr;
    g_policy_seen_story_intro = nullptr;
    g_stub_policy = whacker::app::MatchExitPolicy {};
}

const char* stub_row_name(const int /*row*/) {
    return "row";
}

const char* stub_intro_phase_name(const whacker::app::StoryIntroPhase /*phase*/) {
    return "phase";
}

const char* stub_mode_name(const whacker::app::PaddleMode /*mode*/) {
    return "mode";
}

const char* stub_style_name(const whacker::app::AiStyle /*style*/) {
    return "style";
}

const char* stub_match_kind_name(const whacker::app::StoryMatchKind /*kind*/) {
    return "kind";
}

struct RuntimeRenderDriversFixture {
    whacker::sim::Simulation simulation {};
    whacker::app::MatchFlowState match_flow {};
    bool show_dev_info = false;
    bool ai_controls_player_paddle = false;
    whacker::app::AppState app_state;
    whacker::app::AppState pause_return_state;
    whacker::app::MainMenuState main_menu_state {};
    whacker::app::OptionsMenuState options_menu_state {};
    whacker::app::PauseMenuState pause_menu_state {};
    whacker::app::MenuState menu_state {};
    whacker::app::PaddleTuningState paddle_tuning_state {};
    whacker::app::StoryMenuState story_menu_state {};
    whacker::app::StoryIntroState story_intro_state {};
    whacker::app::StorySceneState story_scene_state {};
    whacker::app::StoryHubState story_hub_state {};
    whacker::app::StoryRuntimeState story_runtime {};
    whacker::app::RuntimeVisualTransitionState visual_transition {};
    whacker::app::MatchOptions options {};
    whacker::app::ControlBindings controls {};
    whacker::app::AudioSettings audio_settings {};
    whacker::app::RuntimeRenderDriverContext context;

    RuntimeRenderDriversFixture(
        const whacker::app::AppState initial_app_state,
        const whacker::app::AppState initial_pause_return_state)
        : app_state(initial_app_state),
          pause_return_state(initial_pause_return_state),
          context {
              nullptr,
              simulation,
              match_flow,
              show_dev_info,
              ai_controls_player_paddle,
              app_state,
              pause_return_state,
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
              visual_transition,
              options,
              controls,
              audio_settings,
              stub_row_name,
              stub_row_name,
              stub_row_name,
              stub_row_name,
              stub_intro_phase_name,
              stub_row_name,
              stub_mode_name,
              stub_style_name,
              stub_match_kind_name} {}

    void run(double& title_cooldown, whacker::app::RuntimeStorySaveExistsCache* story_save_cache = nullptr) {
        whacker::app::run_runtime_render_phases(context, title_cooldown, story_save_cache);
    }
};

void test_runtime_render_drivers_title_update_path_resets_cooldown_and_renders() {
    reset_stubs();

    RuntimeRenderDriversFixture fixture {
        whacker::app::AppState::MainMenu,
        whacker::app::AppState::Playing};
    fixture.ai_controls_player_paddle = true;
    double title_cooldown = 0.12;
    whacker::app::RuntimeStorySaveExistsCache story_save_cache {};

    fixture.run(title_cooldown, &story_save_cache);

    assert(g_title_calls == 1);
    assert(title_cooldown == 0.0);
    assert(g_policy_calls == 0);
    assert(g_render_calls == 1);
    assert(g_transition_overlay_calls == 1);
    assert(!g_last_render_pause_policy_present);
    assert(g_last_title_app_state == whacker::app::AppState::MainMenu);
    assert(g_last_render_app_state == whacker::app::AppState::MainMenu);
    assert(g_last_render_story_save_cache == &story_save_cache);
    assert(g_last_render_ai_controls_player_paddle);
    assert(!g_last_transition_overlay_active);
}

void test_runtime_render_drivers_paused_path_forwards_pause_exit_policy() {
    reset_stubs();

    RuntimeRenderDriversFixture fixture {
        whacker::app::AppState::Paused,
        whacker::app::AppState::StoryIntro};
    double title_cooldown = 0.05;

    g_stub_policy.has_exit_option = true;
    g_stub_policy.can_exit_now = false;
    g_stub_policy.requires_confirmation = true;
    g_stub_policy.action = whacker::app::MatchExitAction::ExitStoryMatch;
    g_stub_policy.story_end_reason = whacker::app::StoryMatchEndReason::Forfeit;

    fixture.run(title_cooldown);

    assert(g_title_calls == 0);
    assert(title_cooldown == 0.05);
    assert(g_policy_calls == 1);
    assert(g_render_calls == 1);
    assert(g_transition_overlay_calls == 1);
    assert(g_last_render_pause_policy_present);
    assert(g_last_render_pause_policy.has_exit_option);
    assert(!g_last_render_pause_policy.can_exit_now);
    assert(g_last_render_pause_policy.requires_confirmation);
    assert(g_last_render_pause_policy.action == whacker::app::MatchExitAction::ExitStoryMatch);
    assert(g_last_render_pause_policy.story_end_reason == whacker::app::StoryMatchEndReason::Forfeit);
    assert(g_last_render_story_save_cache == nullptr);
    assert(!g_last_render_ai_controls_player_paddle);
    assert(!g_last_transition_overlay_active);
    assert(g_policy_seen_simulation == &fixture.simulation);
    assert(g_policy_seen_app_state == whacker::app::AppState::Paused);
    assert(g_policy_seen_pause_return_state == whacker::app::AppState::StoryIntro);
    assert(g_policy_seen_match_flow == &fixture.match_flow);
    assert(g_policy_seen_story_runtime == &fixture.story_runtime);
    assert(g_policy_seen_story_intro == &fixture.story_intro_state);
}

}  // namespace

namespace whacker::app {

void update_window_title(
    GLFWwindow* /*window*/,
    const whacker::sim::Simulation& /*simulation*/,
    const MatchOptions& /*options*/,
    const OptionsMenuState& /*options_menu_state*/,
    const MainMenuState& /*main_menu_state*/,
    const MenuState& /*menu_state*/,
    const StoryMenuState& /*story_menu_state*/,
    const StoryIntroState& /*story_intro_state*/,
    const StoryRuntimeState& /*story_runtime*/,
    const StoryHubState& /*story_hub_state*/,
    const AppState app_state,
    IntNameFn /*main_menu_row_name_fn*/,
    IntNameFn /*options_menu_row_name_fn*/,
    IntNameFn /*quick_row_name_fn*/,
    IntNameFn /*story_menu_row_name_fn*/,
    IntroPhaseNameFn /*story_intro_phase_name_fn*/,
    IntNameFn /*story_hub_row_name_fn*/,
    ModeNameFn /*mode_name_fn*/,
    StyleNameFn /*style_name_fn*/,
    MatchKindNameFn /*match_kind_name_fn*/) {
    ++::g_title_calls;
    ::g_last_title_app_state = app_state;
}

MatchExitPolicy compute_runtime_match_exit_policy(
    const whacker::sim::Simulation& simulation,
    const AppState app_state,
    const AppState pause_return_state,
    const MatchFlowState& match_flow,
    const StoryRuntimeState& story_runtime,
    const StoryIntroState& story_intro_state) {
    ++::g_policy_calls;
    ::g_policy_seen_simulation = &simulation;
    ::g_policy_seen_app_state = app_state;
    ::g_policy_seen_pause_return_state = pause_return_state;
    ::g_policy_seen_match_flow = &match_flow;
    ::g_policy_seen_story_runtime = &story_runtime;
    ::g_policy_seen_story_intro = &story_intro_state;
    return ::g_stub_policy;
}

void render_runtime_frame(
    GLFWwindow* /*window*/,
    const whacker::sim::Simulation& /*simulation*/,
    const MatchFlowState& /*match_flow*/,
    const bool /*show_dev_info*/,
    const bool ai_controls_player_paddle,
    const AppState app_state,
    const MainMenuState& /*main_menu_state*/,
    const OptionsMenuState& /*options_menu_state*/,
    const PauseMenuState& /*pause_menu_state*/,
    const MenuState& /*menu_state*/,
    const PaddleTuningState& /*paddle_tuning_state*/,
    const StoryMenuState& /*story_menu_state*/,
    const StoryIntroState& /*story_intro_state*/,
    const StorySceneState& /*story_scene_state*/,
    const StoryHubState& /*story_hub_state*/,
    const StoryRuntimeState& /*story_runtime*/,
    const MatchOptions& /*options*/,
    const ControlBindings& /*controls*/,
    const AudioSettings& /*audio_settings*/,
    const MatchExitPolicy* pause_exit_policy,
    RuntimeStorySaveExistsCache* story_save_cache) {
    ++::g_render_calls;
    ::g_last_render_app_state = app_state;
    ::g_last_render_pause_policy_present = (pause_exit_policy != nullptr);
    ::g_last_render_story_save_cache = story_save_cache;
    ::g_last_render_ai_controls_player_paddle = ai_controls_player_paddle;
    if (pause_exit_policy != nullptr) {
        ::g_last_render_pause_policy = *pause_exit_policy;
    } else {
        ::g_last_render_pause_policy = MatchExitPolicy {};
    }
}

void render_visual_transition_overlay(
    GLFWwindow* /*window*/,
    const RuntimeVisualTransitionState& transition) {
    ++::g_transition_overlay_calls;
    ::g_last_transition_overlay_active = transition.active;
}

}  // namespace whacker::app

int main() {
    test_runtime_render_drivers_title_update_path_resets_cooldown_and_renders();
    test_runtime_render_drivers_paused_path_forwards_pause_exit_policy();
    return 0;
}
