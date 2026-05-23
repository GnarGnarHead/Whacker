#include "sdl_runtime_update.hpp"

#include <algorithm>
#include <cstddef>
#include <string>

#include "control_plan.hpp"
#include "main_menu_actions.hpp"
#include "match_exit_policy.hpp"
#include "match_flow.hpp"
#include "menu_intent.hpp"
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
#include "sim/math.hpp"
#include "story_hub_controller.hpp"
#include "story_intro_text_layout.hpp"
#include "story_match.hpp"
#include "story_menu_controller.hpp"
#include "story_name_entry.hpp"
#include "story_play_session.hpp"
#include "story_runtime_invariants.hpp"
#include "story_save.hpp"
#include "story_scene_text_layout.hpp"
#include "story_script_catalog.hpp"
#include "story_skill_limits.hpp"

namespace whacker::app {

namespace {

void apply_replace_route(const ScreenRoute route, SdlRuntimeState& runtime) {
    if (route.changed) {
        replace_runtime_screen(runtime, route.screen);
    }
}

void update_main_menu(
    const MenuIntent& intent,
    SdlRuntimeState& runtime,
    SdlPlatform& platform) {
    const int previous_row = runtime.main_menu.selected_row;
    const MainMenuActionResult result = apply_main_menu_action(runtime.main_menu, intent);
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
    const MenuIntent& intent,
    const SdlEventFrame& events,
    SdlRuntimeState& runtime) {
    const SdlOptionsUpdateEffects effects = update_sdl_options_menu(runtime, intent, events);
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
        (void)pop_runtime_screen(runtime);
    }
    runtime.accumulator = 0.0;
}

void apply_story_menu_route(
    const StoryMenuRoute route,
    SdlRuntimeState& runtime) {
    switch (route) {
        case StoryMenuRoute::None:
            return;
        case StoryMenuRoute::Back:
            (void)pop_runtime_screen(runtime);
            return;
        case StoryMenuRoute::StoryIntro:
            replace_runtime_screen(runtime, Screen::StoryIntro);
            return;
        case StoryMenuRoute::StoryHub:
            replace_runtime_screen(runtime, Screen::StoryHub);
            return;
        case StoryMenuRoute::StoryScene:
            push_runtime_screen(runtime, Screen::StoryScene);
            return;
    }
}

void update_story_menu(
    const MenuIntent& intent,
    whacker::sim::Simulation& simulation,
    SdlRuntimeState& runtime) {
    const StoryMenuControllerEffects effects = update_story_menu_controller(
        StoryMenuControllerContext {
            .menu = runtime.story_menu,
            .story_runtime = runtime.story_runtime,
            .story_hub = runtime.story_hub,
            .story_intro = runtime.story_intro,
            .story_scene = runtime.story_scene,
            .options = runtime.options,
            .match_flow = runtime.match_flow,
            .simulation = simulation,
            .feedback = &runtime.story_menu_feedback,
        },
        intent,
        story_save_exists(),
        load_story_career,
        reset_story_career);
    if (effects.play_move_sound) {
        play_menu_move_sound(runtime);
    }
    if (effects.play_confirm_sound) {
        play_menu_confirm_sound(runtime);
    }
    apply_story_menu_route(effects.route, runtime);
    runtime.accumulator = 0.0;
}

void apply_story_hub_route(
    const StoryHubRoute route,
    SdlRuntimeState& runtime) {
    switch (route) {
        case StoryHubRoute::None:
            return;
        case StoryHubRoute::Back:
            (void)pop_runtime_screen(runtime);
            return;
        case StoryHubRoute::StoryMenu:
            replace_runtime_screen(runtime, Screen::StoryMenu);
            return;
        case StoryHubRoute::StoryScene:
            push_runtime_screen(runtime, Screen::StoryScene);
            return;
        case StoryHubRoute::PaddleTuning:
            push_runtime_screen(runtime, Screen::PaddleTuning);
            return;
        case StoryHubRoute::Playing:
            replace_runtime_screen(runtime, Screen::Playing);
            return;
    }
}

void update_story_hub(
    const MenuIntent& intent,
    SdlRuntimeState& runtime,
    whacker::sim::Simulation& simulation) {
    const StoryHubControllerEffects effects = update_story_hub_controller(
        StoryHubControllerContext {
            .story_runtime = runtime.story_runtime,
            .story_hub = runtime.story_hub,
            .story_scene = runtime.story_scene,
            .paddle_tuning = runtime.paddle_tuning,
            .options = runtime.options,
            .match_flow = runtime.match_flow,
            .simulation = simulation,
            .rng = runtime.rng,
        },
        intent,
        save_story_career);
    if (effects.play_move_sound) {
        play_menu_move_sound(runtime);
    }
    if (effects.play_confirm_sound) {
        play_menu_confirm_sound(runtime);
    }
    apply_story_hub_route(effects.route, runtime);
}

void update_quick_match_setup(
    const MenuIntent& intent,
    SdlRuntimeState& runtime,
    whacker::sim::Simulation& simulation) {
    const QuickMenuActionResult result =
        apply_quick_menu_action(runtime.quick_menu, runtime.options, intent);

    if (result.back_requested) {
        play_menu_confirm_sound(runtime);
        (void)pop_runtime_screen(runtime);
        return;
    }

    if (result.row_changed || result.options_changed) {
        play_menu_move_sound(runtime);
    }
    if (result.start_requested) {
        play_menu_confirm_sound(runtime);
        start_quick_match(runtime, simulation);
    } else if (result.tune_p1_requested) {
        play_menu_confirm_sound(runtime);
        begin_quick_paddle_tuning(
            runtime.paddle_tuning,
            PaddleTuningTarget::QuickLeft,
            runtime.options.left_paddle_skills);
        push_runtime_screen(runtime, Screen::PaddleTuning);
    } else if (result.tune_p2_requested) {
        play_menu_confirm_sound(runtime);
        begin_quick_paddle_tuning(
            runtime.paddle_tuning,
            PaddleTuningTarget::QuickRight,
            runtime.options.right_paddle_skills);
        push_runtime_screen(runtime, Screen::PaddleTuning);
    }
    if (result.options_changed) {
        persist_runtime_menu_settings(runtime);
    }
    runtime.accumulator = 0.0;
}

void update_paddle_tuning(
    const PaddleTuningInputIntent& intent,
    SdlRuntimeState& runtime) {
    const PaddleTuningActionResult result =
        apply_paddle_tuning_action(runtime.paddle_tuning, intent);
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
        (void)pop_runtime_screen(runtime);
    } else if (result == PaddleTuningActionResult::Cancel) {
        play_menu_confirm_sound(runtime);
        runtime.paddle_tuning.active = false;
        (void)pop_runtime_screen(runtime);
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
    const bool editing_name = runtime.story_intro.phase == StoryIntroPhase::NameEntry;
    if (editing_name) {
        prepare_story_name_entry(runtime.story_intro);
    }

    const StoryIntroBodyLayout body_layout = compute_story_intro_body_layout_for_framebuffer(
        render_context.framebuffer_width,
        render_context.framebuffer_height,
        runtime.story_intro,
        runtime.controls,
        sdl_key_name,
        sanitize_player_name);

    if (!editing_name && !runtime.story_intro.dialogue_writing && body_layout.max_scroll_rows > 0) {
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

    if (editing_name && !runtime.story_intro.dialogue_writing) {
        const MenuIntent intent = derive_menu_intent(input);
        const StoryNameEntryEditResult edit_result = apply_story_name_entry_input(
            runtime.story_intro,
            intent,
            StoryNameTextInput {
                .text = events.text_input,
                .backspace_pressed = events.backspace_pressed,
            });
        const bool controller_edit =
            intent.left || intent.right || intent.up || intent.down || intent.back || events.backspace_pressed;
        if (edit_result.changed && controller_edit) {
            play_menu_move_sound(runtime);
        }
        if (edit_result.consumed_back) {
            return;
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
        (void)confirm_story_name_entry(runtime.story_intro, sanitize_player_name);
        return;
    }

    if (runtime.story_intro.phase == StoryIntroPhase::RivalIntro) {
        const StoryIntroCompleteResult result = complete_story_intro(
            runtime.story_runtime,
            runtime.story_hub,
            runtime.story_intro,
            runtime.match_flow,
            simulation,
            runtime.authored_transition_request,
            sanitize_player_name,
            save_story_career);
        apply_replace_route(result.route, runtime);
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
        if (pop_runtime_screen(runtime)) {
            clear_story_scene(runtime.story_scene);
            clear_story_runtime_scene_pending_flags(runtime.story_runtime);
            runtime.accumulator = 0.0;
        }
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
    const StorySceneConfirmResult result = handle_story_scene_confirm(
        runtime.story_scene,
        runtime.story_runtime,
        runtime.story_hub,
        runtime.options,
        runtime.match_flow,
        simulation,
        runtime.rng,
        runtime.authored_transition_request,
        save_story_career);
    apply_replace_route(result.route, runtime);
}

void update_dialogue_step(
    SdlRuntimeState& runtime,
    const double frame_dt,
    whacker::sim::Simulation& simulation) {
    const float dt = static_cast<float>(frame_dt);
    tick_story_typewriter_audio(runtime, dt);
    const Screen screen = runtime.navigation.current;
    if (screen == Screen::StoryIntro) {
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
    } else if (screen == Screen::StoryScene) {
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
    MatchControlPlan control_plan = quick_match_control_plan();
    const bool has_active_story_match = runtime.story_runtime.active_match != StoryMatchKind::None;
    const Screen screen = runtime.navigation.current;
    if (screen == Screen::StoryIntro && runtime.story_intro.phase == StoryIntroPhase::PlayMatch) {
        control_plan = story_player_control_plan(
            runtime.story_intro.player_is_right ? CourtSide::Right : CourtSide::Left);
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
        control_plan = story_player_control_plan(player_is_right ? CourtSide::Right : CourtSide::Left);
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
        control_plan,
        InputSlotAxes {
            .p1_move_y = input.p1_move_y,
            .p2_move_y = input.p2_move_y,
        },
        overrides_ptr);
    const whacker::sim::ScoreEvent score_event = simulation.step(whacker::sim::kFixedDt);
    const whacker::sim::RallyState after = simulation.state();
    route_step_audio_events(runtime, before, after, score_event, simulation.config());

    if (screen == Screen::StoryIntro && runtime.story_intro.phase == StoryIntroPhase::PlayMatch) {
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
                    reset_story_name_entry_editor(runtime.story_intro);
                    prepare_story_name_entry(runtime.story_intro);
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
        runtime.pause_menu.selected_row = PauseMenuRowResume;
        runtime.pause_menu.confirm_forfeit = false;
        runtime.pause_menu.confirm_selected = 0;
        push_runtime_screen(runtime, Screen::Paused);
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
    const PauseMenuIntent& intent,
    SdlRuntimeState& runtime,
    whacker::sim::Simulation& simulation) {
    const int row_before = runtime.pause_menu.selected_row;
    const bool confirm_before = runtime.pause_menu.confirm_forfeit;
    const int confirm_selected_before = runtime.pause_menu.confirm_selected;
    const MatchExitPolicy exit_policy = compute_runtime_match_exit_policy(
        simulation,
        runtime_active_screen(runtime),
        runtime.match_flow,
        runtime.story_runtime,
        runtime.story_intro);
    const PauseMenuActionResult result = apply_pause_menu_action(
        runtime.pause_menu,
        exit_policy,
        intent);
    if (runtime.pause_menu.selected_row != row_before ||
        runtime.pause_menu.confirm_selected != confirm_selected_before) {
        play_menu_move_sound(runtime);
    }
    if (result != PauseMenuActionResult::None || runtime.pause_menu.confirm_forfeit != confirm_before) {
        play_menu_confirm_sound(runtime);
    }
    if (result == PauseMenuActionResult::Resume) {
        (void)pop_runtime_screen(runtime);
    } else if (result == PauseMenuActionResult::ExitMatch) {
        const ScreenRoute route = execute_runtime_pause_exit(
            exit_policy,
            runtime.story_runtime,
            runtime.story_hub,
            runtime.story_intro,
            runtime.match_flow,
            simulation,
            runtime.story_scene,
            runtime.authored_transition_request,
            runtime_active_screen(runtime),
            story_official_games_to_win(),
            sanitize_player_name,
            save_story_career);
        if (runtime.navigation.current == Screen::Paused) {
            (void)pop_runtime_screen(runtime);
        }
        apply_replace_route(route, runtime);
    } else if (result == PauseMenuActionResult::QuitToMainMenu) {
        quit_runtime_to_main_menu(
            runtime.story_runtime,
            runtime.story_hub,
            runtime.story_intro,
            runtime.story_scene,
            runtime.match_flow,
            runtime.pause_menu,
            simulation,
            runtime.authored_transition_request,
            runtime_active_screen(runtime),
            story_official_games_to_win(),
            save_story_career);
        reset_runtime_to_root(runtime, Screen::MainMenu);
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
        const ScreenRoute route = advance_visual_transition(
            runtime.visual_transition,
            runtime.story_scene,
            platform.now_seconds());
        apply_replace_route(route, runtime);
    }

    const MenuInputIntent menu_input = derive_menu_input_intent(input);
    const MenuIntent& menu_intent = menu_input.pressed;

    const Screen screen = runtime.navigation.current;
    if (screen == Screen::MainMenu) {
        update_main_menu(menu_intent, runtime, platform);
    } else if (screen == Screen::OptionsMenu) {
        update_options_menu(menu_intent, events, runtime);
    } else if (screen == Screen::QuickMatchSetup) {
        update_quick_match_setup(menu_intent, runtime, simulation);
    } else if (screen == Screen::PaddleTuning) {
        update_paddle_tuning(
            PaddleTuningInputIntent {
                .pressed = menu_intent,
                .held = menu_input.held,
                .pause = menu_input.pause,
            },
            runtime);
    } else if (screen == Screen::StoryMenu) {
        update_story_menu(menu_intent, simulation, runtime);
    } else if (screen == Screen::StoryIntro) {
        update_story_intro_dialogue_input(input, events, render_context, runtime, simulation);
        if (runtime.navigation.current == Screen::StoryIntro &&
            runtime.story_intro.phase == StoryIntroPhase::PlayMatch) {
            update_playing(input, frame_dt, runtime, simulation);
        } else {
            update_dialogue_step(runtime, frame_dt, simulation);
            runtime.accumulator = 0.0;
        }
    } else if (screen == Screen::StoryScene) {
        update_story_scene_input(input, render_context, runtime, simulation);
        update_dialogue_step(runtime, frame_dt, simulation);
        runtime.accumulator = 0.0;
    } else if (screen == Screen::StoryHub) {
        update_story_hub(menu_intent, runtime, simulation);
    } else if (screen == Screen::Playing) {
        update_playing(input, frame_dt, runtime, simulation);
    } else if (screen == Screen::Paused) {
        update_paused(
            PauseMenuIntent {
                .menu = menu_intent,
                .pause = menu_input.pause,
            },
            runtime,
            simulation);
    }
}

}  // namespace whacker::app
