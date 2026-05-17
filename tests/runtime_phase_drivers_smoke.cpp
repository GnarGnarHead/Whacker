#include <cassert>
#include <cstdint>
#include <random>

#include <GLFW/glfw3.h>

#include "runtime_input_phase.hpp"
#include "runtime_phase_drivers.hpp"
#include "runtime_step_input.hpp"
#include "runtime_step_phase.hpp"

namespace {

int g_input_calls = 0;
int g_step_calls = 0;
int g_sample_calls = 0;
bool g_stub_change_app_state = false;
whacker::app::AppState g_stub_next_app_state = whacker::app::AppState::MainMenu;
bool g_stub_change_story_scene = false;
whacker::app::StorySceneId g_stub_next_story_scene_id = whacker::app::StorySceneId::None;
bool g_stub_arm_authored_transition = false;
whacker::app::RuntimeStepInputSnapshot g_stub_sample_snapshot {};
whacker::app::RuntimeStepInputSnapshot g_last_step_snapshot {};
whacker::app::AppState g_last_step_seen_app_state = whacker::app::AppState::MainMenu;
double g_last_input_lockout = -1.0;
int g_last_input_story_official_games_to_win = 0;
whacker::app::RuntimeStorySaveExistsCache* g_last_input_story_save_cache = nullptr;
bool g_last_input_ai_controls_player_paddle = false;
double g_last_step_now = -1.0;
int g_last_step_story_official_games_to_win = 0;
bool g_last_step_ai_controls_player_paddle = false;

void reset_stubs() {
    g_input_calls = 0;
    g_step_calls = 0;
    g_sample_calls = 0;
    g_stub_change_app_state = false;
    g_stub_next_app_state = whacker::app::AppState::MainMenu;
    g_stub_change_story_scene = false;
    g_stub_next_story_scene_id = whacker::app::StorySceneId::None;
    g_stub_arm_authored_transition = false;
    g_stub_sample_snapshot = whacker::app::RuntimeStepInputSnapshot {};
    g_last_step_snapshot = whacker::app::RuntimeStepInputSnapshot {};
    g_last_step_seen_app_state = whacker::app::AppState::MainMenu;
    g_last_input_lockout = -1.0;
    g_last_input_story_official_games_to_win = 0;
    g_last_input_story_save_cache = nullptr;
    g_last_input_ai_controls_player_paddle = false;
    g_last_step_now = -1.0;
    g_last_step_story_official_games_to_win = 0;
    g_last_step_ai_controls_player_paddle = false;
}

struct RuntimePhaseDriversFixture {
    whacker::app::KeyEdgeState edge {};
    whacker::app::AppState app_state;
    whacker::app::AppState pause_return_state;
    bool show_dev_info = false;
    bool ai_controls_player_paddle = false;
    whacker::app::MatchOptions options {};
    whacker::app::ControlBindings controls {};
    whacker::app::AudioSettings audio_settings {};
    whacker::app::AudioEngine audio_engine {};
    whacker::app::MainMenuState main_menu {};
    whacker::app::OptionsMenuState options_menu {};
    whacker::app::PauseMenuState pause_menu {};
    whacker::app::MenuState menu_state {};
    whacker::app::PaddleTuningState paddle_tuning {};
    whacker::app::StoryMenuState story_menu {};
    whacker::app::StoryIntroState story_intro {};
    whacker::app::StorySceneState story_scene {};
    whacker::app::StoryHubState story_hub {};
    whacker::app::StoryRuntimeState story_runtime {};
    whacker::app::RuntimeVisualTransitionState visual_transition {};
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};
    whacker::app::MatchFlowState match_flow {};
    whacker::app::RuntimeAiState left_ai {};
    whacker::app::RuntimeAiState right_ai {};
    whacker::sim::Simulation simulation {};
    std::mt19937_64 rng;
    float type_blip_cooldown = 0.0f;
    std::uint32_t type_blip_pattern_step = 0u;
    whacker::app::RuntimeUpdatePhaseContext context;

