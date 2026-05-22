#include "test_assert.hpp"
#include <algorithm>
#include <limits>
#include <random>
#include <string>

#include "match_end_flow.hpp"
#include "runtime_story_scene.hpp"
#include "story_hub_controller.hpp"
#include "story_continue_resume.hpp"
#include "story_intro.hpp"
#include "story_match.hpp"
#include "story_menu_controller.hpp"
#include "story_play_session.hpp"
#include "story_rivals.hpp"
#include "story_runtime.hpp"
#include "story_runtime_invariants.hpp"
#include "story_script_catalog.hpp"
#include "story_scene.hpp"
#include "story_intro_text_layout.hpp"
#include "text_utils.hpp"

namespace {

struct StubState {
    int save_call_count = 0;
    whacker::app::StoryCareerData saved_career {};
    int load_call_count = 0;
    bool load_succeeds = true;
    std::string load_error {};
    whacker::app::StoryCareerData loaded_career {};
    int reset_career_call_count = 0;
    bool stub_key_left = false;
    bool stub_key_right = false;
    bool stub_menu_up = false;
    bool stub_menu_down = false;
    bool stub_confirm_press = false;

    void reset_save_capture() {
        save_call_count = 0;
        saved_career = whacker::app::StoryCareerData {};
        load_call_count = 0;
        load_succeeds = true;
        load_error.clear();
        loaded_career = whacker::app::StoryCareerData {};
        reset_career_call_count = 0;
    }

    void reset_input_stubs() {
        stub_key_left = false;
        stub_key_right = false;
        stub_menu_up = false;
        stub_menu_down = false;
        stub_confirm_press = false;
    }
};

StubState g_stub_state {};
int& g_save_call_count = g_stub_state.save_call_count;
whacker::app::StoryCareerData& g_saved_career = g_stub_state.saved_career;
int& g_load_call_count = g_stub_state.load_call_count;
bool& g_load_succeeds = g_stub_state.load_succeeds;
std::string& g_load_error = g_stub_state.load_error;
whacker::app::StoryCareerData& g_loaded_career = g_stub_state.loaded_career;
int& g_reset_career_call_count = g_stub_state.reset_career_call_count;
bool& g_stub_key_left = g_stub_state.stub_key_left;
bool& g_stub_key_right = g_stub_state.stub_key_right;
bool& g_stub_menu_up = g_stub_state.stub_menu_up;
bool& g_stub_menu_down = g_stub_state.stub_menu_down;
bool& g_stub_confirm_press = g_stub_state.stub_confirm_press;

void reset_save_capture() {
    g_stub_state.reset_save_capture();
}

void reset_input_stubs() {
    g_stub_state.reset_input_stubs();
}

bool capture_save(const whacker::app::StoryCareerData& career, std::string* save_error) {
    ++g_save_call_count;
    g_saved_career = career;
    if (save_error != nullptr) {
        save_error->clear();
    }
    return true;
}

bool capture_load(whacker::app::StoryCareerData& career, std::string* load_error) {
    ++g_load_call_count;
    if (!g_load_succeeds) {
        if (load_error != nullptr) {
            *load_error = g_load_error;
        }
        return false;
    }
    career = g_loaded_career;
    if (load_error != nullptr) {
        load_error->clear();
    }
    return true;
}

void reset_career_to_custom_seed(whacker::app::StoryCareerData& career) {
    ++g_reset_career_call_count;
    career = whacker::app::StoryCareerData {};
    career.version = 17;
    career.week = 9;
    career.player_name = "SEEDED";
    career.prefers_right_side = true;
    career.joined_club = true;
    career.training_used = 3;
    career.official_completed = true;
}

std::string sanitize_to_confirmed_name(const std::string& /*raw_name*/) {
    return "CONFIRMED";
}

std::string sanitize_to_story_name(const std::string& /*raw_name*/) {
    return "STORYNAME";
}

void confirm_scene_until_app_state_changes(
    whacker::app::StorySceneState& scene,
    whacker::app::StoryRuntimeState& runtime,
    whacker::app::StoryHubState& hub,
    whacker::app::MatchOptions& options,
    whacker::app::MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    std::mt19937_64& rng,
    whacker::app::AppState& app_state,
    whacker::app::RuntimeAuthoredTransitionRequest& authored_transition_request) {
    const whacker::app::AppState starting_state = app_state;
    for (int i = 0; i < 256 && app_state == starting_state; ++i) {
        whacker::app::handle_story_scene_confirm(
            scene,
            runtime,
            hub,
            options,
            match_flow,
            simulation,
            rng,
            app_state,
            authored_transition_request,
            capture_save);
    }
    TEST_CHECK(app_state != starting_state);
}

void reset_story_integration_test_state() {
    reset_save_capture();
    reset_input_stubs();
}

bool take_input_flag(bool& flag) {
    const bool pressed = flag;
    flag = false;
    return pressed;
}

void set_action_pressed(
    whacker::app::ActionInputFrame& input,
    const whacker::app::InputAction action) {
    const auto index = static_cast<int>(action);
    input.pressed[index] = true;
    input.held[index] = true;
}

whacker::app::ActionInputFrame take_stub_action_frame() {
    whacker::app::ActionInputFrame input {};
    if (take_input_flag(g_stub_menu_up)) {
        set_action_pressed(input, whacker::app::InputAction::MenuUp);
    }
    if (take_input_flag(g_stub_menu_down)) {
        set_action_pressed(input, whacker::app::InputAction::MenuDown);
    }
    if (take_input_flag(g_stub_key_left)) {
        set_action_pressed(input, whacker::app::InputAction::MenuLeft);
    }
    if (take_input_flag(g_stub_key_right)) {
        set_action_pressed(input, whacker::app::InputAction::MenuRight);
    }
    if (take_input_flag(g_stub_confirm_press)) {
        set_action_pressed(input, whacker::app::InputAction::Confirm);
    }
    return input;
}

std::string sanitize_name_or_passthrough(
    const whacker::app::StorySanitizeNameFn sanitize_name_fn,
    const std::string& value) {
    return sanitize_name_fn != nullptr ? sanitize_name_fn(value) : value;
}

void apply_story_intro_confirm(
    whacker::app::StoryRuntimeState& runtime,
    whacker::app::StoryHubState& hub,
    whacker::app::StoryIntroState& intro,
    whacker::app::MatchOptions& options,
    whacker::app::MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    std::mt19937_64& rng,
    whacker::app::AppState& app_state,
    whacker::app::RuntimeAuthoredTransitionRequest& authored_transition_request,
    const whacker::app::StoryIntroBodyLayout& body_layout,
    const whacker::app::StorySanitizeNameFn sanitize_name_fn,
    const whacker::app::StorySaveCareerCallback save_career_fn) {
    if (intro.dialogue_writing) {
        whacker::app::reveal_story_intro_typewriter(intro);
        return;
    }
    if (intro.scroll_lines_from_bottom > 0 && body_layout.max_scroll_rows > 0) {
        intro.scroll_lines_from_bottom = 0;
        return;
    }

    if (intro.phase == whacker::app::StoryIntroPhase::Invite) {
        intro.player_is_right = false;
        intro.phase = whacker::app::StoryIntroPhase::PlayMatch;
        intro.break_kind = whacker::app::StoryIntroBreak::None;
        intro.swap_choice = 0;
        intro.phase_timer = 0.0f;
        intro.name_prompted = false;
        intro.name_accept_pending = false;
        intro.name_missing_prompt = false;
        intro.rules_hint_shown = false;
        intro.player_scored = false;
        intro.player_won = false;
        intro.player_forfeited = false;
        intro.points_played = 0;
        intro.final_left_score = 0;
        intro.final_right_score = 0;
        intro.player_usage = {};
        const whacker::app::StoryRivalSpec& intro_rival = whacker::app::story_script_intro_rival_spec();
        intro.rival_id = intro_rival.id;
        intro.rival_name = intro_rival.name;
        intro.rival_style = intro_rival.style;
        intro.rival_skills = intro_rival.skills;
        whacker::app::start_story_play_session(
            options,
            simulation,
            match_flow,
            rng,
            whacker::app::ActiveMatchMode::StoryTraining,
            false,
            intro_rival.style,
            intro_rival.skills,
            runtime.career.player_skills);
        return;
    }

    if (intro.phase == whacker::app::StoryIntroPhase::BetweenBalls) {
        if (intro.break_kind == whacker::app::StoryIntroBreak::SwapSides) {
            const bool previous_player_is_right = intro.player_is_right;
            const bool next_player_is_right = intro.swap_choice == 1;
            intro.player_is_right = next_player_is_right;
            if (next_player_is_right != previous_player_is_right) {
                auto& state = simulation.mutable_state();
                std::swap(state.left_score, state.right_score);
            }
        }
        intro.break_kind = whacker::app::StoryIntroBreak::None;
        intro.phase = whacker::app::StoryIntroPhase::PlayMatch;
        intro.phase_timer = 0.0f;
        return;
    }

    if (intro.phase == whacker::app::StoryIntroPhase::NameEntry) {
        if (whacker::app::trim_copy(intro.entered_name).empty()) {
            if (!intro.name_missing_prompt) {
                intro.name_missing_prompt = true;
                intro.name_accept_pending = false;
                whacker::app::reset_story_intro_typewriter(intro);
            }
            return;
        }
        if (!intro.name_accept_pending) {
            intro.name_accept_pending = true;
            intro.name_missing_prompt = false;
            whacker::app::reset_story_intro_typewriter(intro);
            return;
        }
        intro.entered_name = sanitize_name_or_passthrough(sanitize_name_fn, intro.entered_name);
        intro.name_accept_pending = false;
        intro.name_missing_prompt = false;
        intro.phase = whacker::app::StoryIntroPhase::PlayMatch;
        intro.phase_timer = 0.0f;
        intro.dialogue_writing = false;
        return;
    }

    if (intro.phase == whacker::app::StoryIntroPhase::RivalIntro) {
        whacker::app::complete_story_intro(
            runtime,
            hub,
            intro,
            match_flow,
            simulation,
            app_state,
            authored_transition_request,
            sanitize_name_fn,
            save_career_fn);
    }
}

struct StoryIntroInputFixture {
    whacker::app::StoryRuntimeState runtime {};
    whacker::app::StoryHubState hub {};
    whacker::app::StoryIntroState intro {};
    whacker::app::StorySceneState scene {};
    whacker::app::MatchOptions options {};
    whacker::app::ControlBindings controls {};
    whacker::app::MatchFlowState match_flow {};
    whacker::sim::Simulation simulation {};
    std::mt19937_64 rng;
    whacker::app::AppState app_state = whacker::app::AppState::StoryIntro;
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};

