#include <cassert>
#include <random>
#include <string>

#include <GLFW/glfw3.h>

#include "story_flow.hpp"
#include "story_text.hpp"

namespace {

bool g_stub_confirm_press = false;
bool g_stub_menu_up_press = false;
bool g_stub_menu_down_press = false;
bool g_begin_new_story_intro_called = false;
int g_load_career_calls = 0;
std::string g_next_load_error;
whacker::app::StoryCareerData g_next_loaded_career {};

void reset_stubs() {
    g_stub_confirm_press = false;
    g_stub_menu_up_press = false;
    g_stub_menu_down_press = false;
    g_begin_new_story_intro_called = false;
    g_load_career_calls = 0;
    g_next_load_error.clear();
    g_next_loaded_career = whacker::app::StoryCareerData {};
}

bool fake_load_career_failure(whacker::app::StoryCareerData& /*loaded*/, std::string* error_out) {
    ++g_load_career_calls;
    if (error_out != nullptr) {
        *error_out = g_next_load_error;
    }
    return false;
}

bool fake_load_career_success(whacker::app::StoryCareerData& loaded, std::string* error_out) {
    ++g_load_career_calls;
    loaded = g_next_loaded_career;
    if (error_out != nullptr) {
        error_out->clear();
    }
    return true;
}

whacker::app::StoryRuntimeState make_runtime_sentinel() {
    whacker::app::StoryRuntimeState runtime {};
    runtime.career_loaded = true;
    runtime.career.week = 7;
    runtime.career.player_name = "SENTINEL";
    runtime.career.joined_club = true;
    runtime.career.onboarding_step = whacker::app::StoryOnboardingStep::CoachBriefScene;
    runtime.career.onboarding_style_hint = whacker::app::StoryIntroStyleHint::Power;
    runtime.career.onboarding_performance_hint = whacker::app::StoryIntroPerformanceHint::BigWin;
    runtime.career.onboarding_aya_feedback_available = true;
    runtime.career.onboarding_aya_feedback_from_loss = true;
    runtime.career.onboarding_aya_feedback_hint = whacker::app::StoryIntroStyleHint::Technical;
    runtime.career.onboarding_aya_forfeited = true;
    runtime.onboarding_scene_pending = true;
    runtime.onboarding_step = whacker::app::StoryOnboardingStep::EntryRetryScene;
    runtime.onboarding_style_hint = whacker::app::StoryIntroStyleHint::Spin;
    runtime.onboarding_performance_hint = whacker::app::StoryIntroPerformanceHint::CloseLoss;
    runtime.onboarding_aya_feedback_available = true;
    runtime.onboarding_aya_feedback_from_loss = true;
    runtime.onboarding_aya_feedback_hint = whacker::app::StoryIntroStyleHint::Power;
    runtime.onboarding_aya_forfeited = true;
    runtime.active_match = whacker::app::StoryMatchKind::Official;
    runtime.official_games_left = 2;
    runtime.official_games_right = 1;
    runtime.post_forfeit_scene_pending = true;
    return runtime;
}

void assert_runtime_unchanged(
    const whacker::app::StoryRuntimeState& before,
    const whacker::app::StoryRuntimeState& after) {
    static_cast<void>(before);
    static_cast<void>(after);
    assert(after.career_loaded == before.career_loaded);
    assert(after.career.week == before.career.week);
    assert(after.career.player_name == before.career.player_name);
    assert(after.career.joined_club == before.career.joined_club);
    assert(after.career.onboarding_step == before.career.onboarding_step);
    assert(after.career.onboarding_style_hint == before.career.onboarding_style_hint);
    assert(after.career.onboarding_performance_hint == before.career.onboarding_performance_hint);
    assert(after.career.onboarding_aya_feedback_available == before.career.onboarding_aya_feedback_available);
    assert(after.career.onboarding_aya_feedback_from_loss == before.career.onboarding_aya_feedback_from_loss);
    assert(after.career.onboarding_aya_feedback_hint == before.career.onboarding_aya_feedback_hint);
    assert(after.career.onboarding_aya_forfeited == before.career.onboarding_aya_forfeited);
    assert(after.onboarding_scene_pending == before.onboarding_scene_pending);
    assert(after.onboarding_step == before.onboarding_step);
    assert(after.onboarding_style_hint == before.onboarding_style_hint);
    assert(after.onboarding_performance_hint == before.onboarding_performance_hint);
    assert(after.onboarding_aya_feedback_available == before.onboarding_aya_feedback_available);
    assert(after.onboarding_aya_feedback_from_loss == before.onboarding_aya_feedback_from_loss);
    assert(after.onboarding_aya_feedback_hint == before.onboarding_aya_feedback_hint);
    assert(after.onboarding_aya_forfeited == before.onboarding_aya_forfeited);
    assert(after.active_match == before.active_match);
    assert(after.official_games_left == before.official_games_left);
    assert(after.official_games_right == before.official_games_right);
    assert(after.post_forfeit_scene_pending == before.post_forfeit_scene_pending);
}

struct StoryMenuFixture {
    whacker::app::KeyEdgeState edge_state {};
    whacker::app::StoryMenuState menu_state {};
    whacker::app::StoryRuntimeState runtime = make_runtime_sentinel();
    whacker::app::StoryHubState hub_state {};
    whacker::app::StoryIntroState intro_state {};
    whacker::app::MatchOptions options {};
    whacker::app::ControlBindings controls {};
    whacker::app::MatchFlowState match_flow {};
    whacker::sim::Simulation simulation {};
    whacker::app::AppState app_state = whacker::app::AppState::StoryMenu;

