#include <cstdlib>
#include <cstdint>

#include <GLFW/glfw3.h>

#include "runtime_helpers.hpp"
#include "runtime_loop_state.hpp"

namespace {

bool audio_settings_equal(const whacker::app::AudioSettings& a, const whacker::app::AudioSettings& b) {
    return
        (a.master_volume == b.master_volume) &&
        (a.music_volume == b.music_volume) &&
        (a.sfx_volume == b.sfx_volume) &&
        (a.mute == b.mute);
}

void require(const bool condition) {
    if (!condition) {
        std::abort();
    }
}

struct BootstrapStubState {
    int load_menu_settings_calls = 0;
    int clamp_audio_settings_calls = 0;
    int audio_engine_init_calls = 0;
    int audio_engine_set_settings_calls = 0;
    whacker::app::AudioSettings loaded_audio_settings {};
    whacker::app::AudioSettings clamped_audio_settings {};
    whacker::app::AudioSettings last_clamp_input {};
    whacker::app::AudioSettings last_set_settings_input {};
    whacker::app::MatchOptions loaded_options {};
    whacker::app::ControlBindings loaded_controls {};

    void reset() {
        *this = BootstrapStubState {};
    }
};

BootstrapStubState g_bootstrap_stubs {};

struct RuntimeLoopStateFixture {
    whacker::sim::Simulation simulation {};
    whacker::app::RuntimeLoopState loop_state;

    explicit RuntimeLoopStateFixture(const std::uint64_t seed)
        : loop_state(seed) {}
};

void test_initialize_runtime_loop_state_loads_clamps_and_applies_audio_settings() {
    g_bootstrap_stubs.reset();

    g_bootstrap_stubs.loaded_options.left_mode = whacker::app::PaddleMode::AI;
    g_bootstrap_stubs.loaded_options.right_mode = whacker::app::PaddleMode::Human;
    g_bootstrap_stubs.loaded_audio_settings.master_volume = 91;
    g_bootstrap_stubs.loaded_audio_settings.music_volume = 17;
    g_bootstrap_stubs.loaded_audio_settings.sfx_volume = 88;
    g_bootstrap_stubs.loaded_audio_settings.mute = true;
    g_bootstrap_stubs.clamped_audio_settings.master_volume = 70;
    g_bootstrap_stubs.clamped_audio_settings.music_volume = 55;
    g_bootstrap_stubs.clamped_audio_settings.sfx_volume = 44;
    g_bootstrap_stubs.clamped_audio_settings.mute = false;

    whacker::app::RuntimeLoopState loop_state(0xABCDULL);
    whacker::app::initialize_runtime_loop_state(loop_state);

    require(g_bootstrap_stubs.load_menu_settings_calls == 1);
    require(g_bootstrap_stubs.clamp_audio_settings_calls == 1);
    require(g_bootstrap_stubs.audio_engine_init_calls == 1);
    require(g_bootstrap_stubs.audio_engine_set_settings_calls == 1);
    require(audio_settings_equal(g_bootstrap_stubs.last_clamp_input, g_bootstrap_stubs.loaded_audio_settings));
    require(audio_settings_equal(loop_state.audio_settings, g_bootstrap_stubs.clamped_audio_settings));
    require(audio_settings_equal(
        g_bootstrap_stubs.last_set_settings_input,
        g_bootstrap_stubs.clamped_audio_settings));
    require(loop_state.options.left_mode == whacker::app::PaddleMode::AI);
    require(loop_state.options.right_mode == whacker::app::PaddleMode::Human);
}

void test_make_runtime_update_phase_context_binds_loop_state_references() {
    RuntimeLoopStateFixture fixture(0x1234ULL);

    auto context = whacker::app::make_runtime_update_phase_context(nullptr, fixture.loop_state, fixture.simulation);

    context.app_state = whacker::app::AppState::StoryMenu;
    context.pause_return_state = whacker::app::AppState::StoryIntro;
    context.show_dev_info = true;
    context.ai_controls_player_paddle = true;
    context.type_blip_cooldown = 0.42f;
    context.type_blip_pattern_step = 9u;
    context.visual_transition.active = true;
    context.left_ai_state.plan.has_plan = true;
    context.right_ai_state.plan.has_plan = true;

    require(fixture.loop_state.app_state == whacker::app::AppState::StoryMenu);
    require(fixture.loop_state.pause_return_state == whacker::app::AppState::StoryIntro);
    require(fixture.loop_state.show_dev_info);
    require(fixture.loop_state.ai_controls_player_paddle);
    require(fixture.loop_state.type_blip_cooldown == 0.42f);
    require(fixture.loop_state.type_blip_pattern_step == 9u);
    require(fixture.loop_state.visual_transition.active);
    require(fixture.loop_state.left_ai_state.plan.has_plan);
    require(fixture.loop_state.right_ai_state.plan.has_plan);
    require(&context.simulation == &fixture.simulation);
    require(&context.rng == &fixture.loop_state.rng);
    require(&context.visual_transition == &fixture.loop_state.visual_transition);
}

void test_make_runtime_render_driver_context_binds_loop_state_references() {
    RuntimeLoopStateFixture fixture(0x4321ULL);

    auto context = whacker::app::make_runtime_render_driver_context(nullptr, fixture.loop_state, fixture.simulation);

    context.app_state = whacker::app::AppState::Paused;
    context.pause_return_state = whacker::app::AppState::Playing;
    context.show_dev_info = true;
    context.ai_controls_player_paddle = true;
    fixture.loop_state.story_hub_state.selected_row = whacker::app::StoryHubRowBack;

    require(fixture.loop_state.app_state == whacker::app::AppState::Paused);
    require(fixture.loop_state.pause_return_state == whacker::app::AppState::Playing);
    require(fixture.loop_state.show_dev_info);
    require(fixture.loop_state.ai_controls_player_paddle);
    require(context.story_hub_state.selected_row == whacker::app::StoryHubRowBack);
    require(&context.simulation == &fixture.simulation);
    require(&context.story_hub_state == &fixture.loop_state.story_hub_state);
    require(&context.visual_transition == &fixture.loop_state.visual_transition);
    require(context.main_menu_row_name_fn == whacker::app::main_menu_row_name);
    require(context.options_menu_row_name_fn == whacker::app::options_menu_row_name);
    require(context.quick_row_name_fn == whacker::app::row_name);
    require(context.story_menu_row_name_fn == whacker::app::story_menu_row_name);
    require(context.story_intro_phase_name_fn == whacker::app::story_intro_phase_name);
    require(context.story_hub_row_name_fn == whacker::app::story_hub_row_name);
    require(context.mode_name_fn == whacker::app::mode_name);
    require(context.style_name_fn == whacker::app::ai_style_name);
    require(context.story_match_kind_name_fn == whacker::app::story_match_kind_name);
}

}  // namespace