    explicit StoryIntroInputFixture(const std::mt19937_64::result_type seed)
        : rng(seed) {}

    void run(
        const void* key_to_name_char_fn = nullptr,
        const void* trim_copy_fn = nullptr,
        const whacker::app::StorySanitizeNameFn sanitize_name_fn = nullptr,
        const whacker::app::StorySaveCareerCallback save_career_fn = capture_save) {
        (void)key_to_name_char_fn;
        (void)trim_copy_fn;
        const bool move_left = take_input_flag(g_stub_key_left);
        const bool move_right = take_input_flag(g_stub_key_right);
        const bool move_up = take_input_flag(g_stub_menu_up);
        const bool move_down = take_input_flag(g_stub_menu_down);
        const bool confirm = take_input_flag(g_stub_confirm_press);
        const whacker::app::StoryIntroBodyLayout body_layout =
            whacker::app::compute_story_intro_body_layout_for_framebuffer(
                960,
                540,
                intro,
                controls,
                nullptr,
                sanitize_name_fn);

        intro.scroll_lines_from_bottom = whacker::app::clamp_story_intro_scroll_from_bottom(
            body_layout,
            intro.scroll_lines_from_bottom);
        if (intro.dialogue_writing || body_layout.max_scroll_rows <= 0) {
            intro.scroll_lines_from_bottom = 0;
        } else if (move_up != move_down) {
            intro.scroll_lines_from_bottom = whacker::app::clamp_story_intro_scroll_from_bottom(
                body_layout,
                intro.scroll_lines_from_bottom + (move_up ? 1 : -1));
        }

        if (intro.phase == whacker::app::StoryIntroPhase::BetweenBalls &&
            intro.break_kind == whacker::app::StoryIntroBreak::SwapSides) {
            if (move_left || move_up) {
                intro.swap_choice = 0;
            }
            if (move_right || move_down) {
                intro.swap_choice = 1;
            }
        }

        if (!confirm) {
            return;
        }
        apply_story_intro_confirm(
            runtime,
            hub,
            intro,
            options,
            match_flow,
            simulation,
            rng,
            app_state,
            authored_transition_request,
            body_layout,
            sanitize_name_fn,
            save_career_fn);
    }
};

struct StoryMenuInputFixture {
    whacker::app::StoryMenuState menu {};
    whacker::app::StoryRuntimeState runtime {};
    whacker::app::StoryHubState hub {};
    whacker::app::StoryIntroState intro {};
    whacker::app::StorySceneState scene {};
    whacker::app::MatchOptions options {};
    whacker::app::ControlBindings controls {};
    whacker::app::MatchFlowState match_flow {};
    whacker::sim::Simulation simulation {};
    whacker::app::AppState app_state = whacker::app::AppState::StoryMenu;
    std::string feedback {};

    void run(
        const bool has_save,
        const whacker::app::StoryLoadCareerCallback load_career_fn = capture_load,
        const whacker::app::StoryResetCareerFn reset_career_fn = nullptr) {
        (void)whacker::app::update_story_menu_controller(
            whacker::app::StoryMenuControllerContext {
                .menu = menu,
                .story_runtime = runtime,
                .story_hub = hub,
                .story_intro = intro,
                .story_scene = scene,
                .options = options,
                .match_flow = match_flow,
                .simulation = simulation,
                .app_state = app_state,
                .feedback = &feedback,
            },
            take_stub_action_frame(),
            has_save,
            load_career_fn,
            reset_career_fn);
    }
};

struct StorySceneLifecycleFixture {
    whacker::app::StoryRuntimeState runtime {};
    whacker::app::StorySceneState scene {};
    whacker::app::StoryHubState hub {};
    whacker::app::MatchOptions options {};
    whacker::app::MatchFlowState match_flow {};
    whacker::sim::Simulation simulation {};
    std::mt19937_64 rng;
    whacker::app::AppState app_state;
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};

    explicit StorySceneLifecycleFixture(
        const std::mt19937_64::result_type seed,
        const whacker::app::AppState initial_state = whacker::app::AppState::StoryScene)
        : rng(seed), app_state(initial_state) {}

    void begin_scene_from_runtime() {
        whacker::app::begin_story_onboarding_scene(scene, runtime);
    }

    void confirm_scene_until_app_state_changes() {
        ::confirm_scene_until_app_state_changes(
            scene,
            runtime,
            hub,
            options,
            match_flow,
            simulation,
            rng,
            app_state,
            authored_transition_request);
    }

    void start_story_match(const whacker::app::StoryMatchKind match_kind) {
        whacker::app::start_story_match(runtime, hub, options, simulation, match_flow, rng, match_kind);
    }

    void end_match(
        const whacker::app::StoryMatchEndReason end_reason,
        const int story_official_games_to_win = 3) {
        whacker::app::end_active_or_quick_match(
            runtime,
            hub,
            match_flow,
            simulation,
            scene,
            authored_transition_request,
            app_state,
            end_reason,
            story_official_games_to_win,
            capture_save);
    }
};

struct StoryHubInputFixture {
    whacker::app::StoryRuntimeState runtime {};
    whacker::app::StoryHubState hub {};
    whacker::app::StorySceneState scene {};
    whacker::app::PaddleTuningState paddle_tuning {};
    whacker::app::MatchOptions options {};
    whacker::app::ControlBindings controls {};
    whacker::app::MatchFlowState match_flow {};
    whacker::sim::Simulation simulation {};
    std::mt19937_64 rng;
    whacker::app::AppState app_state = whacker::app::AppState::StoryHub;
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};

    explicit StoryHubInputFixture(const std::mt19937_64::result_type seed)
        : rng(seed) {}

    void run() {
        (void)whacker::app::update_story_hub_controller_frame(
            whacker::app::StoryHubControllerContext {
                .story_runtime = runtime,
                .story_hub = hub,
                .story_scene = scene,
                .paddle_tuning = paddle_tuning,
                .options = options,
                .match_flow = match_flow,
                .simulation = simulation,
                .rng = rng,
                .app_state = app_state,
            },
            take_stub_action_frame(),
            capture_save);
    }

    void end_match(
        const whacker::app::StoryMatchEndReason end_reason,
        const int story_official_games_to_win = 3) {
        whacker::app::end_active_or_quick_match(
            runtime,
            hub,
            match_flow,
            simulation,
            scene,
            authored_transition_request,
            app_state,
            end_reason,
            story_official_games_to_win,
            capture_save);
    }
};

void test_early_arrival_scene_completion_routes_to_club_intro_scene() {
    reset_story_integration_test_state();

    StorySceneLifecycleFixture fixture(0xA11CEULL);
    fixture.runtime.career.player_name = "SCOTT";
    fixture.runtime.career.prefers_right_side = false;
    fixture.runtime.onboarding_step = whacker::app::StoryOnboardingStep::EarlyArrivalScene;
    fixture.runtime.onboarding_scene_pending = true;

    fixture.begin_scene_from_runtime();
    fixture.runtime.onboarding_scene_pending = false;

    TEST_CHECK(fixture.scene.id == whacker::app::StorySceneId::OnboardingEarlyArrival);
    TEST_CHECK(whacker::app::story_scene_has_content(fixture.scene));

    fixture.confirm_scene_until_app_state_changes();

    TEST_CHECK(fixture.app_state == whacker::app::AppState::Playing);
    TEST_CHECK(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::AyaFriendlyMatch);
    TEST_CHECK(fixture.runtime.active_match == whacker::app::StoryMatchKind::OnboardingAyaFriendly);
    const whacker::app::StoryRivalSpec onboarding_rival =
        whacker::app::story_script_match_spec(
            whacker::app::StoryMatchKind::OnboardingAyaFriendly,
            fixture.runtime.career.week);
    static_cast<void>(onboarding_rival);
    TEST_CHECK(fixture.runtime.active_rival_id == onboarding_rival.id);
    TEST_CHECK(fixture.runtime.active_rival_style == onboarding_rival.style);
    TEST_CHECK(fixture.runtime.active_rival_skills.edge == onboarding_rival.skills.edge);
    TEST_CHECK(fixture.runtime.active_rival_skills.power == onboarding_rival.skills.power);
    TEST_CHECK(fixture.runtime.active_rival_skills.spin_inject == onboarding_rival.skills.spin_inject);
    TEST_CHECK(fixture.match_flow.mode == whacker::app::ActiveMatchMode::StoryTraining);

    auto& terminal = fixture.simulation.mutable_state();
    terminal.left_score = 11;
    terminal.right_score = 6;
    fixture.end_match(whacker::app::StoryMatchEndReason::Completed);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryScene);
    TEST_CHECK(fixture.runtime.active_match == whacker::app::StoryMatchKind::None);
    TEST_CHECK(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::ClubIntroScene);
    TEST_CHECK(fixture.runtime.onboarding_scene_pending);
    TEST_CHECK(g_save_call_count == 1);
    TEST_CHECK(g_saved_career.onboarding_step == whacker::app::StoryOnboardingStep::ClubIntroScene);
}

void test_official_forfeit_scene_then_confirm_falls_back_to_story_hub() {
    reset_story_integration_test_state();

    StorySceneLifecycleFixture fixture(0x0FF1C1A1ULL, whacker::app::AppState::Playing);
    fixture.runtime.career.prefers_right_side = false;
    fixture.runtime.career.joined_club = true;
    fixture.runtime.onboarding_step = whacker::app::StoryOnboardingStep::Complete;
    fixture.runtime.career.onboarding_step = whacker::app::StoryOnboardingStep::Complete;
    fixture.runtime.career.week = 4;

    fixture.start_story_match(whacker::app::StoryMatchKind::Official);

    TEST_CHECK(fixture.runtime.active_match == whacker::app::StoryMatchKind::Official);
    TEST_CHECK(fixture.match_flow.mode == whacker::app::ActiveMatchMode::StoryOfficial);

    fixture.end_match(whacker::app::StoryMatchEndReason::Forfeit);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryScene);
    TEST_CHECK(fixture.runtime.post_forfeit_scene_pending);
    TEST_CHECK(fixture.runtime.active_match == whacker::app::StoryMatchKind::None);
    TEST_CHECK(fixture.runtime.career.official_forfeit_streak == 1);
    TEST_CHECK(g_save_call_count == 1);

    fixture.begin_scene_from_runtime();
    fixture.runtime.onboarding_scene_pending = false;
    fixture.runtime.post_forfeit_scene_pending = false;

    TEST_CHECK(fixture.scene.id == whacker::app::StorySceneId::PostForfeitSupport);
    TEST_CHECK(fixture.scene.line_count == 3);

    fixture.confirm_scene_until_app_state_changes();

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryHub);
    TEST_CHECK(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::Complete);
    TEST_CHECK(!whacker::app::story_scene_has_content(fixture.scene));
    TEST_CHECK(g_save_call_count == 1);
}