    RuntimePhaseDriversFixture(
        const whacker::app::AppState initial_app_state,
        const whacker::app::AppState initial_pause_return_state,
        const std::uint64_t seed)
        : app_state(initial_app_state),
          pause_return_state(initial_pause_return_state),
          rng(seed),
          context {
              nullptr,
              edge,
              app_state,
              pause_return_state,
              show_dev_info,
              ai_controls_player_paddle,
              options,
              controls,
              audio_settings,
              audio_engine,
              main_menu,
              options_menu,
              pause_menu,
              menu_state,
              paddle_tuning,
              story_menu,
              story_intro,
              story_scene,
              story_hub,
              story_runtime,
              visual_transition,
              authored_transition_request,
              match_flow,
              left_ai,
              right_ai,
              simulation,
              rng,
              type_blip_cooldown,
              type_blip_pattern_step} {}

    void run(
        const double now,
        double& accumulator,
        double& menu_input_lockout,
        const double menu_input_lockout_seconds,
        const int story_official_games_to_win,
        whacker::app::RuntimeStorySaveExistsCache* story_save_cache = nullptr) {
        whacker::app::run_runtime_update_phases(
            context,
            now,
            accumulator,
            menu_input_lockout,
            menu_input_lockout_seconds,
            story_official_games_to_win,
            story_save_cache);
    }
};

void test_runtime_phase_drivers_preserve_lockout_and_ai_plan_without_state_change() {
    reset_stubs();

    RuntimePhaseDriversFixture fixture {
        whacker::app::AppState::MainMenu,
        whacker::app::AppState::Playing,
        0x1111ULL};
    fixture.left_ai.plan.has_plan = true;
    fixture.right_ai.plan.has_plan = true;

    g_stub_sample_snapshot.text_fast_held = true;
    fixture.ai_controls_player_paddle = true;
    double accumulator = 0.25;
    double menu_input_lockout = 0.05;
    whacker::app::RuntimeStorySaveExistsCache cache {};

    fixture.run(12.0, accumulator, menu_input_lockout, 0.14, 3, &cache);

    assert(g_input_calls == 1);
    assert(g_sample_calls == 1);
    assert(g_step_calls == 1);
    assert(menu_input_lockout == 0.05);
    assert(fixture.left_ai.plan.has_plan);
    assert(fixture.right_ai.plan.has_plan);
    assert(g_last_step_snapshot.text_fast_held);
    assert(g_last_step_seen_app_state == whacker::app::AppState::MainMenu);
    assert(g_last_input_lockout == 0.05);
    assert(g_last_input_story_official_games_to_win == 3);
    assert(g_last_input_story_save_cache == &cache);
    assert(g_last_input_ai_controls_player_paddle);
    assert(g_last_step_now == 12.0);
    assert(g_last_step_story_official_games_to_win == 3);
    assert(g_last_step_ai_controls_player_paddle);
}

void test_runtime_phase_drivers_apply_lockout_and_reset_ai_plan_on_state_change() {
    reset_stubs();

    RuntimePhaseDriversFixture fixture {
        whacker::app::AppState::MainMenu,
        whacker::app::AppState::Playing,
        0x2222ULL};
    fixture.left_ai.plan.has_plan = true;
    fixture.right_ai.plan.has_plan = true;

    g_stub_change_app_state = true;
    g_stub_next_app_state = whacker::app::AppState::StoryMenu;

    double accumulator = 0.50;
    double menu_input_lockout = 0.01;
    fixture.run(7.0, accumulator, menu_input_lockout, 0.20, 4, nullptr);

    assert(g_input_calls == 1);
    assert(g_sample_calls == 1);
    assert(g_step_calls == 1);
    assert(fixture.app_state == whacker::app::AppState::StoryMenu);
    assert(g_last_step_seen_app_state == whacker::app::AppState::StoryMenu);
    assert(!fixture.left_ai.plan.has_plan);
    assert(!fixture.right_ai.plan.has_plan);
    assert(menu_input_lockout == 0.20);
    assert(g_last_input_story_official_games_to_win == 4);
    assert(g_last_step_story_official_games_to_win == 4);
    assert(!g_last_input_ai_controls_player_paddle);
    assert(!g_last_step_ai_controls_player_paddle);
}

