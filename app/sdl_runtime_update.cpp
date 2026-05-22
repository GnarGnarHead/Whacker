#include "sdl_runtime_update.hpp"

#include <algorithm>
#include <cstddef>
#include <string>

#include "main_menu_actions.hpp"
#include "match_exit_policy.hpp"
#include "match_flow.hpp"
#include "paddle_tuning_actions.hpp"
#include "pause_menu_actions.hpp"
#include "play_control.hpp"
#include "quick_menu_actions.hpp"
#include "runtime_story_scene.hpp"
#include "runtime_transitions.hpp"
#include "sdl_runtime_audio.hpp"
#include "sdl_options_controller.hpp"
#include "sdl_runtime_labels.hpp"
#include "sdl_runtime_transitions.hpp"
#include "sdl_story_hub_rules.hpp"
#include "sim/math.hpp"
#include "story_continue_resume.hpp"
#include "story_intro_text_layout.hpp"
#include "story_match.hpp"
#include "story_menu_actions.hpp"
#include "story_play_session.hpp"
#include "story_runtime_invariants.hpp"
#include "story_save.hpp"
#include "story_scene_text_layout.hpp"
#include "story_script_catalog.hpp"
#include "story_skill_limits.hpp"
#include "story_text.hpp"
#include "text_utils.hpp"