void test_continue_entry_resume_normalizes_and_launches_entry_match() {
    reset_story_integration_test_state();

    StorySceneLifecycleFixture fixture(0xC0FFEEULL);
    fixture.runtime.active_match = whacker::app::StoryMatchKind::Official;
    fixture.runtime.official_games_left = 2;
    fixture.runtime.official_games_right = 1;
    fixture.runtime.post_forfeit_scene_pending = true;

    whacker::app::StoryCareerData loaded {};
    loaded.player_name = "SCOTT";
    loaded.week = 3;
    loaded.prefers_right_side = true;
    loaded.joined_club = false;
    loaded.onboarding_step = whacker::app::StoryOnboardingStep::EntryBenchmarkMatch;

    fixture.app_state = whacker::app::apply_continue_loaded_career(fixture.runtime, loaded);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryScene);
    TEST_CHECK(fixture.runtime.career_loaded);
    TEST_CHECK(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::ClubIntroScene);
    TEST_CHECK(fixture.runtime.onboarding_scene_pending);
    TEST_CHECK(fixture.runtime.active_match == whacker::app::StoryMatchKind::None);
    TEST_CHECK(fixture.runtime.official_games_left == 0);
    TEST_CHECK(fixture.runtime.official_games_right == 0);
    TEST_CHECK(!fixture.runtime.post_forfeit_scene_pending);

    fixture.begin_scene_from_runtime();
    fixture.runtime.onboarding_scene_pending = false;
    TEST_CHECK(fixture.scene.id == whacker::app::StorySceneId::OnboardingClubIntro);

    fixture.confirm_scene_until_app_state_changes();

    TEST_CHECK(fixture.app_state == whacker::app::AppState::Playing);
    TEST_CHECK(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::EntryBenchmarkMatch);
    TEST_CHECK(fixture.runtime.active_match == whacker::app::StoryMatchKind::OnboardingEntry);
    const whacker::app::StoryRivalSpec entry_rival =
        whacker::app::story_script_match_spec(
            whacker::app::StoryMatchKind::OnboardingEntry,
            fixture.runtime.career.week);
    static_cast<void>(entry_rival);
    TEST_CHECK(fixture.runtime.active_rival_id == entry_rival.id);
    TEST_CHECK(fixture.runtime.active_rival_style == entry_rival.style);
    TEST_CHECK(fixture.runtime.active_rival_skills.edge == entry_rival.skills.edge);
    TEST_CHECK(fixture.runtime.active_rival_skills.power == entry_rival.skills.power);
    TEST_CHECK(fixture.runtime.active_rival_skills.spin_inject == entry_rival.skills.spin_inject);
    TEST_CHECK(fixture.match_flow.mode == whacker::app::ActiveMatchMode::StoryTraining);
    TEST_CHECK(fixture.options.left_mode == whacker::app::PaddleMode::AI);
    TEST_CHECK(fixture.options.right_mode == whacker::app::PaddleMode::Human);

    auto& terminal = fixture.simulation.mutable_state();
    terminal.left_score = 7;
    terminal.right_score = 11;
    fixture.end_match(whacker::app::StoryMatchEndReason::Completed);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryScene);
    TEST_CHECK(fixture.runtime.active_match == whacker::app::StoryMatchKind::None);
    TEST_CHECK(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::CoachBriefScene);
    TEST_CHECK(fixture.runtime.onboarding_scene_pending);
    TEST_CHECK(g_save_call_count == 1);
    TEST_CHECK(g_saved_career.onboarding_step == whacker::app::StoryOnboardingStep::CoachBriefScene);
}

void test_coach_brief_chains_into_at_home_imagination_match_then_hub() {
    reset_story_integration_test_state();

    StorySceneLifecycleFixture fixture(0xC0DEC0DEULL);
    fixture.runtime.career.player_name = "SCOTT";
    fixture.runtime.career.prefers_right_side = false;
    fixture.runtime.onboarding_step = whacker::app::StoryOnboardingStep::CoachBriefScene;
    fixture.runtime.onboarding_scene_pending = true;
    fixture.runtime.career.joined_club = false;

    fixture.begin_scene_from_runtime();
    fixture.runtime.onboarding_scene_pending = false;
    TEST_CHECK(fixture.scene.id == whacker::app::StorySceneId::OnboardingCoachBrief);

    fixture.scene.dialogue_writing = false;
    fixture.scene.line_index = fixture.scene.line_count - 1;
    whacker::app::handle_story_scene_confirm(
        fixture.scene,
        fixture.runtime,
        fixture.hub,
        fixture.options,
        fixture.match_flow,
        fixture.simulation,
        fixture.rng,
        fixture.app_state,
        fixture.authored_transition_request,
        capture_save);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryScene);
    TEST_CHECK(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::AtHomeYoutubeScene);
    TEST_CHECK(fixture.runtime.onboarding_scene_pending);
    TEST_CHECK(fixture.authored_transition_request.armed);
    TEST_CHECK(g_save_call_count == 1);
    whacker::app::clear_authored_transition_request(fixture.authored_transition_request);

    fixture.begin_scene_from_runtime();
    fixture.runtime.onboarding_scene_pending = false;
    TEST_CHECK(fixture.scene.id == whacker::app::StorySceneId::PostBenjiAtHomeYoutube);
    TEST_CHECK(fixture.scene.line_count == 2);

    fixture.scene.dialogue_writing = false;
    fixture.scene.line_index = fixture.scene.line_count - 1;
    whacker::app::handle_story_scene_confirm(
        fixture.scene,
        fixture.runtime,
        fixture.hub,
        fixture.options,
        fixture.match_flow,
        fixture.simulation,
        fixture.rng,
        fixture.app_state,
        fixture.authored_transition_request,
        capture_save);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::Playing);
    TEST_CHECK(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::Imagination1967Match);
    TEST_CHECK(fixture.runtime.active_match == whacker::app::StoryMatchKind::Imagination1967);
    TEST_CHECK(fixture.match_flow.mode == whacker::app::ActiveMatchMode::StoryTraining);
    TEST_CHECK(!fixture.authored_transition_request.armed);

    auto& terminal = fixture.simulation.mutable_state();
    terminal.left_score = 11;
    terminal.right_score = 8;
    fixture.end_match(whacker::app::StoryMatchEndReason::Completed);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryScene);
    TEST_CHECK(fixture.runtime.active_match == whacker::app::StoryMatchKind::None);
    TEST_CHECK(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::TixMidweekScene);
    TEST_CHECK(fixture.runtime.onboarding_scene_pending);
    TEST_CHECK(fixture.authored_transition_request.armed);
    TEST_CHECK(fixture.authored_transition_request.from_state == whacker::app::AppState::Playing);
    TEST_CHECK(fixture.authored_transition_request.to_state == whacker::app::AppState::StoryScene);
    TEST_CHECK(fixture.runtime.career.joined_club);
    TEST_CHECK(!fixture.runtime.career.official_completed);
    TEST_CHECK(fixture.runtime.career.tix_1967_seen);
    TEST_CHECK(fixture.runtime.career.tix_1967_player_won);
    TEST_CHECK(fixture.runtime.career.tix_1967_score_for == 11);
    TEST_CHECK(fixture.runtime.career.tix_1967_score_against == 8);
    TEST_CHECK(!fixture.runtime.career.tix_midweek_scene_seen);
    TEST_CHECK(g_save_call_count == 2);
}

void test_tix_midweek_scene_auto_routes_from_hub_and_yes_starts_lunch_match() {
    reset_story_integration_test_state();

    StoryHubInputFixture hub_fixture(0xC001D00DULL);
    hub_fixture.runtime.career_loaded = true;
    hub_fixture.runtime.career.joined_club = true;
    hub_fixture.runtime.career.tix_1967_seen = true;
    hub_fixture.runtime.onboarding_step = whacker::app::StoryOnboardingStep::TixMidweekScene;
    hub_fixture.hub.selected_row = whacker::app::StoryHubRowOfficialMatch;

    hub_fixture.run();

    TEST_CHECK(hub_fixture.app_state == whacker::app::AppState::StoryScene);
    TEST_CHECK(!hub_fixture.runtime.onboarding_scene_pending);
    TEST_CHECK(hub_fixture.scene.id == whacker::app::StorySceneId::TixMidweekLunchInvite);

    StorySceneLifecycleFixture scene_fixture(0xFACEB00CULL, whacker::app::AppState::StoryScene);
    scene_fixture.runtime = hub_fixture.runtime;
    scene_fixture.hub = hub_fixture.hub;
    scene_fixture.scene = hub_fixture.scene;
    scene_fixture.options = hub_fixture.options;

    TEST_CHECK(scene_fixture.scene.id == whacker::app::StorySceneId::TixMidweekLunchInvite);
    TEST_CHECK(scene_fixture.scene.has_binary_choice);

    scene_fixture.scene.dialogue_writing = false;
    scene_fixture.scene.line_index = scene_fixture.scene.line_count - 1;
    scene_fixture.scene.binary_choice_yes_selected = true;
    whacker::app::handle_story_scene_confirm(
        scene_fixture.scene,
        scene_fixture.runtime,
        scene_fixture.hub,
        scene_fixture.options,
        scene_fixture.match_flow,
        scene_fixture.simulation,
        scene_fixture.rng,
        scene_fixture.app_state,
        scene_fixture.authored_transition_request,
        capture_save);

    TEST_CHECK(scene_fixture.app_state == whacker::app::AppState::Playing);
    TEST_CHECK(scene_fixture.runtime.active_match == whacker::app::StoryMatchKind::TixLunch);
    TEST_CHECK(scene_fixture.runtime.career.tix_midweek_scene_seen);
    TEST_CHECK(scene_fixture.runtime.career.tix_lunch_match_accepted);
    TEST_CHECK(!scene_fixture.runtime.career.tix_lunch_match_declined);
}