void test_runtime_phase_drivers_story_visual_transition_freezes_input_and_step() {
    reset_stubs();

    RuntimePhaseDriversFixture fixture {
        whacker::app::AppState::StoryHub,
        whacker::app::AppState::Playing,
        0x3333ULL};
    fixture.left_ai.plan.has_plan = true;
    fixture.right_ai.plan.has_plan = true;

    g_stub_change_app_state = true;
    g_stub_next_app_state = whacker::app::AppState::StoryScene;
    g_stub_change_story_scene = true;
    g_stub_next_story_scene_id = whacker::app::StorySceneId::OnboardingEarlyArrival;
    g_stub_arm_authored_transition = true;

    double accumulator = 0.75;
    double menu_input_lockout = 0.0;
    fixture.run(10.0, accumulator, menu_input_lockout, 0.45, 3, nullptr);

    assert(g_input_calls == 1);
    assert(g_sample_calls == 0);
    assert(g_step_calls == 0);
    assert(fixture.visual_transition.active);
    assert(fixture.app_state == whacker::app::AppState::StoryHub);
    assert(!fixture.left_ai.plan.has_plan);
    assert(!fixture.right_ai.plan.has_plan);
    assert(menu_input_lockout == 0.45);

    fixture.run(10.30, accumulator, menu_input_lockout, 0.45, 3, nullptr);
    assert(g_input_calls == 1);
    assert(g_sample_calls == 0);
    assert(g_step_calls == 0);
    assert(fixture.visual_transition.active);
    assert(fixture.app_state == whacker::app::AppState::StoryHub);

    fixture.run(10.60, accumulator, menu_input_lockout, 0.45, 3, nullptr);
    assert(fixture.visual_transition.active);
    assert(g_input_calls == 1);
    assert(g_sample_calls == 0);
    assert(g_step_calls == 0);
    assert(fixture.app_state == whacker::app::AppState::StoryHub);

    fixture.run(10.90, accumulator, menu_input_lockout, 0.45, 3, nullptr);
    assert(fixture.visual_transition.active);
    assert(fixture.app_state == whacker::app::AppState::StoryScene);

    fixture.run(11.20, accumulator, menu_input_lockout, 0.45, 3, nullptr);
    assert(fixture.visual_transition.active);
    assert(fixture.app_state == whacker::app::AppState::StoryScene);

    fixture.run(11.50, accumulator, menu_input_lockout, 0.45, 3, nullptr);
    assert(fixture.visual_transition.active);
    assert(g_input_calls == 1);
    assert(g_sample_calls == 0);
    assert(g_step_calls == 0);
    assert(fixture.app_state == whacker::app::AppState::StoryScene);

    fixture.run(11.80, accumulator, menu_input_lockout, 0.45, 3, nullptr);
    assert(!fixture.visual_transition.active);
    assert(g_input_calls == 1);
    assert(g_sample_calls == 0);
    assert(g_step_calls == 0);
    assert(fixture.app_state == whacker::app::AppState::StoryScene);

    fixture.run(11.82, accumulator, menu_input_lockout, 0.45, 3, nullptr);
    assert(g_input_calls == 2);
    assert(g_sample_calls == 1);
    assert(g_step_calls == 1);
    assert(g_last_step_seen_app_state == whacker::app::AppState::StoryScene);
}