namespace whacker::app {

AudioEngine::~AudioEngine() = default;

bool AudioEngine::init() {
    ++::g_bootstrap_stubs.audio_engine_init_calls;
    return true;
}

void AudioEngine::shutdown() {}

bool AudioEngine::available() const {
    return true;
}

void AudioEngine::set_settings(const AudioSettings& settings) {
    ++::g_bootstrap_stubs.audio_engine_set_settings_calls;
    ::g_bootstrap_stubs.last_set_settings_input = settings;
}

AudioSettings AudioEngine::settings() const {
    return AudioSettings {};
}

void AudioEngine::push_event(const AudioEventId /*event_id*/) {}

void AudioEngine::push_paddle_hit(const PaddleHitAudioParams& /*params*/) {}

void AudioEngine::push_wall_hit(const WallHitAudioParams& /*params*/) {}

AudioSettings clamp_audio_settings(const AudioSettings settings) {
    ++::g_bootstrap_stubs.clamp_audio_settings_calls;
    ::g_bootstrap_stubs.last_clamp_input = settings;
    return ::g_bootstrap_stubs.clamped_audio_settings;
}

void load_menu_settings(MatchOptions& options, ControlBindings& controls, AudioSettings& audio_settings) {
    ++::g_bootstrap_stubs.load_menu_settings_calls;
    options = ::g_bootstrap_stubs.loaded_options;
    controls = ::g_bootstrap_stubs.loaded_controls;
    audio_settings = ::g_bootstrap_stubs.loaded_audio_settings;
}

void save_menu_settings(
    const MatchOptions& /*options*/,
    const ControlBindings& /*controls*/,
    const AudioSettings& /*audio_settings*/) {}

}  // namespace whacker::app

int main() {
    test_initialize_runtime_loop_state_loads_clamps_and_applies_audio_settings();
    test_make_runtime_update_phase_context_binds_loop_state_references();
    test_make_runtime_render_driver_context_binds_loop_state_references();
    return 0;
}