void test_tix_lunch_match_completion_routes_to_post_scene_and_then_star_wipes_to_official() {
    reset_story_integration_test_state();

    StorySceneLifecycleFixture fixture(0x1234BEEF, whacker::app::AppState::Playing);
    fixture.runtime.career_loaded = true;
    fixture.runtime.career.joined_club = true;
    fixture.runtime.career.prefers_right_side = false;
    fixture.runtime.onboarding_step = whacker::app::StoryOnboardingStep::Complete;

    fixture.start_story_match(whacker::app::StoryMatchKind::TixLunch);
    TEST_CHECK(fixture.runtime.active_match == whacker::app::StoryMatchKind::TixLunch);

    auto& terminal = fixture.simulation.mutable_state();
    terminal.left_score = 11;
    terminal.right_score = 9;
    fixture.end_match(whacker::app::StoryMatchEndReason::Completed);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryScene);
    TEST_CHECK(fixture.runtime.active_match == whacker::app::StoryMatchKind::None);
    TEST_CHECK(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::PostTixLunchScene);
    TEST_CHECK(fixture.runtime.onboarding_scene_pending);
    TEST_CHECK(g_save_call_count == 1);

    fixture.begin_scene_from_runtime();
    fixture.runtime.onboarding_scene_pending = false;

    TEST_CHECK(fixture.scene.id == whacker::app::StorySceneId::TixPostLunchThanks);

    fixture.confirm_scene_until_app_state_changes();

    TEST_CHECK(fixture.app_state == whacker::app::AppState::Playing);
    TEST_CHECK(fixture.runtime.active_match == whacker::app::StoryMatchKind::Official);
    TEST_CHECK(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::Complete);
    TEST_CHECK(fixture.authored_transition_request.armed);
    TEST_CHECK(fixture.authored_transition_request.from_state == whacker::app::AppState::StoryScene);
    TEST_CHECK(fixture.authored_transition_request.to_state == whacker::app::AppState::Playing);
    TEST_CHECK(g_save_call_count == 2);
}

void test_story_hub_official_match_completion_chain_routes_to_hub_and_saves() {
    reset_story_integration_test_state();

    StoryHubInputFixture fixture(0xAABBCCDDULL);
    fixture.runtime.career_loaded = true;
    fixture.runtime.career.joined_club = true;
    fixture.runtime.career.prefers_right_side = false;
    fixture.runtime.career.official_completed = false;
    fixture.hub.selected_row = whacker::app::StoryHubRowOfficialMatch;

    g_stub_confirm_press = true;
    fixture.run();

    TEST_CHECK(fixture.app_state == whacker::app::AppState::Playing);
    TEST_CHECK(fixture.runtime.active_match == whacker::app::StoryMatchKind::Official);
    const whacker::app::StoryRivalSpec official_rival =
        whacker::app::story_script_match_spec(
            whacker::app::StoryMatchKind::Official,
            fixture.runtime.career.week);
    static_cast<void>(official_rival);
    TEST_CHECK(fixture.runtime.active_rival_id == official_rival.id);
    TEST_CHECK(fixture.runtime.active_rival_style == official_rival.style);
    TEST_CHECK(fixture.runtime.active_rival_skills.edge == official_rival.skills.edge);
    TEST_CHECK(fixture.runtime.active_rival_skills.power == official_rival.skills.power);
    TEST_CHECK(fixture.runtime.active_rival_skills.spin_inject == official_rival.skills.spin_inject);
    TEST_CHECK(fixture.match_flow.mode == whacker::app::ActiveMatchMode::StoryOfficial);

    auto& terminal = fixture.simulation.mutable_state();
    terminal.left_score = 11;
    terminal.right_score = 8;
    fixture.end_match(whacker::app::StoryMatchEndReason::Completed);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryHub);
    TEST_CHECK(fixture.runtime.active_match == whacker::app::StoryMatchKind::None);
    TEST_CHECK(fixture.runtime.career.official_completed);
    TEST_CHECK(g_save_call_count == 1);
}

void test_story_hub_training_end_chain_routes_to_hub_and_saves() {
    reset_story_integration_test_state();

    StoryHubInputFixture fixture(0xBEEFBEEFULL);
    fixture.runtime.career_loaded = true;
    fixture.runtime.career.joined_club = true;
    fixture.runtime.career.prefers_right_side = true;
    fixture.runtime.career.training_used = 0;
    fixture.runtime.career.training_matches_played = 0;
    fixture.hub.selected_row = whacker::app::StoryHubRowTrainingMatch;

    g_stub_confirm_press = true;
    fixture.run();

    TEST_CHECK(fixture.app_state == whacker::app::AppState::Playing);
    TEST_CHECK(fixture.runtime.active_match == whacker::app::StoryMatchKind::Training);
    const whacker::app::StoryRivalSpec training_rival =
        whacker::app::story_script_match_spec(
            whacker::app::StoryMatchKind::Training,
            fixture.runtime.career.week);
    static_cast<void>(training_rival);
    TEST_CHECK(fixture.runtime.active_rival_id == training_rival.id);
    TEST_CHECK(fixture.runtime.active_rival_style == training_rival.style);
    TEST_CHECK(fixture.runtime.active_rival_skills.edge == training_rival.skills.edge);
    TEST_CHECK(fixture.runtime.active_rival_skills.power == training_rival.skills.power);
    TEST_CHECK(fixture.runtime.active_rival_skills.spin_inject == training_rival.skills.spin_inject);
    TEST_CHECK(fixture.match_flow.mode == whacker::app::ActiveMatchMode::StoryTraining);

    auto& terminal = fixture.simulation.mutable_state();
    terminal.left_score = 6;
    terminal.right_score = 6;
    fixture.end_match(whacker::app::StoryMatchEndReason::EndTraining);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryHub);
    TEST_CHECK(fixture.runtime.active_match == whacker::app::StoryMatchKind::None);
    TEST_CHECK(fixture.runtime.career.training_used == 1);
    TEST_CHECK(fixture.runtime.career.training_matches_played == 1);
    TEST_CHECK(g_save_call_count == 1);
}

void test_story_hub_next_week_disabled_without_authored_next_node() {
    reset_story_integration_test_state();

    StoryHubInputFixture fixture(0x12345678ULL);
    fixture.runtime.career_loaded = true;
    fixture.runtime.career.joined_club = true;
    fixture.runtime.career.week = 1;
    fixture.runtime.career.training_used = 2;
    fixture.runtime.career.official_completed = true;
    fixture.hub.selected_row = whacker::app::StoryHubRowNextWeek;

    g_stub_confirm_press = true;
    fixture.run();

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryHub);
    TEST_CHECK(fixture.runtime.career.week == 1);
    TEST_CHECK(fixture.runtime.career.training_used == 2);
    TEST_CHECK(fixture.runtime.career.official_completed);
    TEST_CHECK(g_save_call_count == 0);
}

void test_story_hub_terminal_progress_has_no_advance_action() {
    reset_story_integration_test_state();

    StoryHubInputFixture fixture(0x12345670ULL);
    fixture.runtime.career_loaded = true;
    fixture.runtime.career.joined_club = true;
    fixture.runtime.career.week = 9;
    fixture.runtime.career.progression_node_id = "club_week_06";
    fixture.runtime.career.official_completed = true;
    fixture.hub.selected_row = whacker::app::StoryHubRowNextWeek;

    g_stub_confirm_press = true;
    fixture.run();

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryHub);
    TEST_CHECK(fixture.runtime.career.week == 9);
    TEST_CHECK(fixture.runtime.career.progression_node_id == "club_week_06");
    TEST_CHECK(fixture.runtime.career.official_completed);
    TEST_CHECK(g_save_call_count == 0);
}

void test_story_hub_back_routes_to_main_menu_and_saves() {
    reset_story_integration_test_state();

    StoryHubInputFixture fixture(0x778899AULL);
    fixture.runtime.career_loaded = true;
    fixture.runtime.career.week = 8;
    fixture.runtime.career.player_name = "SCOTT";
    fixture.hub.selected_row = whacker::app::StoryHubRowBack;

    g_stub_confirm_press = true;
    fixture.run();

    TEST_CHECK(fixture.app_state == whacker::app::AppState::MainMenu);
    TEST_CHECK(g_save_call_count == 1);
    TEST_CHECK(g_saved_career.week == 8);
    TEST_CHECK(g_saved_career.player_name == "SCOTT");
}

void test_story_hub_without_loaded_career_routes_to_story_menu() {
    reset_story_integration_test_state();

    StoryHubInputFixture fixture(0xBAD00BADULL);
    fixture.runtime.career_loaded = false;
    fixture.runtime.active_match = whacker::app::StoryMatchKind::Training;
    fixture.hub.selected_row = whacker::app::StoryHubRowOfficialMatch;

    g_stub_confirm_press = true;
    fixture.run();

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryMenu);
    TEST_CHECK(fixture.runtime.active_match == whacker::app::StoryMatchKind::Training);
    TEST_CHECK(g_save_call_count == 0);
}

void test_story_hub_disabled_rows_ignore_confirm_without_mutation() {
    reset_story_integration_test_state();

    StoryHubInputFixture fixture(0xDEADBEAFULL);
    fixture.runtime.career_loaded = true;
    fixture.runtime.career.joined_club = false;
    fixture.runtime.career.official_completed = false;
    fixture.hub.selected_row = whacker::app::StoryHubRowTrainingMatch;

    g_stub_confirm_press = true;
    fixture.run();

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryHub);
    TEST_CHECK(fixture.runtime.active_match == whacker::app::StoryMatchKind::None);
    TEST_CHECK(fixture.match_flow.mode == whacker::app::ActiveMatchMode::None);
    TEST_CHECK(g_save_call_count == 0);

    fixture.hub.selected_row = whacker::app::StoryHubRowNextWeek;
    g_stub_confirm_press = true;
    fixture.run();

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryHub);
    TEST_CHECK(fixture.runtime.career.week == 1);
    TEST_CHECK(fixture.runtime.career.training_used == 0);
    TEST_CHECK(!fixture.runtime.career.official_completed);
    TEST_CHECK(g_save_call_count == 0);
}