void test_runtime_phase_drivers_scene_swap_transition_requires_authored_request() {
    reset_stubs();

    RuntimePhaseDriversFixture fixture {
        whacker::app::AppState::StoryScene,
        whacker::app::AppState::Playing,
        0x4444ULL};
    fixture.story_scene.id = whacker::app::StorySceneId::OnboardingCoachBrief;

    g_stub_change_story_scene = true;
    g_stub_next_story_scene_id = whacker::app::StorySceneId::PostBenjiAtHomeYoutube;

    double accumulator = 0.10;
    double menu_input_lockout = 0.0;
    fixture.run(8.0, accumulator, menu_input_lockout, 0.33, 3, nullptr);

    assert(!fixture.visual_transition.active);
    assert(fixture.story_scene.id == whacker::app::StorySceneId::PostBenjiAtHomeYoutube);
    assert(menu_input_lockout == 0.0);
    assert(g_input_calls == 1);
    assert(g_sample_calls == 1);
    assert(g_step_calls == 1);

    reset_stubs();
    fixture.story_scene.id = whacker::app::StorySceneId::OnboardingCoachBrief;

    g_stub_change_story_scene = true;
    g_stub_next_story_scene_id = whacker::app::StorySceneId::PostBenjiAtHomeYoutube;
    g_stub_arm_authored_transition = true;

    accumulator = 0.10;
    menu_input_lockout = 0.0;
    fixture.run(9.0, accumulator, menu_input_lockout, 0.33, 3, nullptr);

    assert(fixture.visual_transition.active);
    assert(fixture.story_scene.id == whacker::app::StorySceneId::OnboardingCoachBrief);
    assert(menu_input_lockout == 0.33);
    assert(g_input_calls == 1);
    assert(g_sample_calls == 0);
    assert(g_step_calls == 0);

    fixture.run(9.30, accumulator, menu_input_lockout, 0.33, 3, nullptr);
    assert(fixture.visual_transition.active);
    assert(fixture.story_scene.id == whacker::app::StorySceneId::OnboardingCoachBrief);

    fixture.run(9.60, accumulator, menu_input_lockout, 0.33, 3, nullptr);
    assert(fixture.visual_transition.active);
    assert(fixture.story_scene.id == whacker::app::StorySceneId::OnboardingCoachBrief);

    fixture.run(9.90, accumulator, menu_input_lockout, 0.33, 3, nullptr);
    assert(fixture.visual_transition.active);
    assert(fixture.story_scene.id == whacker::app::StorySceneId::PostBenjiAtHomeYoutube);

    fixture.run(10.20, accumulator, menu_input_lockout, 0.33, 3, nullptr);
    assert(fixture.visual_transition.active);
    assert(fixture.story_scene.id == whacker::app::StorySceneId::PostBenjiAtHomeYoutube);

    fixture.run(10.50, accumulator, menu_input_lockout, 0.33, 3, nullptr);
    assert(fixture.visual_transition.active);
    assert(fixture.story_scene.id == whacker::app::StorySceneId::PostBenjiAtHomeYoutube);

    fixture.run(10.80, accumulator, menu_input_lockout, 0.33, 3, nullptr);
    assert(!fixture.visual_transition.active);
    assert(fixture.story_scene.id == whacker::app::StorySceneId::PostBenjiAtHomeYoutube);

    fixture.run(10.82, accumulator, menu_input_lockout, 0.33, 3, nullptr);
    assert(g_input_calls == 2);
    assert(g_sample_calls == 1);
    assert(g_step_calls == 1);
}