    void run(
        const bool has_save,
        const whacker::app::StoryLoadCareerFn load_career_fn,
        const whacker::app::StoryResetCareerFn reset_career_fn) {
        whacker::app::handle_story_menu_input(
            nullptr,
            edge_state,
            menu_state,
            runtime,
            hub_state,
            intro_state,
            options,
            controls,
            match_flow,
            simulation,
            app_state,
            has_save,
            load_career_fn,
            reset_career_fn);
    }
};

void test_continue_with_null_loader_sets_failed_feedback_and_keeps_state() {
    reset_stubs();
    g_stub_confirm_press = true;

    StoryMenuFixture fixture {};
    fixture.menu_state.selected_row = whacker::app::StoryMenuRowContinue;
    fixture.menu_state.confirm_selected = 1;
    const whacker::app::StoryRuntimeState runtime_before = fixture.runtime;
    fixture.hub_state.selected_row = whacker::app::StoryHubRowTrainingMatch;
    fixture.hub_state.feedback_line_1 = "existing feedback 1";
    fixture.hub_state.feedback_line_2 = "existing feedback 2";

    fixture.run(true, nullptr, nullptr);

    assert(g_load_career_calls == 0);
    assert(!g_begin_new_story_intro_called);
    assert(fixture.app_state == whacker::app::AppState::StoryMenu);
    assert(fixture.menu_state.selected_row == whacker::app::StoryMenuRowContinue);
    assert(fixture.menu_state.confirm_selected == 1);
    assert(fixture.hub_state.selected_row == whacker::app::StoryHubRowTrainingMatch);
    assert(fixture.hub_state.feedback_line_1 == whacker::app::story_text::continue_failed_feedback_line_1());
    assert(fixture.hub_state.feedback_line_2.empty());
    assert_runtime_unchanged(runtime_before, fixture.runtime);
}

void test_continue_with_load_failure_sets_error_feedback_and_keeps_state() {
    reset_stubs();
    g_stub_confirm_press = true;
    g_next_load_error = "Save parse error";

    StoryMenuFixture fixture {};
    fixture.menu_state.selected_row = whacker::app::StoryMenuRowContinue;
    const whacker::app::StoryRuntimeState runtime_before = fixture.runtime;
    fixture.hub_state.selected_row = whacker::app::StoryHubRowOfficialMatch;
    fixture.hub_state.feedback_line_1 = "old line 1";
    fixture.hub_state.feedback_line_2 = "old line 2";

    fixture.run(true, fake_load_career_failure, nullptr);

    assert(g_load_career_calls == 1);
    assert(!g_begin_new_story_intro_called);
    assert(fixture.app_state == whacker::app::AppState::StoryMenu);
    assert(fixture.menu_state.selected_row == whacker::app::StoryMenuRowContinue);
    assert(fixture.hub_state.selected_row == whacker::app::StoryHubRowOfficialMatch);
    assert(fixture.hub_state.feedback_line_1 == whacker::app::story_text::continue_failed_feedback_line_1());
    assert(fixture.hub_state.feedback_line_2 == "Save parse error");
    assert_runtime_unchanged(runtime_before, fixture.runtime);
}

void test_continue_without_save_ignores_confirm_and_keeps_state() {
    reset_stubs();
    g_stub_confirm_press = true;

    StoryMenuFixture fixture {};
    fixture.menu_state.selected_row = whacker::app::StoryMenuRowContinue;
    const whacker::app::StoryRuntimeState runtime_before = fixture.runtime;
    fixture.hub_state.selected_row = whacker::app::StoryHubRowTrainingMatch;
    fixture.hub_state.feedback_line_1 = "keep line 1";
    fixture.hub_state.feedback_line_2 = "keep line 2";

    fixture.run(false, fake_load_career_failure, nullptr);

    assert(g_load_career_calls == 0);
    assert(!g_begin_new_story_intro_called);
    assert(fixture.app_state == whacker::app::AppState::StoryMenu);
    assert(fixture.menu_state.selected_row == whacker::app::StoryMenuRowContinue);
    assert(fixture.hub_state.selected_row == whacker::app::StoryHubRowTrainingMatch);
    assert(fixture.hub_state.feedback_line_1 == "keep line 1");
    assert(fixture.hub_state.feedback_line_2 == "keep line 2");
    assert_runtime_unchanged(runtime_before, fixture.runtime);
}

void test_story_menu_row_wraps_up_from_continue_to_back() {
    reset_stubs();
    g_stub_menu_up_press = true;

    StoryMenuFixture fixture {};
    fixture.menu_state.selected_row = whacker::app::StoryMenuRowContinue;
    const whacker::app::StoryRuntimeState runtime_before = fixture.runtime;
    fixture.hub_state.feedback_line_1 = "wrap line 1";
    fixture.hub_state.feedback_line_2 = "wrap line 2";

    fixture.run(false, fake_load_career_failure, nullptr);

    assert(g_load_career_calls == 0);
    assert(!g_begin_new_story_intro_called);
    assert(fixture.app_state == whacker::app::AppState::StoryMenu);
    assert(fixture.menu_state.selected_row == whacker::app::StoryMenuRowBack);
    assert(fixture.hub_state.feedback_line_1 == "wrap line 1");
    assert(fixture.hub_state.feedback_line_2 == "wrap line 2");
    assert_runtime_unchanged(runtime_before, fixture.runtime);
}

void test_story_menu_row_wraps_down_from_back_to_continue() {
    reset_stubs();
    g_stub_menu_down_press = true;

    StoryMenuFixture fixture {};
    fixture.menu_state.selected_row = whacker::app::StoryMenuRowBack;
    const whacker::app::StoryRuntimeState runtime_before = fixture.runtime;
    fixture.hub_state.feedback_line_1 = "wrap line 1";
    fixture.hub_state.feedback_line_2 = "wrap line 2";

    fixture.run(false, fake_load_career_failure, nullptr);

    assert(g_load_career_calls == 0);
    assert(!g_begin_new_story_intro_called);
    assert(fixture.app_state == whacker::app::AppState::StoryMenu);
    assert(fixture.menu_state.selected_row == whacker::app::StoryMenuRowContinue);
    assert(fixture.hub_state.feedback_line_1 == "wrap line 1");
    assert(fixture.hub_state.feedback_line_2 == "wrap line 2");
    assert_runtime_unchanged(runtime_before, fixture.runtime);
}

void test_story_menu_back_confirm_routes_to_main_menu_without_state_mutation() {
    reset_stubs();
    g_stub_confirm_press = true;

    StoryMenuFixture fixture {};
    fixture.menu_state.selected_row = whacker::app::StoryMenuRowBack;
    const whacker::app::StoryRuntimeState runtime_before = fixture.runtime;
    fixture.hub_state.selected_row = whacker::app::StoryHubRowTrainingMatch;
    fixture.hub_state.feedback_line_1 = "back line 1";
    fixture.hub_state.feedback_line_2 = "back line 2";

    fixture.run(true, fake_load_career_failure, nullptr);

    assert(g_load_career_calls == 0);
    assert(!g_begin_new_story_intro_called);
    assert(fixture.app_state == whacker::app::AppState::MainMenu);
    assert(fixture.menu_state.selected_row == whacker::app::StoryMenuRowBack);
    assert(!fixture.menu_state.confirm_overwrite);
    assert(fixture.menu_state.confirm_selected == 0);
    assert(fixture.hub_state.selected_row == whacker::app::StoryHubRowTrainingMatch);
    assert(fixture.hub_state.feedback_line_1 == "back line 1");
    assert(fixture.hub_state.feedback_line_2 == "back line 2");
    assert_runtime_unchanged(runtime_before, fixture.runtime);
}

void test_new_career_with_save_opens_overwrite_confirm_without_launch() {
    reset_stubs();
    g_stub_confirm_press = true;

    StoryMenuFixture fixture {};
    fixture.menu_state.selected_row = whacker::app::StoryMenuRowNewCareer;
    fixture.menu_state.confirm_selected = 1;
    const whacker::app::StoryRuntimeState runtime_before = fixture.runtime;
    fixture.hub_state.selected_row = whacker::app::StoryHubRowTrainingMatch;
    fixture.hub_state.feedback_line_1 = "keep line 1";
    fixture.hub_state.feedback_line_2 = "keep line 2";

    fixture.run(true, fake_load_career_failure, nullptr);

    assert(g_load_career_calls == 0);
    assert(!g_begin_new_story_intro_called);
    assert(fixture.app_state == whacker::app::AppState::StoryMenu);
    assert(fixture.menu_state.selected_row == whacker::app::StoryMenuRowNewCareer);
    assert(fixture.menu_state.confirm_overwrite);
    assert(fixture.menu_state.confirm_selected == 0);
    assert(fixture.hub_state.selected_row == whacker::app::StoryHubRowTrainingMatch);
    assert(fixture.hub_state.feedback_line_1 == "keep line 1");
    assert(fixture.hub_state.feedback_line_2 == "keep line 2");
    assert_runtime_unchanged(runtime_before, fixture.runtime);
}

void test_new_career_overwrite_confirm_cancel_clears_dialog_without_launch() {
    reset_stubs();
    g_stub_confirm_press = true;

    StoryMenuFixture fixture {};
    fixture.menu_state.selected_row = whacker::app::StoryMenuRowNewCareer;
    fixture.menu_state.confirm_overwrite = true;
    fixture.menu_state.confirm_selected = 0;
    const whacker::app::StoryRuntimeState runtime_before = fixture.runtime;
    fixture.hub_state.selected_row = whacker::app::StoryHubRowTrainingMatch;
    fixture.hub_state.feedback_line_1 = "keep line 1";
    fixture.hub_state.feedback_line_2 = "keep line 2";

    fixture.run(true, fake_load_career_failure, nullptr);

    assert(g_load_career_calls == 0);
    assert(!g_begin_new_story_intro_called);
    assert(fixture.app_state == whacker::app::AppState::StoryMenu);
    assert(fixture.menu_state.selected_row == whacker::app::StoryMenuRowNewCareer);
    assert(!fixture.menu_state.confirm_overwrite);
    assert(fixture.menu_state.confirm_selected == 0);
    assert(fixture.hub_state.selected_row == whacker::app::StoryHubRowTrainingMatch);
    assert(fixture.hub_state.feedback_line_1 == "keep line 1");
    assert(fixture.hub_state.feedback_line_2 == "keep line 2");
    assert_runtime_unchanged(runtime_before, fixture.runtime);
}

void test_new_career_overwrite_confirm_accept_launches_intro() {
    reset_stubs();
    g_stub_menu_up_press = true;
    g_stub_confirm_press = true;

    StoryMenuFixture fixture {};
    fixture.menu_state.selected_row = whacker::app::StoryMenuRowNewCareer;
    fixture.menu_state.confirm_overwrite = true;
    fixture.menu_state.confirm_selected = 0;
    const whacker::app::StoryRuntimeState runtime_before = fixture.runtime;
    fixture.hub_state.selected_row = whacker::app::StoryHubRowTrainingMatch;
    fixture.hub_state.feedback_line_1 = "keep line 1";
    fixture.hub_state.feedback_line_2 = "keep line 2";

    fixture.run(true, fake_load_career_failure, nullptr);

    assert(g_load_career_calls == 0);
    assert(g_begin_new_story_intro_called);
    assert(fixture.app_state == whacker::app::AppState::StoryMenu);
    assert(fixture.menu_state.selected_row == whacker::app::StoryMenuRowNewCareer);
    assert(!fixture.menu_state.confirm_overwrite);
    assert(fixture.menu_state.confirm_selected == 0);
    assert(fixture.hub_state.selected_row == whacker::app::StoryHubRowTrainingMatch);
    assert(fixture.hub_state.feedback_line_1 == "keep line 1");
    assert(fixture.hub_state.feedback_line_2 == "keep line 2");
    assert_runtime_unchanged(runtime_before, fixture.runtime);
}

void test_new_career_without_save_launches_intro_without_overwrite_confirm() {
    reset_stubs();
    g_stub_confirm_press = true;

    StoryMenuFixture fixture {};
    fixture.menu_state.selected_row = whacker::app::StoryMenuRowNewCareer;
    fixture.menu_state.confirm_selected = 1;
    const whacker::app::StoryRuntimeState runtime_before = fixture.runtime;
    fixture.hub_state.selected_row = whacker::app::StoryHubRowTrainingMatch;
    fixture.hub_state.feedback_line_1 = "keep line 1";
    fixture.hub_state.feedback_line_2 = "keep line 2";

    fixture.run(false, fake_load_career_failure, nullptr);

    assert(g_load_career_calls == 0);
    assert(g_begin_new_story_intro_called);
    assert(fixture.app_state == whacker::app::AppState::StoryMenu);
    assert(fixture.menu_state.selected_row == whacker::app::StoryMenuRowNewCareer);
    assert(!fixture.menu_state.confirm_overwrite);
    assert(fixture.menu_state.confirm_selected == 1);
    assert(fixture.hub_state.selected_row == whacker::app::StoryHubRowTrainingMatch);
    assert(fixture.hub_state.feedback_line_1 == "keep line 1");
    assert(fixture.hub_state.feedback_line_2 == "keep line 2");
    assert_runtime_unchanged(runtime_before, fixture.runtime);
}

void test_continue_with_load_success_routes_unjoined_to_story_scene_and_sets_feedback() {
    reset_stubs();
    g_stub_confirm_press = true;

    g_next_loaded_career.week = 12;
    g_next_loaded_career.player_name = "LOADED-A";
    g_next_loaded_career.joined_club = false;
    g_next_loaded_career.onboarding_step = whacker::app::StoryOnboardingStep::EntryBenchmarkMatch;
    g_next_loaded_career.onboarding_style_hint = whacker::app::StoryIntroStyleHint::Spin;
    g_next_loaded_career.onboarding_performance_hint = whacker::app::StoryIntroPerformanceHint::CloseLoss;
    g_next_loaded_career.onboarding_aya_feedback_available = true;
    g_next_loaded_career.onboarding_aya_feedback_from_loss = true;
    g_next_loaded_career.onboarding_aya_feedback_hint = whacker::app::StoryIntroStyleHint::Technical;
    g_next_loaded_career.onboarding_aya_forfeited = true;

    StoryMenuFixture fixture {};
    fixture.menu_state.selected_row = whacker::app::StoryMenuRowContinue;
    fixture.hub_state.selected_row = whacker::app::StoryHubRowTrainingMatch;
    fixture.hub_state.feedback_line_1 = "old success line 1";
    fixture.hub_state.feedback_line_2 = "old success line 2";

    fixture.run(true, fake_load_career_success, nullptr);

    assert(g_load_career_calls == 1);
    assert(!g_begin_new_story_intro_called);
    assert(fixture.app_state == whacker::app::AppState::StoryScene);
    assert(fixture.menu_state.selected_row == whacker::app::StoryMenuRowContinue);
    assert(fixture.hub_state.selected_row == whacker::app::StoryHubRowOfficialMatch);
    assert(fixture.hub_state.feedback_line_1 == whacker::app::story_text::career_loaded_feedback_line_1());
    assert(fixture.hub_state.feedback_line_2 == whacker::app::story_text::career_loaded_feedback_line_2(12));

    assert(fixture.runtime.career_loaded);
    assert(fixture.runtime.career.week == 12);
    assert(fixture.runtime.career.player_name == "LOADED-A");
    assert(!fixture.runtime.career.joined_club);
    assert(fixture.runtime.career.onboarding_step == whacker::app::StoryOnboardingStep::EntryBenchmarkMatch);
    assert(fixture.runtime.active_match == whacker::app::StoryMatchKind::None);
    assert(fixture.runtime.official_games_left == 0);
    assert(fixture.runtime.official_games_right == 0);
    assert(!fixture.runtime.post_forfeit_scene_pending);
    assert(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::ClubIntroScene);
    assert(fixture.runtime.onboarding_scene_pending);
    assert(fixture.runtime.onboarding_style_hint == whacker::app::StoryIntroStyleHint::Spin);
    assert(fixture.runtime.onboarding_performance_hint == whacker::app::StoryIntroPerformanceHint::CloseLoss);
    assert(fixture.runtime.onboarding_aya_feedback_available);
    assert(fixture.runtime.onboarding_aya_feedback_from_loss);
    assert(fixture.runtime.onboarding_aya_feedback_hint == whacker::app::StoryIntroStyleHint::Technical);
    assert(fixture.runtime.onboarding_aya_forfeited);
}

void test_continue_with_load_success_routes_joined_to_story_hub_and_sets_feedback() {
    reset_stubs();
    g_stub_confirm_press = true;

    g_next_loaded_career.week = 23;
    g_next_loaded_career.player_name = "LOADED-B";
    g_next_loaded_career.joined_club = true;
    g_next_loaded_career.onboarding_step = whacker::app::StoryOnboardingStep::CoachBriefScene;
    g_next_loaded_career.onboarding_style_hint = whacker::app::StoryIntroStyleHint::Power;
    g_next_loaded_career.onboarding_performance_hint = whacker::app::StoryIntroPerformanceHint::BigWin;
    g_next_loaded_career.onboarding_aya_feedback_available = true;
    g_next_loaded_career.onboarding_aya_feedback_from_loss = false;
    g_next_loaded_career.onboarding_aya_feedback_hint = whacker::app::StoryIntroStyleHint::Power;
    g_next_loaded_career.onboarding_aya_forfeited = false;

    StoryMenuFixture fixture {};
    fixture.menu_state.selected_row = whacker::app::StoryMenuRowContinue;
    fixture.hub_state.selected_row = whacker::app::StoryHubRowTrainingMatch;
    fixture.hub_state.feedback_line_1 = "old success line 1";
    fixture.hub_state.feedback_line_2 = "old success line 2";

    fixture.run(true, fake_load_career_success, nullptr);

    assert(g_load_career_calls == 1);
    assert(!g_begin_new_story_intro_called);
    assert(fixture.app_state == whacker::app::AppState::StoryHub);
    assert(fixture.menu_state.selected_row == whacker::app::StoryMenuRowContinue);
    assert(fixture.hub_state.selected_row == whacker::app::StoryHubRowOfficialMatch);
    assert(fixture.hub_state.feedback_line_1 == whacker::app::story_text::career_loaded_feedback_line_1());
    assert(fixture.hub_state.feedback_line_2 == whacker::app::story_text::career_loaded_feedback_line_2(23));

    assert(fixture.runtime.career_loaded);
    assert(fixture.runtime.career.week == 23);
    assert(fixture.runtime.career.player_name == "LOADED-B");
    assert(fixture.runtime.career.joined_club);
    assert(fixture.runtime.career.onboarding_step == whacker::app::StoryOnboardingStep::CoachBriefScene);
    assert(fixture.runtime.active_match == whacker::app::StoryMatchKind::None);
    assert(fixture.runtime.official_games_left == 0);
    assert(fixture.runtime.official_games_right == 0);
    assert(!fixture.runtime.post_forfeit_scene_pending);
    assert(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::Complete);
    assert(!fixture.runtime.onboarding_scene_pending);
    assert(fixture.runtime.onboarding_style_hint == whacker::app::StoryIntroStyleHint::Power);
    assert(fixture.runtime.onboarding_performance_hint == whacker::app::StoryIntroPerformanceHint::BigWin);
    assert(fixture.runtime.onboarding_aya_feedback_available);
    assert(!fixture.runtime.onboarding_aya_feedback_from_loss);
    assert(fixture.runtime.onboarding_aya_feedback_hint == whacker::app::StoryIntroStyleHint::Power);
    assert(!fixture.runtime.onboarding_aya_forfeited);
}

}  // namespace