void test_story_hub_row_wraps_up_from_official_to_back() {
    reset_story_integration_test_state();

    StoryHubInputFixture fixture(0xABCD1234ULL);
    fixture.runtime.career_loaded = true;
    fixture.hub.selected_row = whacker::app::StoryHubRowOfficialMatch;

    g_stub_menu_up = true;
    fixture.run();

    TEST_CHECK(fixture.hub.selected_row == whacker::app::StoryHubRowBack);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryHub);
    TEST_CHECK(g_save_call_count == 0);
}

void test_story_hub_row_wraps_down_from_back_to_official() {
    reset_story_integration_test_state();

    StoryHubInputFixture fixture(0x1234ABCDULL);
    fixture.runtime.career_loaded = true;
    fixture.hub.selected_row = whacker::app::StoryHubRowBack;

    g_stub_menu_down = true;
    fixture.run();

    TEST_CHECK(fixture.hub.selected_row == whacker::app::StoryHubRowOfficialMatch);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryHub);
    TEST_CHECK(g_save_call_count == 0);
}

void test_story_hub_navigation_visits_disabled_rows_without_auto_skip() {
    reset_story_integration_test_state();

    StoryHubInputFixture fixture(0xC001D00DULL);
    fixture.runtime.career_loaded = true;
    fixture.runtime.career.joined_club = false;
    fixture.runtime.career.official_completed = false;
    fixture.hub.selected_row = whacker::app::StoryHubRowOfficialMatch;

    g_stub_menu_down = true;
    fixture.run();
    TEST_CHECK(fixture.hub.selected_row == whacker::app::StoryHubRowTrainingMatch);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryHub);

    g_stub_menu_down = true;
    fixture.run();
    TEST_CHECK(fixture.hub.selected_row == whacker::app::StoryHubRowNextWeek);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryHub);

    g_stub_menu_down = true;
    fixture.run();
    TEST_CHECK(fixture.hub.selected_row == whacker::app::StoryHubRowPaddleTuning);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryHub);

    g_stub_menu_down = true;
    fixture.run();
    TEST_CHECK(fixture.hub.selected_row == whacker::app::StoryHubRowBack);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryHub);
    TEST_CHECK(g_save_call_count == 0);
}

void test_story_intro_rival_dialogue_guard_reveals_without_transition_or_save() {
    reset_story_integration_test_state();

    StoryIntroInputFixture fixture(0xF00DFACEULL);
    fixture.runtime.career_loaded = true;
    fixture.intro.phase = whacker::app::StoryIntroPhase::RivalIntro;
    fixture.intro.dialogue_writing = true;
    fixture.intro.entered_name = "SCOTT";

    g_stub_confirm_press = true;
    fixture.run();

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryIntro);
    TEST_CHECK(!fixture.intro.dialogue_writing);
    TEST_CHECK(!fixture.runtime.onboarding_scene_pending);
    TEST_CHECK(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::None);
    TEST_CHECK(g_save_call_count == 0);
}

void test_story_intro_rival_scroll_input_moves_scroll_when_overflow_present() {
    reset_story_integration_test_state();

    StoryIntroInputFixture fixture(0x0BADF00DULL);
    fixture.intro.phase = whacker::app::StoryIntroPhase::RivalIntro;
    fixture.intro.dialogue_writing = false;
    fixture.intro.visible_chars = std::numeric_limits<std::size_t>::max();
    fixture.intro.entered_name =
        "PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER";

    const whacker::app::StoryIntroBodyLayout layout =
        whacker::app::compute_story_intro_body_layout_for_framebuffer(
            960,
            540,
            fixture.intro,
            fixture.controls,
            nullptr,
            sanitize_to_story_name);
    TEST_CHECK(layout.max_scroll_rows > 0);
    TEST_CHECK(fixture.intro.scroll_lines_from_bottom == 0);

    g_stub_menu_up = true;
    fixture.run(nullptr, nullptr, sanitize_to_story_name);
    TEST_CHECK(fixture.intro.scroll_lines_from_bottom == 1);

    g_stub_menu_down = true;
    fixture.run(nullptr, nullptr, sanitize_to_story_name);
    TEST_CHECK(fixture.intro.scroll_lines_from_bottom == 0);
}

void test_story_intro_scroll_input_ignored_without_overflow() {
    reset_story_integration_test_state();

    StoryIntroInputFixture fixture(0xAC1D0FFULL);
    fixture.intro.phase = whacker::app::StoryIntroPhase::Invite;
    fixture.intro.dialogue_writing = false;
    fixture.intro.visible_chars = std::numeric_limits<std::size_t>::max();

    const whacker::app::StoryIntroBodyLayout layout =
        whacker::app::compute_story_intro_body_layout_for_framebuffer(
            960,
            540,
            fixture.intro,
            fixture.controls,
            nullptr,
            sanitize_to_story_name);
    TEST_CHECK(layout.max_scroll_rows == 0);
    TEST_CHECK(fixture.intro.scroll_lines_from_bottom == 0);

    g_stub_menu_up = true;
    fixture.run(nullptr, nullptr, sanitize_to_story_name);
    TEST_CHECK(fixture.intro.scroll_lines_from_bottom == 0);
    TEST_CHECK(fixture.intro.phase == whacker::app::StoryIntroPhase::Invite);
}

void test_story_intro_rival_confirm_when_scrolled_snaps_before_advancing() {
    reset_story_integration_test_state();

    StoryIntroInputFixture fixture(0xCAFED00DULL);
    fixture.runtime.career_loaded = true;
    fixture.intro.phase = whacker::app::StoryIntroPhase::RivalIntro;
    fixture.intro.dialogue_writing = false;
    fixture.intro.visible_chars = std::numeric_limits<std::size_t>::max();
    fixture.intro.entered_name =
        "PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER PLAYER";

    const whacker::app::StoryIntroBodyLayout layout =
        whacker::app::compute_story_intro_body_layout_for_framebuffer(
            960,
            540,
            fixture.intro,
            fixture.controls,
            nullptr,
            sanitize_to_story_name);
    TEST_CHECK(layout.max_scroll_rows > 0);
    fixture.intro.scroll_lines_from_bottom = std::min(2, layout.max_scroll_rows);
    TEST_CHECK(fixture.intro.scroll_lines_from_bottom > 0);

    g_stub_confirm_press = true;
    fixture.run(nullptr, nullptr, sanitize_to_story_name);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryIntro);
    TEST_CHECK(fixture.intro.phase == whacker::app::StoryIntroPhase::RivalIntro);
    TEST_CHECK(fixture.intro.scroll_lines_from_bottom == 0);
    TEST_CHECK(g_save_call_count == 0);
}

void test_story_intro_invite_confirm_starts_play_match_with_reset_state() {
    reset_story_integration_test_state();

    StoryIntroInputFixture fixture(0xABCDEF01ULL);
    fixture.intro.phase = whacker::app::StoryIntroPhase::Invite;
    fixture.intro.dialogue_writing = false;
    fixture.intro.player_is_right = true;
    fixture.intro.break_kind = whacker::app::StoryIntroBreak::Rules;
    fixture.intro.swap_choice = 1;
    fixture.intro.name_prompted = true;
    fixture.intro.name_accept_pending = true;
    fixture.intro.name_missing_prompt = true;
    fixture.intro.rules_hint_shown = true;
    fixture.intro.player_scored = true;
    fixture.intro.player_won = true;
    fixture.intro.points_played = 4;
    fixture.intro.final_left_score = 8;
    fixture.intro.final_right_score = 11;
    fixture.intro.player_usage.contacts = 42;
    fixture.options.left_mode = whacker::app::PaddleMode::AI;
    fixture.options.right_mode = whacker::app::PaddleMode::Human;
    fixture.options.left_ai_style = whacker::app::AiStyle::Power;
    fixture.options.right_ai_style = whacker::app::AiStyle::Spin;
    fixture.match_flow.mode = whacker::app::ActiveMatchMode::Quick;
    auto& state = fixture.simulation.mutable_state();
    state.left_score = 5;
    state.right_score = 2;

    g_stub_confirm_press = true;
    fixture.run();

    TEST_CHECK(fixture.intro.phase == whacker::app::StoryIntroPhase::PlayMatch);
    TEST_CHECK(fixture.intro.break_kind == whacker::app::StoryIntroBreak::None);
    TEST_CHECK(fixture.intro.swap_choice == 0);
    TEST_CHECK(fixture.intro.phase_timer == 0.0f);
    TEST_CHECK(!fixture.intro.name_prompted);
    TEST_CHECK(!fixture.intro.name_accept_pending);
    TEST_CHECK(!fixture.intro.name_missing_prompt);
    TEST_CHECK(!fixture.intro.rules_hint_shown);
    TEST_CHECK(!fixture.intro.player_scored);
    TEST_CHECK(!fixture.intro.player_won);
    TEST_CHECK(fixture.intro.points_played == 0);
    TEST_CHECK(fixture.intro.final_left_score == 0);
    TEST_CHECK(fixture.intro.final_right_score == 0);
    TEST_CHECK(fixture.intro.player_usage.contacts == 0);
    TEST_CHECK(fixture.options.left_mode == whacker::app::PaddleMode::Human);
    TEST_CHECK(fixture.options.right_mode == whacker::app::PaddleMode::AI);
    TEST_CHECK(fixture.options.left_ai_style == whacker::app::AiStyle::Balanced);
    TEST_CHECK(fixture.options.right_ai_style == whacker::app::AiStyle::Balanced);
    const whacker::app::StoryRivalSpec intro_rival = whacker::app::story_script_intro_rival_spec();
    static_cast<void>(intro_rival);
    TEST_CHECK(fixture.intro.rival_name == intro_rival.name);
    TEST_CHECK(fixture.intro.rival_style == intro_rival.style);
    TEST_CHECK(fixture.intro.rival_skills.edge == intro_rival.skills.edge);
    TEST_CHECK(fixture.intro.rival_skills.power == intro_rival.skills.power);
    TEST_CHECK(fixture.intro.rival_skills.spin_inject == intro_rival.skills.spin_inject);
    TEST_CHECK(fixture.options.left_paddle_skills.edge == fixture.runtime.career.player_skills.edge);
    TEST_CHECK(fixture.options.left_paddle_skills.power == fixture.runtime.career.player_skills.power);
    TEST_CHECK(fixture.options.left_paddle_skills.spin_inject == fixture.runtime.career.player_skills.spin_inject);
    TEST_CHECK(fixture.options.right_paddle_skills.edge == intro_rival.skills.edge);
    TEST_CHECK(fixture.options.right_paddle_skills.power == intro_rival.skills.power);
    TEST_CHECK(fixture.options.right_paddle_skills.spin_inject == intro_rival.skills.spin_inject);
    TEST_CHECK(fixture.match_flow.mode == whacker::app::ActiveMatchMode::StoryTraining);
    TEST_CHECK(fixture.match_flow.opening_countdown_active);
    TEST_CHECK(state.left_score == 0);
    TEST_CHECK(state.right_score == 0);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryIntro);
    TEST_CHECK(g_save_call_count == 0);
}