namespace whacker::app {

namespace {

void append_story_name_text(StoryIntroState& story_intro, const std::string& text_input) {
    for (const char raw_ch : text_input) {
        if (story_intro.entered_name.size() >= 16u) {
            return;
        }
        char ch = raw_ch;
        if (ch >= 'a' && ch <= 'z') {
            ch = static_cast<char>(ch - 'a' + 'A');
        }
        const bool allowed =
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9') ||
            ch == ' ' ||
            ch == '-' ||
            ch == '_';
        if (!allowed) {
            continue;
        }
        if (ch == ' ' && (story_intro.entered_name.empty() || story_intro.entered_name.back() == ' ')) {
            continue;
        }
        story_intro.entered_name.push_back(ch);
    }
}

void update_main_menu(
    const ActionInputFrame& input,
    SdlRuntimeState& runtime,
    SdlPlatform& platform) {
    const int previous_row = runtime.main_menu.selected_row;
    const MainMenuActionResult result = apply_main_menu_action_frame(runtime.main_menu, input);
    if (runtime.main_menu.selected_row != previous_row) {
        runtime.main_menu_feedback.clear();
        play_menu_move_sound(runtime);
    }
    if (result != MainMenuActionResult::None) {
        play_menu_confirm_sound(runtime);
    }
    apply_main_menu_result(result, runtime, platform);
    runtime.accumulator = 0.0;
}

void update_options_menu(
    const ActionInputFrame& input,
    const SdlEventFrame& events,
    SdlRuntimeState& runtime) {
    const SdlOptionsUpdateEffects effects = update_sdl_options_menu(runtime, input, events);
    if (effects.audio_changed) {
        apply_sdl_runtime_audio_settings(runtime);
    }
    if (effects.persist_requested) {
        persist_runtime_menu_settings(runtime);
    }
    if (effects.play_move_sound) {
        play_menu_move_sound(runtime);
    }
    if (effects.play_confirm_sound) {
        play_menu_confirm_sound(runtime);
    }
    if (effects.back_requested) {
        return_to_main_menu(runtime);
    }
    runtime.accumulator = 0.0;
}

void update_story_menu(
    const ActionInputFrame& input,
    whacker::sim::Simulation& simulation,
    SdlRuntimeState& runtime) {
    const int previous_row = runtime.story_menu.selected_row;
    const bool confirm_before = runtime.story_menu.confirm_overwrite;
    const int confirm_selected_before = runtime.story_menu.confirm_selected;
    const bool has_save = story_save_exists();
    const StoryMenuActionResult result =
        apply_story_menu_action_frame(runtime.story_menu, has_save, input);
    if (runtime.story_menu.selected_row != previous_row) {
        runtime.story_menu_feedback.clear();
        play_menu_move_sound(runtime);
    }
    if (runtime.story_menu.confirm_selected != confirm_selected_before) {
        play_menu_move_sound(runtime);
    }
    if (result != StoryMenuActionResult::None || runtime.story_menu.confirm_overwrite != confirm_before) {
        play_menu_confirm_sound(runtime);
    }
    if (result == StoryMenuActionResult::Back) {
        return_to_main_menu(runtime);
    } else if (result == StoryMenuActionResult::Continue) {
        StoryCareerData loaded {};
        std::string load_error;
        if (!load_story_career(loaded, &load_error)) {
            runtime.story_menu_feedback = load_error.empty() ? "COULD NOT LOAD STORY SAVE" : load_error;
        } else {
            StoryRuntimeState next_story_runtime = runtime.story_runtime;
            const AppState loaded_state = apply_continue_loaded_career(next_story_runtime, loaded);
            if (loaded_state == AppState::StoryHub) {
                runtime.story_runtime = next_story_runtime;
                runtime.story_hub.selected_row = StoryHubRowOfficialMatch;
                runtime.story_hub.feedback_line_1 = story_text::career_loaded_feedback_line_1();
                runtime.story_hub.feedback_line_2 = story_text::career_loaded_feedback_line_2(runtime.story_runtime.career.week);
                runtime.app_state = AppState::StoryHub;
            } else {
                runtime.story_runtime = next_story_runtime;
                begin_story_onboarding_scene(runtime.story_scene, runtime.story_runtime);
                clear_story_runtime_scene_pending_flags(runtime.story_runtime);
                runtime.app_state = AppState::StoryScene;
            }
        }
    } else if (result == StoryMenuActionResult::NewCareer) {
        runtime.story_menu.confirm_overwrite = false;
        runtime.story_menu.confirm_selected = 0;
        begin_new_story_intro(
            runtime.story_runtime,
            runtime.story_hub,
            runtime.story_intro,
            runtime.options,
            runtime.match_flow,
            simulation,
            runtime.app_state,
            reset_story_career);
    }
    runtime.accumulator = 0.0;
}

void update_story_hub(
    const ActionInputFrame& input,
    SdlRuntimeState& runtime,
    whacker::sim::Simulation& simulation) {
    if (!runtime.story_runtime.career_loaded) {
        enter_story_menu(runtime);
        return;
    }
    const bool tix_midweek_pending =
        runtime.story_runtime.career.joined_club &&
        runtime.story_runtime.career.tix_1967_seen &&
        !runtime.story_runtime.career.tix_midweek_scene_seen &&
        !runtime.story_runtime.career.tix_lunch_match_declined &&
        !runtime.story_runtime.career.tix_lunch_match_completed;
    if (tix_midweek_pending) {
        queue_story_onboarding_scene(runtime.story_runtime, StoryOnboardingStep::TixMidweekScene);
        copy_onboarding_runtime_to_career(runtime.story_runtime);
        (void)save_story_career(runtime.story_runtime.career, nullptr);
        begin_story_onboarding_scene(runtime.story_scene, runtime.story_runtime);
        clear_story_runtime_scene_pending_flags(runtime.story_runtime);
        runtime.app_state = AppState::StoryScene;
        return;
    }
    const int previous_row = runtime.story_hub.selected_row;
    if (input_pressed(input, InputAction::MenuUp)) {
        runtime.story_hub.selected_row =
            (runtime.story_hub.selected_row + StoryHubRowCount - 1) % StoryHubRowCount;
    }
    if (input_pressed(input, InputAction::MenuDown)) {
        runtime.story_hub.selected_row =
            (runtime.story_hub.selected_row + 1) % StoryHubRowCount;
    }
    if (runtime.story_hub.selected_row != previous_row) {
        play_menu_move_sound(runtime);
    }
    if (input_pressed(input, InputAction::Back)) {
        play_menu_confirm_sound(runtime);
        (void)save_story_career(runtime.story_runtime.career, nullptr);
        return_to_main_menu(runtime);
        return;
    }
    if (!input_pressed(input, InputAction::Confirm)) {
        return;
    }
    play_menu_confirm_sound(runtime);

    const StoryHubRow row = static_cast<StoryHubRow>(runtime.story_hub.selected_row);
    if (!sdl_story_hub_row_enabled(row, runtime.story_runtime.career)) {
        runtime.story_hub.feedback_line_1 = "LOCKED FOR THIS WEEK";
        runtime.story_hub.feedback_line_2.clear();
        return;
    }
    if (row == StoryHubRowBack) {
        (void)save_story_career(runtime.story_runtime.career, nullptr);
        return_to_main_menu(runtime);
        return;
    }

    if (row == StoryHubRowNextWeek) {
        advance_story_week(runtime.story_runtime, runtime.story_hub, save_story_career);
        return;
    }
    if (row == StoryHubRowPaddleTuning) {
        normalize_story_player_skill_progress(
            runtime.story_runtime.career.player_skills,
            runtime.story_runtime.career.player_skill_caps);
        begin_story_player_paddle_tuning(runtime.paddle_tuning, runtime.story_runtime.career);
        runtime.app_state = AppState::PaddleTuning;
        return;
    }
    if (row == StoryHubRowOfficialMatch || row == StoryHubRowTrainingMatch) {
        const StoryMatchKind kind =
            row == StoryHubRowOfficialMatch ? StoryMatchKind::Official : StoryMatchKind::Training;
        start_story_match(
            runtime.story_runtime,
            runtime.story_hub,
            runtime.options,
            simulation,
            runtime.match_flow,
            runtime.rng,
            kind);
        runtime.app_state = AppState::Playing;
    }
}

void update_quick_match_setup(
    const ActionInputFrame& input,
    SdlRuntimeState& runtime,
    whacker::sim::Simulation& simulation) {
    if (input_pressed(input, InputAction::Back)) {
        play_menu_confirm_sound(runtime);
        return_to_main_menu(runtime);
        return;
    }

    const int row_before = runtime.quick_menu.selected_row;
    const MatchOptions options_before = runtime.options;
    const QuickMenuActionResult result =
        apply_quick_menu_action_frame(runtime.quick_menu, runtime.options, input);
    const bool options_changed = !options_equal(options_before, runtime.options);
    if (runtime.quick_menu.selected_row != row_before || options_changed) {
        play_menu_move_sound(runtime);
    }
    if (result == QuickMenuActionResult::StartMatch) {
        play_menu_confirm_sound(runtime);
        start_quick_match(runtime, simulation);
    } else if (result == QuickMenuActionResult::TuneP1) {
        play_menu_confirm_sound(runtime);
        begin_quick_paddle_tuning(
            runtime.paddle_tuning,
            AppState::QuickMatchSetup,
            PaddleTuningTarget::QuickLeft,
            runtime.options.left_paddle_skills);
        runtime.app_state = AppState::PaddleTuning;
    } else if (result == QuickMenuActionResult::TuneP2) {
        play_menu_confirm_sound(runtime);
        begin_quick_paddle_tuning(
            runtime.paddle_tuning,
            AppState::QuickMatchSetup,
            PaddleTuningTarget::QuickRight,
            runtime.options.right_paddle_skills);
        runtime.app_state = AppState::PaddleTuning;
    }
    if (options_changed) {
        persist_runtime_menu_settings(runtime);
    }
    runtime.accumulator = 0.0;
}

void update_paddle_tuning(
    const ActionInputFrame& input,
    SdlRuntimeState& runtime) {
    const PaddleTuningActionResult result =
        apply_paddle_tuning_action_frame(runtime.paddle_tuning, input);
    if (result == PaddleTuningActionResult::Changed) {
        play_menu_move_sound(runtime);
    } else if (result == PaddleTuningActionResult::Commit) {
        play_menu_confirm_sound(runtime);
        if (runtime.paddle_tuning.target == PaddleTuningTarget::StoryPlayer) {
            commit_paddle_tuning_to_career(runtime.paddle_tuning, runtime.story_runtime.career);
            copy_onboarding_runtime_to_career(runtime.story_runtime);
            (void)save_story_career(runtime.story_runtime.career, nullptr);
        } else {
            commit_paddle_tuning_to_options(runtime.paddle_tuning, runtime.options);
            persist_runtime_menu_settings(runtime);
        }
        runtime.paddle_tuning.active = false;
        runtime.app_state = runtime.paddle_tuning.return_state;
    } else if (result == PaddleTuningActionResult::Cancel) {
        play_menu_confirm_sound(runtime);
        runtime.paddle_tuning.active = false;
        runtime.app_state = runtime.paddle_tuning.return_state;
    }
    runtime.accumulator = 0.0;
}

void track_story_intro_contact_usage(
    StoryIntroState& story_intro,
    const whacker::sim::SimulationConfig& config,
    const whacker::sim::RallyState& before,
    const whacker::sim::RallyState& after) {
    if (after.rally_hits <= before.rally_hits) {
        return;
    }
    const bool hitter_left = after.ball.velocity.x > 0.0f;
    const bool hitter_is_player = story_intro.player_is_right ? !hitter_left : hitter_left;
    if (!hitter_is_player) {
        return;
    }
    const float denom = std::max(config.paddle_half_height, 1.0e-3f);
    const auto& player_paddle = story_intro.player_is_right ? after.right : after.left;
    const float contact_u = whacker::sim::clampf((after.ball.position.y - player_paddle.center_y) / denom, -1.0f, 1.0f);
    const float ball_speed = whacker::sim::speed_of(after.ball);
    whacker::progression::accumulate_contact_usage(
        story_intro.player_usage,
        contact_u,
        player_paddle.velocity_y,
        ball_speed,
        config);
}

void update_story_intro_dialogue_input(
    const ActionInputFrame& input,
    const SdlEventFrame& events,
    const RenderContext& render_context,
    SdlRuntimeState& runtime,
    whacker::sim::Simulation& simulation) {
    const StoryIntroBodyLayout body_layout = compute_story_intro_body_layout_for_framebuffer(
        render_context.framebuffer_width,
        render_context.framebuffer_height,
        runtime.story_intro,
        runtime.controls,
        sdl_key_name,
        sanitize_player_name);

    if (!runtime.story_intro.dialogue_writing && body_layout.max_scroll_rows > 0) {
        int scroll_delta = 0;
        if (input_pressed(input, InputAction::MenuUp)) {
            scroll_delta += 1;
        }
        if (input_pressed(input, InputAction::MenuDown)) {
            scroll_delta -= 1;
        }
        if (scroll_delta != 0) {
            const int next_scroll = clamp_story_intro_scroll_from_bottom(
                body_layout,
                runtime.story_intro.scroll_lines_from_bottom + scroll_delta);
            if (next_scroll != runtime.story_intro.scroll_lines_from_bottom) {
                play_menu_move_sound(runtime);
            }
            runtime.story_intro.scroll_lines_from_bottom = next_scroll;
        }
    } else {
        runtime.story_intro.scroll_lines_from_bottom = 0;
    }

    if (runtime.story_intro.phase == StoryIntroPhase::BetweenBalls &&
        runtime.story_intro.break_kind == StoryIntroBreak::SwapSides) {
        if (input_pressed(input, InputAction::MenuLeft) || input_pressed(input, InputAction::MenuUp)) {
            if (runtime.story_intro.swap_choice != 0) {
                play_menu_move_sound(runtime);
            }
            runtime.story_intro.swap_choice = 0;
        }
        if (input_pressed(input, InputAction::MenuRight) || input_pressed(input, InputAction::MenuDown)) {
            if (runtime.story_intro.swap_choice != 1) {
                play_menu_move_sound(runtime);
            }
            runtime.story_intro.swap_choice = 1;
        }
    }

    if (runtime.story_intro.phase == StoryIntroPhase::NameEntry && !runtime.story_intro.dialogue_writing) {
        const bool had_text_edit =
            !events.text_input.empty() || events.backspace_pressed;
        if (events.backspace_pressed && !runtime.story_intro.entered_name.empty()) {
            runtime.story_intro.entered_name.pop_back();
        }
        append_story_name_text(runtime.story_intro, events.text_input);
        if (had_text_edit && (runtime.story_intro.name_accept_pending || runtime.story_intro.name_missing_prompt)) {
            runtime.story_intro.name_accept_pending = false;
            runtime.story_intro.name_missing_prompt = false;
            reset_story_intro_typewriter(runtime.story_intro);
        }
    }

    if (!input_pressed(input, InputAction::Confirm)) {
        return;
    }
    play_menu_confirm_sound(runtime);
    if (runtime.story_intro.dialogue_writing) {
        reveal_story_intro_typewriter(runtime.story_intro);
        return;
    }
    if (runtime.story_intro.scroll_lines_from_bottom > 0 && body_layout.max_scroll_rows > 0) {
        runtime.story_intro.scroll_lines_from_bottom = 0;
        return;
    }

    if (runtime.story_intro.phase == StoryIntroPhase::Invite) {
        runtime.story_intro.player_is_right = false;
        runtime.story_intro.phase = StoryIntroPhase::PlayMatch;
        runtime.story_intro.break_kind = StoryIntroBreak::None;
        runtime.story_intro.swap_choice = 0;
        runtime.story_intro.phase_timer = 0.0f;
        runtime.story_intro.name_prompted = false;
        runtime.story_intro.name_accept_pending = false;
        runtime.story_intro.name_missing_prompt = false;
        runtime.story_intro.rules_hint_shown = false;
        runtime.story_intro.player_scored = false;
        runtime.story_intro.player_won = false;
        runtime.story_intro.player_forfeited = false;
        runtime.story_intro.points_played = 0;
        runtime.story_intro.final_left_score = 0;
        runtime.story_intro.final_right_score = 0;
        runtime.story_intro.player_usage = {};
        const StoryRivalSpec& intro_rival = story_script_intro_rival_spec();
        runtime.story_intro.rival_id = intro_rival.id;
        runtime.story_intro.rival_name = intro_rival.name;
        runtime.story_intro.rival_style = intro_rival.style;
        runtime.story_intro.rival_skills = intro_rival.skills;
        start_story_play_session(
            runtime.options,
            simulation,
            runtime.match_flow,
            runtime.rng,
            ActiveMatchMode::StoryTraining,
            false,
            intro_rival.style,
            intro_rival.skills,
            runtime.story_runtime.career.player_skills);
        return;
    }

    if (runtime.story_intro.phase == StoryIntroPhase::BetweenBalls) {
        if (runtime.story_intro.break_kind == StoryIntroBreak::SwapSides) {
            const bool previous_player_is_right = runtime.story_intro.player_is_right;
            const bool next_player_is_right = runtime.story_intro.swap_choice == 1;
            runtime.story_intro.player_is_right = next_player_is_right;
            if (next_player_is_right != previous_player_is_right) {
                auto& state = simulation.mutable_state();
                std::swap(state.left_score, state.right_score);
            }
        }
        runtime.story_intro.break_kind = StoryIntroBreak::None;
        runtime.story_intro.phase = StoryIntroPhase::PlayMatch;
        runtime.story_intro.phase_timer = 0.0f;
        return;
    }

    if (runtime.story_intro.phase == StoryIntroPhase::NameEntry) {
        if (trim_copy(runtime.story_intro.entered_name).empty()) {
            if (!runtime.story_intro.name_missing_prompt) {
                runtime.story_intro.name_missing_prompt = true;
                runtime.story_intro.name_accept_pending = false;
                reset_story_intro_typewriter(runtime.story_intro);
            }
            return;
        }
        if (!runtime.story_intro.name_accept_pending) {
            runtime.story_intro.name_accept_pending = true;
            runtime.story_intro.name_missing_prompt = false;
            reset_story_intro_typewriter(runtime.story_intro);
            return;
        }
        runtime.story_intro.entered_name = sanitize_player_name(runtime.story_intro.entered_name);
        runtime.story_intro.name_accept_pending = false;
        runtime.story_intro.name_missing_prompt = false;
        runtime.story_intro.phase = StoryIntroPhase::PlayMatch;
        runtime.story_intro.phase_timer = 0.0f;
        runtime.story_intro.dialogue_writing = false;
        return;
    }

    if (runtime.story_intro.phase == StoryIntroPhase::RivalIntro) {
        complete_story_intro(
            runtime.story_runtime,
            runtime.story_hub,
            runtime.story_intro,
            runtime.match_flow,
            simulation,
            runtime.app_state,
            runtime.authored_transition_request,
            sanitize_player_name,
            save_story_career);
    }
}

void update_story_scene_input(
    const ActionInputFrame& input,
    const RenderContext& render_context,
    SdlRuntimeState& runtime,
    whacker::sim::Simulation& simulation) {
    if (runtime.story_runtime.onboarding_scene_pending || runtime.story_runtime.post_forfeit_scene_pending) {
        begin_story_onboarding_scene(runtime.story_scene, runtime.story_runtime);
        clear_story_runtime_scene_pending_flags(runtime.story_runtime);
    }

    const StorySceneBodyLayout body_layout = compute_story_scene_body_layout_for_framebuffer(
        render_context.framebuffer_width,
        render_context.framebuffer_height,
        runtime.story_scene);
    if (!runtime.story_scene.dialogue_writing && body_layout.max_scroll_lines > 0) {
        int scroll_delta = 0;
        if (input_pressed(input, InputAction::MenuUp)) {
            scroll_delta += 1;
        }
        if (input_pressed(input, InputAction::MenuDown)) {
            scroll_delta -= 1;
        }
        if (scroll_delta != 0) {
            const int next_scroll = clamp_story_scene_scroll_from_bottom(
                body_layout,
                runtime.story_scene.scroll_lines_from_bottom + scroll_delta);
            if (next_scroll != runtime.story_scene.scroll_lines_from_bottom) {
                play_menu_move_sound(runtime);
            }
            runtime.story_scene.scroll_lines_from_bottom = next_scroll;
        }
    } else {
        runtime.story_scene.scroll_lines_from_bottom = 0;
    }
    if (!runtime.story_scene.dialogue_writing && runtime.story_scene.has_binary_choice) {
        if (input_pressed(input, InputAction::MenuLeft) || input_pressed(input, InputAction::MenuRight)) {
            runtime.story_scene.binary_choice_yes_selected = !runtime.story_scene.binary_choice_yes_selected;
            play_menu_move_sound(runtime);
        }
    }
    if (input_pressed(input, InputAction::Back)) {
        play_menu_confirm_sound(runtime);
        clear_story_scene(runtime.story_scene);
        clear_story_runtime_scene_pending_flags(runtime.story_runtime);
        runtime.app_state = AppState::StoryMenu;
        runtime.accumulator = 0.0;
        return;
    }
    if (!input_pressed(input, InputAction::Confirm)) {
        return;
    }
    play_menu_confirm_sound(runtime);
    if (runtime.story_scene.dialogue_writing) {
        reveal_story_scene_current_line(runtime.story_scene);
        return;
    }
    if (runtime.story_scene.scroll_lines_from_bottom > 0 && body_layout.max_scroll_lines > 0) {
        runtime.story_scene.scroll_lines_from_bottom = 0;
        return;
    }
    handle_story_scene_confirm(
        runtime.story_scene,
        runtime.story_runtime,
        runtime.story_hub,
        runtime.options,
        runtime.match_flow,
        simulation,
        runtime.rng,
        runtime.app_state,
        runtime.authored_transition_request,
        save_story_career);
}

void update_dialogue_step(
    SdlRuntimeState& runtime,
    const double frame_dt,
    whacker::sim::Simulation& simulation) {
    const float dt = static_cast<float>(frame_dt);
    tick_story_typewriter_audio(runtime, dt);
    if (runtime.app_state == AppState::StoryIntro) {
        runtime.story_intro.phase_timer += dt;
        const std::size_t visible_before = runtime.story_intro.visible_chars;
        update_story_intro_typewriter(
            runtime.story_intro,
            runtime.controls,
            dt,
            1.0f,
            sdl_key_name,
            sanitize_player_name);
        route_story_typewriter_audio(
            runtime,
            runtime.story_intro.dialogue_writing,
            visible_before,
            runtime.story_intro.visible_chars);
        if (runtime.story_intro.phase != StoryIntroPhase::PlayMatch) {
            auto& state = simulation.mutable_state();
            const auto& config = simulation.config();
            set_paddle_execution_full(state.left);
            set_paddle_execution_full(state.right);
            state.left.target_y = 0.5f * config.court_height;
            state.right.target_y = 0.5f * config.court_height;
            state.left.feedforward_velocity_y = 0.0f;
            state.right.feedforward_velocity_y = 0.0f;
            state.ball.position.x = 0.5f * config.court_width;
            state.ball.position.y = 0.5f * config.court_height;
            state.ball.velocity.x = 0.0f;
            state.ball.velocity.y = 0.0f;
            state.ball.spin = 0.0f;
            state.ball.speed_scalar = 1.0f;
        }
    } else if (runtime.app_state == AppState::StoryScene) {
        const std::size_t visible_before = runtime.story_scene.visible_chars;
        update_story_scene_typewriter(runtime.story_scene, dt, 1.0f);
        route_story_typewriter_audio(
            runtime,
            runtime.story_scene.dialogue_writing,
            visible_before,
            runtime.story_scene.visible_chars);
        auto& state = simulation.mutable_state();
        const auto& config = simulation.config();
        set_paddle_execution_full(state.left);
        set_paddle_execution_full(state.right);
        state.left.target_y = 0.5f * config.court_height;
        state.right.target_y = 0.5f * config.court_height;
        state.left.feedforward_velocity_y = 0.0f;
        state.right.feedforward_velocity_y = 0.0f;
        state.ball.position.x = 0.5f * config.court_width;
        state.ball.position.y = 0.5f * config.court_height;
        state.ball.velocity.x = 0.0f;
        state.ball.velocity.y = 0.0f;
        state.ball.spin = 0.0f;
        state.ball.speed_scalar = 1.0f;
    }
}

whacker::progression::SkillState resolve_story_active_rival_skills(const StoryRuntimeState& story_runtime) {
    if (story_runtime.active_rival_id != StoryRivalId::None) {
        return story_runtime.active_rival_skills;
    }
    if (story_runtime.active_match == StoryMatchKind::None) {
        return {};
    }
    const StoryMatchPolicyDescriptor& policy = story_match_policy_for_kind(story_runtime.active_match);
    return story_policy_rival_spec_for_career(policy, story_runtime.career).skills;
}

void update_match_opening_countdown_and_play_cue(
    SdlRuntimeState& runtime,
    whacker::sim::Simulation& simulation,
    const float dt_seconds) {
    const bool was_active = runtime.match_flow.opening_countdown_active;
    const bool was_visible = runtime.match_flow.opening_ball_visible;
    const float elapsed_before = runtime.match_flow.opening_countdown_elapsed;
    (void)update_match_opening_countdown(runtime.match_flow, simulation, dt_seconds);
    if (!was_active) {
        return;
    }
    const bool first_flash = elapsed_before <= 1.0e-6f && runtime.match_flow.opening_countdown_active;
    const bool visible_flash =
        runtime.match_flow.opening_countdown_active && !was_visible && runtime.match_flow.opening_ball_visible;
    if (first_flash || visible_flash) {
        play_match_opening_countdown_cue(runtime);
    }
}

void step_match_simulation(
    const ActionInputFrame& input,
    SdlRuntimeState& runtime,
    whacker::sim::Simulation& simulation) {
    update_match_opening_countdown_and_play_cue(runtime, simulation, whacker::sim::kFixedDt);
    const whacker::sim::RallyState before = simulation.state();

    PlayControlOverrides overrides {};
    PlayControlOverrides* overrides_ptr = nullptr;
    StoryMatchPolicyDescriptor story_policy = story_match_policy_fallback();
    const bool has_active_story_match = runtime.story_runtime.active_match != StoryMatchKind::None;
    if (runtime.app_state == AppState::StoryIntro && runtime.story_intro.phase == StoryIntroPhase::PlayMatch) {
        overrides.force_modes = true;
        overrides.left_mode = runtime.story_intro.player_is_right ? PaddleMode::AI : PaddleMode::Human;
        overrides.right_mode = runtime.story_intro.player_is_right ? PaddleMode::Human : PaddleMode::AI;
        overrides.override_left_skills = true;
        overrides.override_right_skills = true;
        if (runtime.story_intro.player_is_right) {
            overrides.left_skills = runtime.story_intro.rival_skills;
            overrides.right_skills = runtime.story_runtime.career.player_skills;
        } else {
            overrides.left_skills = runtime.story_runtime.career.player_skills;
            overrides.right_skills = runtime.story_intro.rival_skills;
        }
        overrides_ptr = &overrides;
    } else if (has_active_story_match) {
        story_policy = story_match_policy_for_kind(runtime.story_runtime.active_match);
        const bool player_is_right = runtime.story_runtime.career.prefers_right_side;
        const whacker::progression::SkillState rival_skills = resolve_story_active_rival_skills(runtime.story_runtime);
        const int points_played = std::max(0, before.left_score + before.right_score);
        const bool ai_preview_active =
            story_policy.ai_preview_points > 0 && points_played < story_policy.ai_preview_points;
        if (runtime.story_runtime.active_match == StoryMatchKind::Imagination1967 &&
            !ai_preview_active &&
            !runtime.story_runtime.imagination_takeover_cue_shown) {
            runtime.story_runtime.imagination_takeover_cue_shown = true;
            runtime.story_runtime.imagination_takeover_cue_seconds = 1.15f;
        }
        overrides.force_modes = true;
        if (ai_preview_active) {
            overrides.left_mode = PaddleMode::AI;
            overrides.right_mode = PaddleMode::AI;
        } else {
            overrides.left_mode = player_is_right ? PaddleMode::AI : PaddleMode::Human;
            overrides.right_mode = player_is_right ? PaddleMode::Human : PaddleMode::AI;
        }
        overrides.ai_training_context = story_policy.ai_training_context;
        overrides.override_left_skills = true;
        overrides.override_right_skills = true;
        if (runtime.story_runtime.active_match == StoryMatchKind::Imagination1967) {
            const whacker::progression::SkillState champion_player = story_script_imagination_1967_player_skills();
            const whacker::progression::SkillState champion_rival = story_script_imagination_1967_rival_skills();
            if (player_is_right) {
                overrides.left_skills = champion_rival;
                overrides.right_skills = champion_player;
            } else {
                overrides.left_skills = champion_player;
                overrides.right_skills = champion_rival;
            }
        } else if (player_is_right) {
            overrides.left_skills = rival_skills;
            overrides.right_skills = runtime.story_runtime.career.player_skills;
        } else {
            overrides.left_skills = runtime.story_runtime.career.player_skills;
            overrides.right_skills = rival_skills;
        }
        overrides_ptr = &overrides;
    }

    update_targets_for_play(
        simulation,
        runtime.options,
        runtime.left_ai,
        runtime.right_ai,
        whacker::sim::kFixedDt,
        input.p1_move_y,
        input.p2_move_y,
        overrides_ptr);
    const whacker::sim::ScoreEvent score_event = simulation.step(whacker::sim::kFixedDt);
    const whacker::sim::RallyState after = simulation.state();
    route_step_audio_events(runtime, before, after, score_event, simulation.config());

    if (runtime.app_state == AppState::StoryIntro && runtime.story_intro.phase == StoryIntroPhase::PlayMatch) {
        track_story_intro_contact_usage(runtime.story_intro, simulation.config(), before, after);
        if (score_event != whacker::sim::ScoreEvent::None) {
            const bool player_scored_point =
                runtime.story_intro.player_is_right
                    ? score_event == whacker::sim::ScoreEvent::RightPlayerScored
                    : score_event == whacker::sim::ScoreEvent::LeftPlayerScored;
            runtime.story_intro.player_scored = player_scored_point;
            runtime.story_intro.points_played += 1;

            int game_winner = 0;
            const bool game_complete = table_tennis_game_complete(after.left_score, after.right_score, &game_winner);
            if (!game_complete) {
                update_serve_after_scored_point(runtime.match_flow, after, simulation);
                if (runtime.story_intro.points_played == 1) {
                    runtime.story_intro.break_kind = StoryIntroBreak::SwapSides;
                    runtime.story_intro.swap_choice = runtime.story_intro.player_is_right ? 1 : 0;
                    runtime.story_intro.phase = StoryIntroPhase::BetweenBalls;
                    runtime.story_intro.phase_timer = 0.0f;
                    reset_story_intro_typewriter(runtime.story_intro);
                } else if (runtime.story_intro.points_played == 2 && !player_scored_point) {
                    runtime.story_intro.break_kind = StoryIntroBreak::Controls;
                    runtime.story_intro.phase = StoryIntroPhase::BetweenBalls;
                    runtime.story_intro.phase_timer = 0.0f;
                    reset_story_intro_typewriter(runtime.story_intro);
                } else if (!runtime.story_intro.name_prompted && runtime.story_intro.points_played == 3) {
                    runtime.story_intro.name_prompted = true;
                    runtime.story_intro.name_accept_pending = false;
                    runtime.story_intro.name_missing_prompt = false;
                    runtime.story_intro.phase = StoryIntroPhase::NameEntry;
                    runtime.story_intro.phase_timer = 0.0f;
                    reset_story_intro_typewriter(runtime.story_intro);
                } else if (!runtime.story_intro.rules_hint_shown && runtime.story_intro.points_played == 5) {
                    runtime.story_intro.rules_hint_shown = true;
                    runtime.story_intro.break_kind = StoryIntroBreak::Rules;
                    runtime.story_intro.phase = StoryIntroPhase::BetweenBalls;
                    runtime.story_intro.phase_timer = 0.0f;
                    reset_story_intro_typewriter(runtime.story_intro);
                }
            } else {
                runtime.story_intro.player_won =
                    runtime.story_intro.player_is_right ? (game_winner < 0) : (game_winner > 0);
                runtime.story_intro.player_forfeited = false;
                runtime.story_intro.final_left_score = after.left_score;
                runtime.story_intro.final_right_score = after.right_score;
                runtime.story_intro.phase = StoryIntroPhase::RivalIntro;
                runtime.story_intro.phase_timer = 0.0f;
                reset_story_intro_typewriter(runtime.story_intro);
                reset_match_flow(runtime.match_flow);
            }
        }
        return;
    }

    if (has_active_story_match) {
        update_story_match_tracking(
            runtime.story_runtime,
            simulation.config(),
            before,
            after,
            whacker::sim::kFixedDt);
    }
    if (score_event == whacker::sim::ScoreEvent::None) {
        return;
    }
    if (has_active_story_match) {
        switch (story_policy.score_model) {
            case StoryMatchScoreModel::BestOfGames: {
                int game_winner = 0;
                if (table_tennis_game_complete(after.left_score, after.right_score, &game_winner)) {
                    if (game_winner > 0) {
                        runtime.story_runtime.official_games_left += 1;
                    } else if (game_winner < 0) {
                        runtime.story_runtime.official_games_right += 1;
                    }
                    if (runtime.story_runtime.official_games_left >= story_official_games_to_win() ||
                        runtime.story_runtime.official_games_right >= story_official_games_to_win()) {
                        finish_active_or_quick_match(runtime, simulation, StoryMatchEndReason::Completed);
                    } else {
                        start_next_table_tennis_game(runtime.match_flow, simulation, true);
                    }
                } else {
                    update_serve_after_scored_point(runtime.match_flow, after, simulation);
                }
                break;
            }
            case StoryMatchScoreModel::RallyLoop:
                update_serve_after_scored_point(runtime.match_flow, after, simulation);
                break;
            case StoryMatchScoreModel::SingleGame:
            case StoryMatchScoreModel::None:
            default: {
                int game_winner = 0;
                if (table_tennis_game_complete(after.left_score, after.right_score, &game_winner)) {
                    finish_active_or_quick_match(runtime, simulation, StoryMatchEndReason::Completed);
                } else {
                    update_serve_after_scored_point(runtime.match_flow, after, simulation);
                }
                break;
            }
        }
    } else {
        int game_winner = 0;
        if (table_tennis_game_complete(after.left_score, after.right_score, &game_winner)) {
            finish_active_or_quick_match(runtime, simulation, StoryMatchEndReason::Completed);
        } else {
            update_serve_after_scored_point(runtime.match_flow, after, simulation);
        }
    }
}

void update_playing(
    const ActionInputFrame& input,
    const double frame_dt,
    SdlRuntimeState& runtime,
    whacker::sim::Simulation& simulation) {
    if (input_pressed(input, InputAction::Pause)) {
        play_menu_confirm_sound(runtime);
        runtime.pause_return_state = runtime.app_state;
        runtime.pause_menu.selected_row = PauseMenuRowResume;
        runtime.pause_menu.confirm_forfeit = false;
        runtime.pause_menu.confirm_selected = 0;
        runtime.app_state = AppState::Paused;
        runtime.accumulator = 0.0;
        return;
    }

    runtime.accumulator = std::min(runtime.accumulator + frame_dt, 0.25);
    while (runtime.accumulator >= static_cast<double>(whacker::sim::kFixedDt)) {
        step_match_simulation(input, runtime, simulation);
        runtime.accumulator -= static_cast<double>(whacker::sim::kFixedDt);
    }
}

void update_paused(
    const ActionInputFrame& input,
    SdlRuntimeState& runtime,
    whacker::sim::Simulation& simulation) {
    const int row_before = runtime.pause_menu.selected_row;
    const bool confirm_before = runtime.pause_menu.confirm_forfeit;
    const int confirm_selected_before = runtime.pause_menu.confirm_selected;
    const MatchExitPolicy exit_policy = compute_runtime_match_exit_policy(
        simulation,
        runtime.app_state,
        runtime.pause_return_state,
        runtime.match_flow,
        runtime.story_runtime,
        runtime.story_intro);
    const PauseMenuActionResult result = apply_pause_menu_action_frame(
        runtime.pause_menu,
        exit_policy,
        input,
        input_pressed(input, InputAction::Pause));
    if (runtime.pause_menu.selected_row != row_before ||
        runtime.pause_menu.confirm_selected != confirm_selected_before) {
        play_menu_move_sound(runtime);
    }
    if (result != PauseMenuActionResult::None || runtime.pause_menu.confirm_forfeit != confirm_before) {
        play_menu_confirm_sound(runtime);
    }
    if (result == PauseMenuActionResult::Resume) {
        runtime.app_state = runtime.pause_return_state;
    } else if (result == PauseMenuActionResult::ExitMatch) {
        execute_runtime_pause_exit(
            exit_policy,
            runtime.story_runtime,
            runtime.story_hub,
            runtime.story_intro,
            runtime.match_flow,
            simulation,
            runtime.story_scene,
            runtime.authored_transition_request,
            runtime.app_state,
            story_official_games_to_win(),
            sanitize_player_name,
            save_story_career);
    } else if (result == PauseMenuActionResult::QuitToMainMenu) {
        quit_runtime_to_main_menu(
            runtime.story_runtime,
            runtime.story_hub,
            runtime.story_intro,
            runtime.story_scene,
            runtime.match_flow,
            runtime.pause_menu,
            runtime.pause_return_state,
            simulation,
            runtime.authored_transition_request,
            story_official_games_to_win(),
            save_story_career,
            runtime.app_state);
    }
    runtime.accumulator = 0.0;
}

}  // namespace

void update_runtime(
    const ActionInputFrame& input,
    const SdlEventFrame& events,
    const RenderContext& render_context,
    const double frame_dt,
    SdlRuntimeState& runtime,
    whacker::sim::Simulation& simulation,
    SdlPlatform& platform) {
    if (runtime.authored_transition_request.armed && !runtime.visual_transition.active) {
        begin_visual_transition_for_authored_request(
            runtime.visual_transition,
            runtime.authored_transition_request,
            platform.now_seconds());
        clear_authored_transition_request(runtime.authored_transition_request);
    }
    if (runtime.visual_transition.active) {
        advance_visual_transition(
            runtime.visual_transition,
            runtime.app_state,
            runtime.story_scene,
            platform.now_seconds());
    }

    if (runtime.app_state == AppState::MainMenu) {
        update_main_menu(input, runtime, platform);
    } else if (runtime.app_state == AppState::OptionsMenu) {
        update_options_menu(input, events, runtime);
    } else if (runtime.app_state == AppState::QuickMatchSetup) {
        update_quick_match_setup(input, runtime, simulation);
    } else if (runtime.app_state == AppState::PaddleTuning) {
        update_paddle_tuning(input, runtime);
    } else if (runtime.app_state == AppState::StoryMenu) {
        update_story_menu(input, simulation, runtime);
    } else if (runtime.app_state == AppState::StoryIntro) {
        update_story_intro_dialogue_input(input, events, render_context, runtime, simulation);
        if (runtime.app_state == AppState::StoryIntro && runtime.story_intro.phase == StoryIntroPhase::PlayMatch) {
            update_playing(input, frame_dt, runtime, simulation);
        } else {
            update_dialogue_step(runtime, frame_dt, simulation);
            runtime.accumulator = 0.0;
        }
    } else if (runtime.app_state == AppState::StoryScene) {
        update_story_scene_input(input, render_context, runtime, simulation);
        update_dialogue_step(runtime, frame_dt, simulation);
        runtime.accumulator = 0.0;
    } else if (runtime.app_state == AppState::StoryHub) {
        update_story_hub(input, runtime, simulation);
    } else if (runtime.app_state == AppState::Playing) {
        update_playing(input, frame_dt, runtime, simulation);
    } else if (runtime.app_state == AppState::Paused) {
        update_paused(input, runtime, simulation);
    }
}

}  // namespace whacker::app