namespace whacker::app {

bool consume_key_press(GLFWwindow* /*window*/, int /*key*/, bool& previous_down) {
    previous_down = false;
    return false;
}

bool consume_confirm_press(GLFWwindow* /*window*/, KeyEdgeState& /*edge_state*/) {
    const bool pressed = g_stub_confirm_press;
    g_stub_confirm_press = false;
    return pressed;
}

bool consume_menu_up_press(GLFWwindow* /*window*/, KeyEdgeState& /*edge_state*/, const ControlBindings& /*controls*/) {
    const bool pressed = g_stub_menu_up_press;
    g_stub_menu_up_press = false;
    return pressed;
}

bool consume_menu_down_press(GLFWwindow* /*window*/, KeyEdgeState& /*edge_state*/, const ControlBindings& /*controls*/) {
    const bool pressed = g_stub_menu_down_press;
    g_stub_menu_down_press = false;
    return pressed;
}

int consume_last_pressed_key() {
    return GLFW_KEY_UNKNOWN;
}

void clear_last_pressed_key() {}

void reset_story_intro_typewriter(StoryIntroState& story_intro_state) {
    story_intro_state.visible_chars = 0;
    story_intro_state.type_accum = 0.0f;
}

void reveal_story_intro_typewriter(StoryIntroState& story_intro_state) {
    story_intro_state.dialogue_writing = false;
}

bool randomize_opening_serve(whacker::sim::Simulation& /*simulation*/, std::mt19937_64& /*rng*/) {
    return true;
}

void start_match_flow(
    MatchFlowState& /*match_flow*/,
    ActiveMatchMode /*mode*/,
    bool /*opening_serve_to_right*/,
    bool /*use_deuce_serve*/) {}

void start_match_opening_countdown(MatchFlowState& /*match_flow*/, whacker::sim::Simulation& /*simulation*/) {}

void begin_new_story_intro(
    StoryRuntimeState& /*story_runtime*/,
    StoryHubState& /*story_hub_state*/,
    StoryIntroState& /*story_intro_state*/,
    MatchOptions& /*options*/,
    MatchFlowState& /*match_flow*/,
    whacker::sim::Simulation& /*simulation*/,
    AppState& /*app_state*/,
    StoryResetCareerFn /*reset_career_fn*/) {
    g_begin_new_story_intro_called = true;
}

void complete_story_intro(
    StoryRuntimeState& /*story_runtime*/,
    StoryHubState& /*story_hub_state*/,
    StoryIntroState& /*story_intro_state*/,
    MatchFlowState& /*match_flow*/,
    whacker::sim::Simulation& /*simulation*/,
    AppState& /*app_state*/,
    RuntimeAuthoredTransitionRequest& /*authored_transition_request*/,
    StorySanitizeNameFn /*sanitize_name_fn*/,
    StorySaveCareerCallback /*save_career_fn*/) {}

std::string trim_copy(const std::string& value) {
    return value;
}

}  // namespace whacker::app

int main() {
    test_continue_with_null_loader_sets_failed_feedback_and_keeps_state();
    test_continue_with_load_failure_sets_error_feedback_and_keeps_state();
    test_continue_without_save_ignores_confirm_and_keeps_state();
    test_story_menu_row_wraps_up_from_continue_to_back();
    test_story_menu_row_wraps_down_from_back_to_continue();
    test_story_menu_back_confirm_routes_to_main_menu_without_state_mutation();
    test_new_career_with_save_opens_overwrite_confirm_without_launch();
    test_new_career_overwrite_confirm_cancel_clears_dialog_without_launch();
    test_new_career_overwrite_confirm_accept_launches_intro();
    test_new_career_without_save_launches_intro_without_overwrite_confirm();
    test_continue_with_load_success_routes_unjoined_to_story_scene_and_sets_feedback();
    test_continue_with_load_success_routes_joined_to_story_hub_and_sets_feedback();
    return 0;
}