void test_story_intro_invite_confirm_bootstrap_matches_onboarding_friendly_match_bootstrap() {
    reset_story_integration_test_state();

    constexpr std::mt19937_64::result_type seed = 0x13579BDFULL;

    StoryIntroInputFixture intro_fixture(seed);
    intro_fixture.runtime.career.prefers_right_side = false;
    intro_fixture.intro.phase = whacker::app::StoryIntroPhase::Invite;
    intro_fixture.intro.dialogue_writing = false;
    auto& intro_state = intro_fixture.simulation.mutable_state();
    intro_state.left_score = 6;
    intro_state.right_score = 4;
    intro_state.ball.velocity.x = 2.0f;
    intro_state.ball.velocity.y = -1.0f;

    g_stub_confirm_press = true;
    intro_fixture.run();

    StorySceneLifecycleFixture match_fixture(seed, whacker::app::AppState::Playing);
    match_fixture.runtime.career.prefers_right_side = false;
    auto& match_state = match_fixture.simulation.mutable_state();
    match_state.left_score = 6;
    match_state.right_score = 4;
    match_state.ball.velocity.x = 2.0f;
    match_state.ball.velocity.y = -1.0f;

    match_fixture.start_story_match(whacker::app::StoryMatchKind::OnboardingAyaFriendly);

    TEST_CHECK(intro_fixture.options.left_mode == match_fixture.options.left_mode);
    TEST_CHECK(intro_fixture.options.right_mode == match_fixture.options.right_mode);
    TEST_CHECK(intro_fixture.options.left_ai_style == match_fixture.options.left_ai_style);
    TEST_CHECK(intro_fixture.options.right_ai_style == match_fixture.options.right_ai_style);

    TEST_CHECK(intro_fixture.match_flow.mode == match_fixture.match_flow.mode);
    TEST_CHECK(intro_fixture.match_flow.use_deuce_serve == match_fixture.match_flow.use_deuce_serve);
    TEST_CHECK(intro_fixture.match_flow.opening_serve_to_right == match_fixture.match_flow.opening_serve_to_right);
    TEST_CHECK(intro_fixture.match_flow.serve_to_right == match_fixture.match_flow.serve_to_right);
    TEST_CHECK(intro_fixture.match_flow.serves_by_current_server == match_fixture.match_flow.serves_by_current_server);
    TEST_CHECK(intro_fixture.match_flow.opening_countdown_active == match_fixture.match_flow.opening_countdown_active);
    TEST_CHECK(intro_fixture.match_flow.opening_countdown_elapsed == match_fixture.match_flow.opening_countdown_elapsed);
    TEST_CHECK(intro_fixture.match_flow.opening_ball_visible == match_fixture.match_flow.opening_ball_visible);

    const auto& intro_after = intro_fixture.simulation.state();
    const auto& match_after = match_fixture.simulation.state();
    static_cast<void>(intro_after);
    static_cast<void>(match_after);
    TEST_CHECK(intro_after.left_score == match_after.left_score);
    TEST_CHECK(intro_after.right_score == match_after.right_score);
    TEST_CHECK(intro_after.rally_hits == match_after.rally_hits);
    TEST_CHECK(intro_after.ball.position.x == match_after.ball.position.x);
    TEST_CHECK(intro_after.ball.position.y == match_after.ball.position.y);
    TEST_CHECK(intro_after.ball.velocity.x == match_after.ball.velocity.x);
    TEST_CHECK(intro_after.ball.velocity.y == match_after.ball.velocity.y);
    TEST_CHECK(intro_after.ball.spin == match_after.ball.spin);
    TEST_CHECK(intro_after.ball.speed_scalar == match_after.ball.speed_scalar);
}

void test_story_intro_name_entry_empty_confirm_sets_missing_prompt() {
    reset_story_integration_test_state();

    StoryIntroInputFixture fixture(0xABCDEF02ULL);
    fixture.intro.phase = whacker::app::StoryIntroPhase::NameEntry;
    fixture.intro.dialogue_writing = false;
    fixture.intro.entered_name = "    ";
    fixture.intro.name_accept_pending = false;
    fixture.intro.name_missing_prompt = false;

    g_stub_confirm_press = true;
    fixture.run();

    TEST_CHECK(fixture.intro.phase == whacker::app::StoryIntroPhase::NameEntry);
    TEST_CHECK(!fixture.intro.name_accept_pending);
    TEST_CHECK(fixture.intro.name_missing_prompt);
    TEST_CHECK(fixture.intro.dialogue_writing);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryIntro);
    TEST_CHECK(g_save_call_count == 0);
}

void test_story_intro_name_entry_two_confirms_accepts_sanitized_name() {
    reset_story_integration_test_state();

    StoryIntroInputFixture fixture(0xABCDEF03ULL);
    fixture.intro.phase = whacker::app::StoryIntroPhase::NameEntry;
    fixture.intro.dialogue_writing = false;
    fixture.intro.entered_name = "alpha";
    fixture.intro.name_accept_pending = false;
    fixture.intro.name_missing_prompt = true;

    g_stub_confirm_press = true;
    fixture.run(nullptr, nullptr, sanitize_to_confirmed_name);

    TEST_CHECK(fixture.intro.phase == whacker::app::StoryIntroPhase::NameEntry);
    TEST_CHECK(fixture.intro.name_accept_pending);
    TEST_CHECK(!fixture.intro.name_missing_prompt);
    TEST_CHECK(fixture.intro.dialogue_writing);

    fixture.intro.dialogue_writing = false;
    g_stub_confirm_press = true;
    fixture.run(nullptr, nullptr, sanitize_to_confirmed_name);

    TEST_CHECK(fixture.intro.phase == whacker::app::StoryIntroPhase::PlayMatch);
    TEST_CHECK(fixture.intro.entered_name == "CONFIRMED");
    TEST_CHECK(!fixture.intro.name_accept_pending);
    TEST_CHECK(!fixture.intro.name_missing_prompt);
    TEST_CHECK(!fixture.intro.dialogue_writing);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryIntro);
    TEST_CHECK(g_save_call_count == 0);
}

void test_story_intro_rival_confirm_completes_to_story_scene_and_saves_once() {
    reset_story_integration_test_state();

    StoryIntroInputFixture fixture(0xFACEB00CULL);
    fixture.runtime.career_loaded = true;
    fixture.hub.feedback_line_1 = "stale";
    fixture.hub.feedback_line_2 = "stale";
    fixture.intro.phase = whacker::app::StoryIntroPhase::RivalIntro;
    fixture.intro.dialogue_writing = false;
    fixture.intro.entered_name = "SCOTT";
    fixture.intro.player_is_right = true;
    fixture.intro.player_won = true;
    fixture.intro.final_left_score = 4;
    fixture.intro.final_right_score = 7;

    g_stub_confirm_press = true;
    fixture.run();

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryScene);
    TEST_CHECK(fixture.runtime.career.player_name == "SCOTT");
    TEST_CHECK(fixture.runtime.career.prefers_right_side);
    TEST_CHECK(fixture.runtime.onboarding_scene_pending);
    TEST_CHECK(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::EarlyArrivalScene);
    TEST_CHECK(fixture.hub.selected_row == whacker::app::StoryHubRowOfficialMatch);
    TEST_CHECK(fixture.hub.feedback_line_1.empty());
    TEST_CHECK(fixture.hub.feedback_line_2.empty());
    TEST_CHECK(fixture.intro.phase == whacker::app::StoryIntroPhase::Invite);
    TEST_CHECK(g_save_call_count == 1);
    TEST_CHECK(g_saved_career.player_name == "SCOTT");
    TEST_CHECK(g_saved_career.onboarding_step == whacker::app::StoryOnboardingStep::EarlyArrivalScene);
}