void test_runtime_phase_drivers_pending_wipe_waits_for_scene_materialization_delta() {
    reset_stubs();

    RuntimePhaseDriversFixture fixture {
        whacker::app::AppState::StoryScene,
        whacker::app::AppState::Playing,
        0x5555ULL};
    fixture.story_scene.id = whacker::app::StorySceneId::OnboardingCoachBrief;
    fixture.story_runtime.onboarding_scene_pending = true;
    fixture.story_runtime.onboarding_step = whacker::app::StoryOnboardingStep::AtHomeYoutubeScene;

    double accumulator = 0.10;
    double menu_input_lockout = 0.0;
    fixture.run(12.0, accumulator, menu_input_lockout, 0.33, 3, nullptr);

    assert(!fixture.visual_transition.active);
    assert(fixture.story_runtime.onboarding_scene_pending);
    assert(fixture.story_scene.id == whacker::app::StorySceneId::OnboardingCoachBrief);
    assert(menu_input_lockout == 0.0);
    assert(g_input_calls == 1);
    assert(g_sample_calls == 1);
    assert(g_step_calls == 1);
}

}  // namespace

namespace whacker::app {

AudioEngine::~AudioEngine() = default;

bool AudioEngine::init() {
    return true;
}

void AudioEngine::shutdown() {}

bool AudioEngine::available() const {
    return true;
}

void AudioEngine::set_settings(const AudioSettings& /*settings*/) {}

AudioSettings AudioEngine::settings() const {
    return AudioSettings {};
}

void AudioEngine::push_event(const AudioEventId /*event_id*/) {}

void AudioEngine::push_paddle_hit(const PaddleHitAudioParams& /*params*/) {}

void AudioEngine::push_wall_hit(const WallHitAudioParams& /*params*/) {}

AudioSettings clamp_audio_settings(const AudioSettings settings) {
    return settings;
}

void handle_runtime_input_phase(RuntimeInputPhaseArgs& args) {
    ++::g_input_calls;
    ::g_last_input_lockout = args.menu_input_lockout;
    ::g_last_input_story_official_games_to_win = args.story_official_games_to_win;
    ::g_last_input_story_save_cache = args.story_save_cache;
    ::g_last_input_ai_controls_player_paddle = args.ai_controls_player_paddle;
    const AppState app_state_before = args.app_state;
    const StorySceneState story_scene_before = args.story_scene_state;
    if (::g_stub_change_app_state) {
        args.app_state = ::g_stub_next_app_state;
        ::g_stub_change_app_state = false;
    }
    if (::g_stub_change_story_scene) {
        args.story_scene_state.id = ::g_stub_next_story_scene_id;
        ::g_stub_change_story_scene = false;
    }
    if (::g_stub_arm_authored_transition) {
        const StorySceneState* from_scene_ptr =
            app_state_before == AppState::StoryScene ? &story_scene_before : nullptr;
        const StorySceneState* to_scene_ptr =
            args.app_state == AppState::StoryScene ? &args.story_scene_state : nullptr;
        (void)arm_authored_star_wipe_transition(
            args.authored_transition_request,
            app_state_before,
            from_scene_ptr,
            args.app_state,
            to_scene_ptr);
        ::g_stub_arm_authored_transition = false;
    }
}

RuntimeStepInputSnapshot sample_runtime_step_input(GLFWwindow* /*window*/) {
    ++::g_sample_calls;
    return ::g_stub_sample_snapshot;
}

void handle_runtime_step_phase(RuntimeStepPhaseArgs& args) {
    ++::g_step_calls;
    ::g_last_step_now = args.now;
    ::g_last_step_snapshot = args.step_input;
    ::g_last_step_seen_app_state = args.app_state;
    ::g_last_step_story_official_games_to_win = args.story_official_games_to_win;
    ::g_last_step_ai_controls_player_paddle = args.ai_controls_player_paddle;
}

}  // namespace whacker::app

int main() {
    test_runtime_phase_drivers_preserve_lockout_and_ai_plan_without_state_change();
    test_runtime_phase_drivers_apply_lockout_and_reset_ai_plan_on_state_change();
    test_runtime_phase_drivers_story_visual_transition_freezes_input_and_step();
    test_runtime_phase_drivers_scene_swap_transition_requires_authored_request();
    test_runtime_phase_drivers_pending_wipe_waits_for_scene_materialization_delta();
    return 0;
}