void test_begin_new_story_intro_applies_canonical_reset_defaults() {
    reset_save_capture();
    reset_input_stubs();

    whacker::app::StoryRuntimeState runtime {};
    runtime.career_loaded = false;
    runtime.onboarding_scene_pending = true;
    runtime.onboarding_step = whacker::app::StoryOnboardingStep::CoachBriefScene;
    runtime.intro_style_hint = whacker::app::StoryIntroStyleHint::Spin;
    runtime.intro_performance_hint = whacker::app::StoryIntroPerformanceHint::BigWin;
    runtime.onboarding_style_hint = whacker::app::StoryIntroStyleHint::Power;
    runtime.onboarding_performance_hint = whacker::app::StoryIntroPerformanceHint::CloseLoss;
    runtime.onboarding_aya_feedback_available = true;
    runtime.onboarding_aya_feedback_from_loss = true;
    runtime.onboarding_aya_feedback_hint = whacker::app::StoryIntroStyleHint::Technical;
    runtime.onboarding_aya_forfeited = true;
    runtime.active_match = whacker::app::StoryMatchKind::Official;
    runtime.active_match_seconds = 42.0f;
    runtime.active_peak_lead = 7;
    runtime.active_peak_deficit = 5;
    runtime.official_games_left = 2;
    runtime.official_games_right = 1;
    runtime.post_forfeit_scene_pending = true;
    runtime.player_usage.contacts = 12;

    whacker::app::StoryHubState hub {};
    hub.selected_row = whacker::app::StoryHubRowBack;
    hub.feedback_line_1 = "stale";
    hub.feedback_line_2 = "stale";

    whacker::app::StoryIntroState intro {};
    intro.phase = whacker::app::StoryIntroPhase::RivalIntro;
    intro.break_kind = whacker::app::StoryIntroBreak::Rules;
    intro.swap_choice = 1;
    intro.player_is_right = true;
    intro.dialogue_writing = false;
    intro.entered_name = "stale";
    intro.rival_name = "RIVAL";
    intro.visible_chars = 19;

    whacker::app::MatchOptions options {};
    options.left_mode = whacker::app::PaddleMode::AI;
    options.right_mode = whacker::app::PaddleMode::Human;
    options.left_ai_style = whacker::app::AiStyle::Power;
    options.right_ai_style = whacker::app::AiStyle::Spin;

    whacker::app::MatchFlowState match_flow {};
    match_flow.mode = whacker::app::ActiveMatchMode::Quick;
    match_flow.opening_countdown_active = true;

    whacker::sim::Simulation simulation {};
    auto& state = simulation.mutable_state();
    state.left_score = 8;
    state.right_score = 6;
    state.ball.velocity.x = 3.0f;
    state.ball.velocity.y = -1.0f;

    whacker::app::AppState app_state = whacker::app::AppState::StoryMenu;

    whacker::app::begin_new_story_intro(
        runtime,
        hub,
        intro,
        options,
        match_flow,
        simulation,
        app_state,
        reset_career_to_custom_seed);

    TEST_CHECK(g_reset_career_call_count == 1);
    TEST_CHECK(runtime.career_loaded);
    TEST_CHECK(!runtime.onboarding_scene_pending);
    TEST_CHECK(runtime.onboarding_step == whacker::app::StoryOnboardingStep::None);
    TEST_CHECK(runtime.intro_style_hint == whacker::app::StoryIntroStyleHint::Balanced);
    TEST_CHECK(runtime.intro_performance_hint == whacker::app::StoryIntroPerformanceHint::Neutral);
    TEST_CHECK(runtime.onboarding_style_hint == whacker::app::StoryIntroStyleHint::Balanced);
    TEST_CHECK(runtime.onboarding_performance_hint == whacker::app::StoryIntroPerformanceHint::Neutral);
    TEST_CHECK(!runtime.onboarding_aya_feedback_available);
    TEST_CHECK(!runtime.onboarding_aya_feedback_from_loss);
    TEST_CHECK(runtime.onboarding_aya_feedback_hint == whacker::app::StoryIntroStyleHint::Balanced);
    TEST_CHECK(!runtime.onboarding_aya_forfeited);
    TEST_CHECK(!runtime.post_forfeit_scene_pending);
    TEST_CHECK(runtime.active_match == whacker::app::StoryMatchKind::None);
    TEST_CHECK(runtime.active_match_seconds == 0.0f);
    TEST_CHECK(runtime.active_peak_lead == 0);
    TEST_CHECK(runtime.active_peak_deficit == 0);
    TEST_CHECK(runtime.player_usage.contacts == 0);
    TEST_CHECK(runtime.official_games_left == 0);
    TEST_CHECK(runtime.official_games_right == 0);

    TEST_CHECK(runtime.career.version == 17);
    TEST_CHECK(runtime.career.week == 9);
    TEST_CHECK(runtime.career.player_name == "SEEDED");
    TEST_CHECK(!runtime.career.joined_club);
    TEST_CHECK(runtime.career.onboarding_step == whacker::app::StoryOnboardingStep::None);
    TEST_CHECK(runtime.career.onboarding_style_hint == whacker::app::StoryIntroStyleHint::Balanced);
    TEST_CHECK(runtime.career.onboarding_performance_hint == whacker::app::StoryIntroPerformanceHint::Neutral);
    TEST_CHECK(!runtime.career.onboarding_aya_feedback_available);
    TEST_CHECK(!runtime.career.onboarding_aya_feedback_from_loss);
    TEST_CHECK(runtime.career.onboarding_aya_feedback_hint == whacker::app::StoryIntroStyleHint::Balanced);
    TEST_CHECK(!runtime.career.onboarding_aya_forfeited);

    TEST_CHECK(hub.selected_row == whacker::app::StoryHubRowOfficialMatch);
    TEST_CHECK(hub.feedback_line_1.empty());
    TEST_CHECK(hub.feedback_line_2.empty());

    TEST_CHECK(intro.phase == whacker::app::StoryIntroPhase::Invite);
    TEST_CHECK(intro.break_kind == whacker::app::StoryIntroBreak::None);
    TEST_CHECK(intro.swap_choice == 0);
    TEST_CHECK(!intro.player_is_right);
    TEST_CHECK(intro.dialogue_writing);
    TEST_CHECK(intro.entered_name.empty());
    TEST_CHECK(intro.rival_name == "KAI");
    TEST_CHECK(intro.visible_chars == 0);
    TEST_CHECK(intro.typed_phase == whacker::app::StoryIntroPhase::Invite);
    TEST_CHECK(intro.typed_break == whacker::app::StoryIntroBreak::None);

    TEST_CHECK(options.left_mode == whacker::app::PaddleMode::Human);
    TEST_CHECK(options.right_mode == whacker::app::PaddleMode::AI);
    TEST_CHECK(options.left_ai_style == whacker::app::AiStyle::Balanced);
    TEST_CHECK(options.right_ai_style == whacker::app::AiStyle::Balanced);

    TEST_CHECK(match_flow.mode == whacker::app::ActiveMatchMode::None);
    TEST_CHECK(!match_flow.opening_countdown_active);
    TEST_CHECK(state.left_score == 0);
    TEST_CHECK(state.right_score == 0);
    TEST_CHECK(state.ball.velocity.x == 0.0f);
    TEST_CHECK(state.ball.velocity.y == 0.0f);
    TEST_CHECK(app_state == whacker::app::AppState::StoryIntro);
    TEST_CHECK(g_save_call_count == 0);
}

void test_complete_story_intro_applies_canonical_post_intro_defaults() {
    reset_save_capture();
    reset_input_stubs();

    whacker::app::StoryRuntimeState runtime {};
    runtime.career_loaded = true;
    runtime.career.joined_club = true;
    runtime.onboarding_scene_pending = false;
    runtime.onboarding_step = whacker::app::StoryOnboardingStep::EntryRetryScene;
    runtime.onboarding_aya_feedback_available = true;
    runtime.onboarding_aya_feedback_from_loss = true;
    runtime.onboarding_aya_feedback_hint = whacker::app::StoryIntroStyleHint::Spin;
    runtime.onboarding_aya_forfeited = true;
    runtime.post_forfeit_scene_pending = true;
    runtime.intro_style_hint = whacker::app::StoryIntroStyleHint::Power;
    runtime.intro_performance_hint = whacker::app::StoryIntroPerformanceHint::CloseLoss;

    whacker::app::StoryHubState hub {};
    hub.selected_row = whacker::app::StoryHubRowBack;
    hub.feedback_line_1 = "stale";
    hub.feedback_line_2 = "stale";

    whacker::app::StoryIntroState intro {};
    intro.phase = whacker::app::StoryIntroPhase::RivalIntro;
    intro.entered_name = "raw";
    intro.player_is_right = true;
    intro.player_won = true;
    intro.final_left_score = 5;
    intro.final_right_score = 11;
    intro.player_usage.contacts = 9;
    intro.rival_name = "RIVAL";

    whacker::app::MatchFlowState match_flow {};
    match_flow.mode = whacker::app::ActiveMatchMode::StoryTraining;
    match_flow.opening_countdown_active = true;

    whacker::sim::Simulation simulation {};
    auto& state = simulation.mutable_state();
    state.left_score = 4;
    state.right_score = 7;
    state.ball.velocity.x = 2.0f;
    state.ball.velocity.y = 1.0f;

    whacker::app::AppState app_state = whacker::app::AppState::StoryIntro;
    whacker::app::RuntimeAuthoredTransitionRequest authored_transition_request {};

    whacker::app::complete_story_intro(
        runtime,
        hub,
        intro,
        match_flow,
        simulation,
        app_state,
        authored_transition_request,
        sanitize_to_story_name,
        capture_save);

    TEST_CHECK(runtime.career.player_name == "STORYNAME");
    TEST_CHECK(runtime.career.prefers_right_side);
    TEST_CHECK(!runtime.career.joined_club);
    TEST_CHECK(runtime.onboarding_scene_pending);
    TEST_CHECK(runtime.onboarding_step == whacker::app::StoryOnboardingStep::EarlyArrivalScene);
    TEST_CHECK(!runtime.onboarding_aya_feedback_available);
    TEST_CHECK(!runtime.onboarding_aya_feedback_from_loss);
    TEST_CHECK(runtime.onboarding_aya_feedback_hint == whacker::app::StoryIntroStyleHint::Balanced);
    TEST_CHECK(!runtime.onboarding_aya_forfeited);
    TEST_CHECK(!runtime.post_forfeit_scene_pending);
    TEST_CHECK(runtime.onboarding_style_hint == runtime.intro_style_hint);
    TEST_CHECK(runtime.onboarding_performance_hint == runtime.intro_performance_hint);
    TEST_CHECK(authored_transition_request.armed);

    TEST_CHECK(runtime.career.onboarding_step == whacker::app::StoryOnboardingStep::EarlyArrivalScene);
    TEST_CHECK(runtime.career.onboarding_style_hint == runtime.onboarding_style_hint);
    TEST_CHECK(runtime.career.onboarding_performance_hint == runtime.onboarding_performance_hint);
    TEST_CHECK(!runtime.career.onboarding_aya_feedback_available);
    TEST_CHECK(!runtime.career.onboarding_aya_feedback_from_loss);
    TEST_CHECK(runtime.career.onboarding_aya_feedback_hint == whacker::app::StoryIntroStyleHint::Balanced);
    TEST_CHECK(!runtime.career.onboarding_aya_forfeited);

    TEST_CHECK(hub.selected_row == whacker::app::StoryHubRowOfficialMatch);
    TEST_CHECK(hub.feedback_line_1.empty());
    TEST_CHECK(hub.feedback_line_2.empty());

    TEST_CHECK(g_save_call_count == 1);
    TEST_CHECK(g_saved_career.player_name == "STORYNAME");
    TEST_CHECK(g_saved_career.onboarding_step == whacker::app::StoryOnboardingStep::EarlyArrivalScene);

    TEST_CHECK(intro.phase == whacker::app::StoryIntroPhase::Invite);
    TEST_CHECK(intro.break_kind == whacker::app::StoryIntroBreak::None);
    TEST_CHECK(intro.swap_choice == 0);
    TEST_CHECK(!intro.player_is_right);
    TEST_CHECK(!intro.dialogue_writing);
    TEST_CHECK(intro.entered_name.empty());
    TEST_CHECK(intro.rival_name == "KAI");

    TEST_CHECK(match_flow.mode == whacker::app::ActiveMatchMode::None);
    TEST_CHECK(!match_flow.opening_countdown_active);
    TEST_CHECK(state.left_score == 0);
    TEST_CHECK(state.right_score == 0);
    TEST_CHECK(state.ball.velocity.x == simulation.config().ball_base_speed);
    TEST_CHECK(state.ball.velocity.y == 0.0f);
    TEST_CHECK(app_state == whacker::app::AppState::StoryScene);
}

void test_story_intro_swap_sides_confirm_swaps_scoreboard() {
    reset_story_integration_test_state();

    StoryIntroInputFixture fixture(0xBADC0DEULL);
    fixture.runtime.career_loaded = true;
    fixture.intro.phase = whacker::app::StoryIntroPhase::BetweenBalls;
    fixture.intro.break_kind = whacker::app::StoryIntroBreak::SwapSides;
    fixture.intro.dialogue_writing = false;
    fixture.intro.player_is_right = false;
    fixture.intro.swap_choice = 1;
    auto& state = fixture.simulation.mutable_state();
    state.left_score = 3;
    state.right_score = 1;

    g_stub_confirm_press = true;
    fixture.run();

    TEST_CHECK(fixture.intro.player_is_right);
    TEST_CHECK(fixture.intro.phase == whacker::app::StoryIntroPhase::PlayMatch);
    TEST_CHECK(fixture.intro.break_kind == whacker::app::StoryIntroBreak::None);
    TEST_CHECK(state.left_score == 1);
    TEST_CHECK(state.right_score == 3);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryIntro);
    TEST_CHECK(g_save_call_count == 0);
}

void test_story_menu_new_career_without_save_starts_intro_with_runtime_reset() {
    reset_story_integration_test_state();

    StoryMenuInputFixture fixture {};
    fixture.menu.selected_row = whacker::app::StoryMenuRowNewCareer;
    fixture.runtime.career_loaded = true;
    fixture.runtime.career.player_name = "STALE";
    fixture.runtime.career.week = 4;
    fixture.runtime.career.joined_club = true;
    fixture.runtime.onboarding_scene_pending = true;
    fixture.runtime.onboarding_step = whacker::app::StoryOnboardingStep::CoachBriefScene;
    fixture.runtime.post_forfeit_scene_pending = true;
    fixture.runtime.active_match = whacker::app::StoryMatchKind::Official;
    fixture.runtime.official_games_left = 2;
    fixture.runtime.official_games_right = 1;
    fixture.hub.selected_row = whacker::app::StoryHubRowTrainingMatch;
    fixture.hub.feedback_line_1 = "stale";
    fixture.hub.feedback_line_2 = "stale";
    fixture.intro.phase = whacker::app::StoryIntroPhase::RivalIntro;
    fixture.intro.dialogue_writing = false;
    fixture.options.left_mode = whacker::app::PaddleMode::AI;
    fixture.options.right_mode = whacker::app::PaddleMode::Human;
    fixture.options.left_ai_style = whacker::app::AiStyle::Spin;
    fixture.options.right_ai_style = whacker::app::AiStyle::Maxed;
    fixture.match_flow.mode = whacker::app::ActiveMatchMode::Quick;
    auto& state = fixture.simulation.mutable_state();
    state.ball.velocity.x = 4.0f;
    state.ball.velocity.y = -2.0f;

    g_stub_confirm_press = true;
    fixture.run(false);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryIntro);
    TEST_CHECK(!fixture.menu.confirm_overwrite);
    TEST_CHECK(fixture.runtime.career_loaded);
    TEST_CHECK(fixture.runtime.career.player_name == "PLAYER");
    TEST_CHECK(fixture.runtime.career.week == 1);
    TEST_CHECK(!fixture.runtime.career.joined_club);
    TEST_CHECK(!fixture.runtime.onboarding_scene_pending);
    TEST_CHECK(fixture.runtime.onboarding_step == whacker::app::StoryOnboardingStep::None);
    TEST_CHECK(!fixture.runtime.post_forfeit_scene_pending);
    TEST_CHECK(fixture.runtime.active_match == whacker::app::StoryMatchKind::None);
    TEST_CHECK(fixture.runtime.official_games_left == 0);
    TEST_CHECK(fixture.runtime.official_games_right == 0);
    TEST_CHECK(fixture.hub.selected_row == whacker::app::StoryHubRowOfficialMatch);
    TEST_CHECK(fixture.hub.feedback_line_1.empty());
    TEST_CHECK(fixture.hub.feedback_line_2.empty());
    TEST_CHECK(fixture.intro.phase == whacker::app::StoryIntroPhase::Invite);
    TEST_CHECK(fixture.intro.dialogue_writing);
    TEST_CHECK(fixture.options.left_mode == whacker::app::PaddleMode::Human);
    TEST_CHECK(fixture.options.right_mode == whacker::app::PaddleMode::AI);
    TEST_CHECK(fixture.options.left_ai_style == whacker::app::AiStyle::Balanced);
    TEST_CHECK(fixture.options.right_ai_style == whacker::app::AiStyle::Balanced);
    TEST_CHECK(fixture.match_flow.mode == whacker::app::ActiveMatchMode::None);
    TEST_CHECK(fixture.simulation.state().ball.velocity.x == 0.0f);
    TEST_CHECK(fixture.simulation.state().ball.velocity.y == 0.0f);
    TEST_CHECK(g_save_call_count == 0);
}

void test_story_menu_overwrite_accept_starts_intro_after_modal() {
    reset_story_integration_test_state();

    StoryMenuInputFixture fixture {};
    fixture.menu.selected_row = whacker::app::StoryMenuRowNewCareer;
    fixture.runtime.career_loaded = true;
    fixture.runtime.career.player_name = "STALE";

    g_stub_confirm_press = true;
    fixture.run(true);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryMenu);
    TEST_CHECK(fixture.menu.confirm_overwrite);
    TEST_CHECK(fixture.menu.confirm_selected == 0);

    g_stub_key_right = true;
    fixture.run(true);
    TEST_CHECK(fixture.menu.confirm_overwrite);
    TEST_CHECK(fixture.menu.confirm_selected == 1);

    g_stub_confirm_press = true;
    fixture.run(true);

    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryIntro);
    TEST_CHECK(!fixture.menu.confirm_overwrite);
    TEST_CHECK(fixture.menu.confirm_selected == 0);
    TEST_CHECK(fixture.runtime.career.player_name == "PLAYER");
    TEST_CHECK(fixture.intro.phase == whacker::app::StoryIntroPhase::Invite);
    TEST_CHECK(g_save_call_count == 0);
}

void test_story_menu_continue_failure_surfaces_feedback() {
    reset_story_integration_test_state();

    StoryMenuInputFixture fixture {};
    fixture.menu.selected_row = whacker::app::StoryMenuRowContinue;
    g_load_succeeds = false;
    g_load_error = "SAVE DATA BROKEN";

    g_stub_confirm_press = true;
    fixture.run(true, capture_load);

    TEST_CHECK(g_load_call_count == 1);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryMenu);
    TEST_CHECK(fixture.feedback == "SAVE DATA BROKEN");
    TEST_CHECK(!fixture.runtime.career_loaded);
}

void test_story_menu_continue_success_routes_loaded_club_career_to_hub() {
    reset_story_integration_test_state();

    StoryMenuInputFixture fixture {};
    fixture.menu.selected_row = whacker::app::StoryMenuRowContinue;
    g_loaded_career = whacker::app::StoryCareerData {};
    g_loaded_career.joined_club = true;
    g_loaded_career.week = 3;
    g_loaded_career.player_name = "LOADED";

    g_stub_confirm_press = true;
    fixture.run(true, capture_load);

    TEST_CHECK(g_load_call_count == 1);
    TEST_CHECK(fixture.app_state == whacker::app::AppState::StoryHub);
    TEST_CHECK(fixture.runtime.career_loaded);
    TEST_CHECK(fixture.runtime.career.player_name == "LOADED");
    TEST_CHECK(fixture.hub.selected_row == whacker::app::StoryHubRowOfficialMatch);
    TEST_CHECK(!fixture.hub.feedback_line_1.empty());
    TEST_CHECK(!fixture.hub.feedback_line_2.empty());
    TEST_CHECK(fixture.feedback.empty());
}

}  // namespace

int main() {
    test_early_arrival_scene_completion_routes_to_club_intro_scene();
    test_official_forfeit_scene_then_confirm_falls_back_to_story_hub();
    test_continue_entry_resume_normalizes_and_launches_entry_match();
    test_coach_brief_chains_into_at_home_imagination_match_then_hub();
    test_tix_midweek_scene_auto_routes_from_hub_and_yes_starts_lunch_match();
    test_tix_lunch_match_completion_routes_to_post_scene_and_then_star_wipes_to_official();
    test_story_hub_official_match_completion_chain_routes_to_hub_and_saves();
    test_story_hub_training_end_chain_routes_to_hub_and_saves();
    test_story_hub_next_week_disabled_without_authored_next_node();
    test_story_hub_terminal_progress_has_no_advance_action();
    test_story_hub_back_routes_to_main_menu_and_saves();
    test_story_hub_without_loaded_career_routes_to_story_menu();
    test_story_hub_disabled_rows_ignore_confirm_without_mutation();
    test_story_hub_row_wraps_up_from_official_to_back();
    test_story_hub_row_wraps_down_from_back_to_official();
    test_story_hub_navigation_visits_disabled_rows_without_auto_skip();
    test_story_intro_rival_dialogue_guard_reveals_without_transition_or_save();
    test_story_intro_rival_scroll_input_moves_scroll_when_overflow_present();
    test_story_intro_scroll_input_ignored_without_overflow();
    test_story_intro_rival_confirm_when_scrolled_snaps_before_advancing();
    test_story_intro_invite_confirm_starts_play_match_with_reset_state();
    test_story_intro_invite_confirm_bootstrap_matches_onboarding_friendly_match_bootstrap();
    test_story_intro_name_entry_empty_confirm_sets_missing_prompt();
    test_story_intro_name_entry_two_confirms_accepts_sanitized_name();
    test_story_intro_rival_confirm_completes_to_story_scene_and_saves_once();
    test_begin_new_story_intro_applies_canonical_reset_defaults();
    test_complete_story_intro_applies_canonical_post_intro_defaults();
    test_story_intro_swap_sides_confirm_swaps_scoreboard();
    test_story_menu_new_career_without_save_starts_intro_with_runtime_reset();
    test_story_menu_overwrite_accept_starts_intro_after_modal();
    test_story_menu_continue_failure_surfaces_feedback();
    test_story_menu_continue_success_routes_loaded_club_career_to_hub();
    return 0;
}
