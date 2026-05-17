#include "runtime_input_phase_internal.hpp"

#ifdef WHACKER_HAS_GLFW

#include <algorithm>
#include <cmath>

#include <GLFW/glfw3.h>

#include "match_exit_policy.hpp"
#include "menu_flow.hpp"
#include "menu_settings.hpp"
#include "options_menu_input.hpp"
#include "runtime_escape.hpp"
#include "runtime_pause.hpp"
#include "runtime_story_scene.hpp"
#include "runtime_transitions.hpp"
#include "story_flow.hpp"
#include "story_intro.hpp"
#include "story_runtime_invariants.hpp"
#include "story_save_helpers.hpp"
#include "story_save.hpp"
#include "story_scene.hpp"
#include "story_scene_text_layout.hpp"
#include "story_skill_limits.hpp"
#include "text_utils.hpp"

namespace whacker::app {

namespace {

constexpr int kPaddleTuningComponentCount = 3;
constexpr int kPaddleTuningAdjustLeft = -1;
constexpr int kPaddleTuningAdjustRight = 1;

float clampf(const float value, const float lo, const float hi) {
    return std::max(lo, std::min(value, hi));
}

float paddle_tuning_component_max(const whacker::progression::SkillState& max_skills, const int component_index) {
    switch (component_index) {
        case 0:
            return max_skills.edge;
        case 1:
            return max_skills.power;
        case 2:
            return max_skills.spin_inject;
        default:
            return max_skills.edge;
    }
}

whacker::progression::SkillState clamp_skill_components(const whacker::progression::SkillState& skills) {
    whacker::progression::SkillState clamped = skills;
    clamped.edge = clampf(clamped.edge, 0.0f, 1.0f);
    clamped.power = clampf(clamped.power, 0.0f, 1.0f);
    clamped.spin_inject = clampf(clamped.spin_inject, 0.0f, 1.0f);
    return clamped;
}

float paddle_tuning_limits_budget(
    const whacker::progression::SkillState& max_skills,
    const float requested_budget_limit) {
    const float caps_sum = story_skill_sum(max_skills);
    return clampf(std::min(requested_budget_limit, caps_sum), 0.0f, kPaddleTuningBudgetCap);
}

void clamp_paddle_tuning_to_limits(
    PaddleTuning& tuning,
    const whacker::progression::SkillState& max_skills,
    const float max_budget) {
    normalize_paddle_tuning(tuning);
    tuning.edge = std::min(tuning.edge, max_skills.edge);
    tuning.power = std::min(tuning.power, max_skills.power);
    tuning.spin_inject = std::min(tuning.spin_inject, max_skills.spin_inject);

    const float budget_limit = paddle_tuning_limits_budget(max_skills, max_budget);
    const float sum = tuning.edge + tuning.power + tuning.spin_inject;
    if (sum > budget_limit && sum > 1.0e-6f) {
        const float scale = budget_limit / sum;
        tuning.edge *= scale;
        tuning.power *= scale;
        tuning.spin_inject *= scale;
    }
    tuning.budget = clampf(tuning.edge + tuning.power + tuning.spin_inject, 0.0f, budget_limit);
}

float* paddle_tuning_component_ptr(PaddleTuning& tuning, const int component_index) {
    switch (component_index) {
        case 0:
            return &tuning.edge;
        case 1:
            return &tuning.power;
        case 2:
            return &tuning.spin_inject;
        default:
            return &tuning.edge;
    }
}

bool adjust_paddle_tuning_component(
    PaddleTuning& tuning,
    const whacker::progression::SkillState& max_skills,
    const float max_budget,
    const int component_index,
    const int direction) {
    float* selected_value = paddle_tuning_component_ptr(tuning, component_index);
    if (selected_value == nullptr || direction == 0) {
        return false;
    }
    const float before = *selected_value;
    if (direction < 0) {
        *selected_value = std::max(0.0f, *selected_value - kPaddleTuningBarStep);
    } else {
        const float other_sum = std::max(
            0.0f,
            (tuning.edge + tuning.power + tuning.spin_inject) - *selected_value);
        const float budget_limit = paddle_tuning_limits_budget(max_skills, max_budget);
        const float component_max = paddle_tuning_component_max(max_skills, component_index);
        const float max_for_selected =
            std::min(component_max, std::max(0.0f, budget_limit - other_sum));
        *selected_value = std::min(max_for_selected, *selected_value + kPaddleTuningBarStep);
    }
    return std::abs(*selected_value - before) > 1.0e-6f;
}

int resolve_horizontal_hold_direction(const bool left_down, const bool right_down) {
    if (left_down == right_down) {
        return 0;
    }
    return left_down ? -1 : 1;
}

bool should_fire_horizontal_hold_repeat(PaddleTuningState& tuning_state, const int hold_direction) {
    if (hold_direction == 0) {
        tuning_state.horizontal_hold_direction = 0;
        tuning_state.horizontal_hold_frames = 0;
        return false;
    }
    if (tuning_state.horizontal_hold_direction != hold_direction) {
        tuning_state.horizontal_hold_direction = hold_direction;
        tuning_state.horizontal_hold_frames = 0;
        return false;
    }
    ++tuning_state.horizontal_hold_frames;
    if (tuning_state.horizontal_hold_frames < kPaddleTuningHoldRepeatDelayFrames) {
        return false;
    }
    const int repeat_frames = tuning_state.horizontal_hold_frames - kPaddleTuningHoldRepeatDelayFrames;
    return (repeat_frames % kPaddleTuningHoldRepeatIntervalFrames) == 0;
}

void reset_paddle_tuning_horizontal_hold(PaddleTuningState& tuning_state) {
    tuning_state.horizontal_hold_direction = 0;
    tuning_state.horizontal_hold_frames = 0;
}

void initialize_paddle_tuning_state(
    PaddleTuningState& tuning_state,
    const AppState return_state,
    const PaddleTuningTarget target,
    const whacker::progression::SkillState& skills,
    const whacker::progression::SkillState& max_skills,
    const float max_budget) {
    const whacker::progression::SkillState clamped_max = clamp_skill_components(max_skills);
    tuning_state.active = true;
    tuning_state.return_state = return_state;
    tuning_state.target = target;
    tuning_state.selected_component = 0;
    reset_paddle_tuning_horizontal_hold(tuning_state);
    tuning_state.max_skills = clamped_max;
    tuning_state.max_budget = paddle_tuning_limits_budget(clamped_max, max_budget);
    tuning_state.working = paddle_tuning_from_skills(skills);
    clamp_paddle_tuning_to_limits(tuning_state.working, tuning_state.max_skills, tuning_state.max_budget);
}

void begin_quick_paddle_tuning_from_row(RuntimeInputPhaseContext& context, const int row) {
    static const whacker::progression::SkillState kQuickTuningMaxSkills {
        .edge = 1.0f,
        .power = 1.0f,
        .spin_inject = 1.0f,
    };
    if (row == MenuRowP2Tuning) {
        initialize_paddle_tuning_state(
            context.paddle_tuning_state,
            AppState::QuickMatchSetup,
            PaddleTuningTarget::QuickRight,
            context.options.right_paddle_skills,
            kQuickTuningMaxSkills,
            kPaddleTuningBudgetCap);
    } else {
        initialize_paddle_tuning_state(
            context.paddle_tuning_state,
            AppState::QuickMatchSetup,
            PaddleTuningTarget::QuickLeft,
            context.options.left_paddle_skills,
            kQuickTuningMaxSkills,
            kPaddleTuningBudgetCap);
    }
}

void begin_story_player_paddle_tuning(RuntimeInputPhaseContext& context) {
    normalize_story_player_skill_progress(
        context.story_runtime.career.player_skills,
        context.story_runtime.career.player_skill_caps);
    initialize_paddle_tuning_state(
        context.paddle_tuning_state,
        AppState::StoryHub,
        PaddleTuningTarget::StoryPlayer,
        context.story_runtime.career.player_skills,
        context.story_runtime.career.player_skill_caps,
        story_skill_sum(context.story_runtime.career.player_skill_caps));
}

bool consume_paddle_tuning_component_selection(RuntimeInputPhaseContext& context) {
    bool changed = false;
    if (consume_key_press(context.window, GLFW_KEY_UP, context.edge_state.up) ||
        consume_menu_up_press(context.window, context.edge_state, context.controls)) {
        context.paddle_tuning_state.selected_component =
            (context.paddle_tuning_state.selected_component + kPaddleTuningComponentCount - 1) %
            kPaddleTuningComponentCount;
        changed = true;
    }
    if (consume_key_press(context.window, GLFW_KEY_DOWN, context.edge_state.down) ||
        consume_menu_down_press(context.window, context.edge_state, context.controls)) {
        context.paddle_tuning_state.selected_component =
            (context.paddle_tuning_state.selected_component + 1) % kPaddleTuningComponentCount;
        changed = true;
    }
    return changed;
}

int consume_paddle_tuning_horizontal_edge_direction(RuntimeInputPhaseContext& context) {
    const bool left_pressed = consume_key_press(context.window, GLFW_KEY_LEFT, context.edge_state.left);
    const bool right_pressed = consume_key_press(context.window, GLFW_KEY_RIGHT, context.edge_state.right);
    if (left_pressed == right_pressed) {
        return 0;
    }
    return left_pressed ? kPaddleTuningAdjustLeft : kPaddleTuningAdjustRight;
}

bool consume_paddle_tuning_horizontal_adjustment(RuntimeInputPhaseContext& context) {
    const int edge_adjust_direction = consume_paddle_tuning_horizontal_edge_direction(context);
    const int hold_direction = resolve_horizontal_hold_direction(context.edge_state.left, context.edge_state.right);
    const bool repeat_triggered = should_fire_horizontal_hold_repeat(context.paddle_tuning_state, hold_direction);

    if (edge_adjust_direction != 0) {
        return adjust_paddle_tuning_component(
            context.paddle_tuning_state.working,
            context.paddle_tuning_state.max_skills,
            context.paddle_tuning_state.max_budget,
            context.paddle_tuning_state.selected_component,
            edge_adjust_direction);
    }
    if (repeat_triggered) {
        return adjust_paddle_tuning_component(
            context.paddle_tuning_state.working,
            context.paddle_tuning_state.max_skills,
            context.paddle_tuning_state.max_budget,
            context.paddle_tuning_state.selected_component,
            hold_direction);
    }
    return false;
}

void commit_paddle_tuning(RuntimeInputPhaseContext& context, RuntimeInputBranchEffects& effects) {
    clamp_paddle_tuning_to_limits(
        context.paddle_tuning_state.working,
        context.paddle_tuning_state.max_skills,
        context.paddle_tuning_state.max_budget);
    const whacker::progression::SkillState skills = paddle_tuning_to_skills(context.paddle_tuning_state.working);
    if (context.paddle_tuning_state.target == PaddleTuningTarget::QuickLeft) {
        context.options.left_paddle_skills = skills;
        context.options.left_ai_style = style_for_skills(skills);
        effects.persist_menu_settings = true;
    } else if (context.paddle_tuning_state.target == PaddleTuningTarget::QuickRight) {
        context.options.right_paddle_skills = skills;
        context.options.right_ai_style = style_for_skills(skills);
        effects.persist_menu_settings = true;
    } else {
        context.story_runtime.career.player_skills = skills;
        normalize_story_player_skill_progress(
            context.story_runtime.career.player_skills,
            context.story_runtime.career.player_skill_caps);
        (void)persist_story_career_with_feedback(
            context.story_runtime.career,
            save_story_career,
            &context.story_hub_state);
    }
    context.paddle_tuning_state.active = false;
    context.app_state = context.paddle_tuning_state.return_state;
    ++effects.menu_confirm_events;
}

void handle_runtime_escape_hotkey(RuntimeInputPhaseContext& context) {
    if (!consume_key_press(context.window, GLFW_KEY_ESCAPE, context.edge_state.escape)) {
        return;
    }
    const bool played_confirm = handle_runtime_escape_key(
        context.window,
        context.app_state,
        context.pause_return_state,
        context.story_menu_state,
        context.options_menu_state,
        context.pause_menu_state,
        context.story_scene_state,
        context.paddle_tuning_state,
        context.story_runtime);
    if (played_confirm) {
        context.audio_engine.push_event(AudioEventId::MenuConfirm);
    }
}

void apply_story_scene_pending_gate(RuntimeInputPhaseContext& context) {
    if (context.app_state != AppState::StoryScene) {
        return;
    }
    if (!context.story_runtime.onboarding_scene_pending && !context.story_runtime.post_forfeit_scene_pending) {
        return;
    }
    begin_story_onboarding_scene(context.story_scene_state, context.story_runtime);
    clear_story_runtime_scene_pending_flags(context.story_runtime);
}

void handle_runtime_menu_shortcut(RuntimeInputPhaseContext& context) {
    if (!consume_key_press(context.window, GLFW_KEY_M, context.edge_state.menu)) {
        return;
    }
    if (context.app_state == AppState::Playing && context.story_runtime.active_match == StoryMatchKind::None) {
        context.app_state = AppState::QuickMatchSetup;
    } else if (context.app_state == AppState::QuickMatchSetup && context.match_flow.mode == ActiveMatchMode::Quick) {
        context.app_state = AppState::Playing;
    }
}

bool runtime_dev_ai_toggle_enabled(const RuntimeInputPhaseContext& context) {
    return context.app_state == AppState::Playing ||
        (context.app_state == AppState::StoryIntro && context.story_intro_state.phase == StoryIntroPhase::PlayMatch);
}

void handle_runtime_dev_shortcuts(RuntimeInputPhaseContext& context, bool& show_dev_info) {
    if (consume_key_press(context.window, GLFW_KEY_F10, context.edge_state.dev_info)) {
        show_dev_info = !show_dev_info;
    }
    if (show_dev_info &&
        runtime_dev_ai_toggle_enabled(context) &&
        consume_key_press(context.window, GLFW_KEY_P, context.edge_state.dev_player_ai)) {
        context.ai_controls_player_paddle = !context.ai_controls_player_paddle;
    }
}

}  // namespace

void apply_runtime_input_branch_effects(const RuntimeInputBranchEffects& effects, RuntimeInputPhaseContext& context) {
    for (int i = 0; i < effects.menu_move_events; ++i) {
        context.audio_engine.push_event(AudioEventId::MenuMove);
    }
    for (int i = 0; i < effects.menu_confirm_events; ++i) {
        context.audio_engine.push_event(AudioEventId::MenuConfirm);
    }
    if (effects.persist_menu_settings) {
        save_menu_settings(context.options, context.controls, context.audio_settings);
    }
}

void handle_main_menu_branch(RuntimeInputPhaseContext& context) {
    const int row_before = context.main_menu_state.selected_row;
    const AppState state_before = context.app_state;
    handle_main_menu_input(
        context.window,
        context.edge_state,
        context.main_menu_state,
        context.menu_state,
        context.story_menu_state,
        context.options_menu_state,
        context.controls,
        context.app_state);
    RuntimeInputBranchEffects effects {};
    if (context.main_menu_state.selected_row != row_before) {
        ++effects.menu_move_events;
    }
    if (context.app_state != state_before) {
        ++effects.menu_confirm_events;
    }
    apply_runtime_input_branch_effects(effects, context);
}

void handle_options_menu_branch(RuntimeInputPhaseContext& context) {
    const int row_before = context.options_menu_state.selected_row;
    const bool waiting_before = context.options_menu_state.waiting_for_key;
    const AppState state_before = context.app_state;
    bool changed_bindings = false;
    bool changed_audio_settings = false;
    handle_options_menu_input(
        context.window,
        context.edge_state,
        context.options_menu_state,
        context.controls,
        context.audio_settings,
        context.app_state,
        changed_bindings,
        changed_audio_settings);
    RuntimeInputBranchEffects effects {};
    if (context.options_menu_state.selected_row != row_before) {
        ++effects.menu_move_events;
    }
    if (changed_audio_settings) {
        context.audio_settings = clamp_audio_settings(context.audio_settings);
        context.audio_engine.set_settings(context.audio_settings);
        ++effects.menu_move_events;
    }
    if (waiting_before != context.options_menu_state.waiting_for_key || changed_bindings || context.app_state != state_before) {
        ++effects.menu_confirm_events;
    }
    effects.persist_menu_settings = changed_bindings || changed_audio_settings;
    apply_runtime_input_branch_effects(effects, context);
}

void handle_quick_match_setup_branch(RuntimeInputPhaseContext& context) {
    const int row_before = context.menu_state.selected_row;
    const MatchOptions before_options = context.options;
    const AppState state_before = context.app_state;
    handle_menu_input(
        context.window,
        context.edge_state,
        context.menu_state,
        context.options,
        context.controls,
        context.match_flow,
        context.app_state,
        context.simulation,
        context.rng);
    if (state_before == AppState::QuickMatchSetup && context.app_state == AppState::PaddleTuning) {
        begin_quick_paddle_tuning_from_row(context, context.menu_state.selected_row);
    }
    const bool options_changed = !options_equal(before_options, context.options);
    RuntimeInputBranchEffects effects {};
    if (context.menu_state.selected_row != row_before || options_changed) {
        ++effects.menu_move_events;
    }
    if (context.app_state != state_before) {
        ++effects.menu_confirm_events;
    }
    effects.persist_menu_settings = options_changed;
    apply_runtime_input_branch_effects(effects, context);
}

void handle_story_menu_branch(RuntimeInputPhaseContext& context, const bool has_save) {
    const int row_before = context.story_menu_state.selected_row;
    const bool confirm_before = context.story_menu_state.confirm_overwrite;
    const int confirm_selected_before = context.story_menu_state.confirm_selected;
    const AppState state_before = context.app_state;
    handle_story_menu_input(
        context.window,
        context.edge_state,
        context.story_menu_state,
        context.story_runtime,
        context.story_hub_state,
        context.story_intro_state,
        context.options,
        context.controls,
        context.match_flow,
        context.simulation,
        context.app_state,
        has_save,
        load_story_career,
        reset_story_career);
    RuntimeInputBranchEffects effects {};
    if (context.story_menu_state.selected_row != row_before ||
        context.story_menu_state.confirm_selected != confirm_selected_before) {
        ++effects.menu_move_events;
    }
    if (context.app_state != state_before || context.story_menu_state.confirm_overwrite != confirm_before) {
        ++effects.menu_confirm_events;
    }
    apply_runtime_input_branch_effects(effects, context);
}

void handle_story_intro_branch(RuntimeInputPhaseContext& context) {
    const StoryIntroPhase phase_before = context.story_intro_state.phase;
    const StoryIntroBreak break_before = context.story_intro_state.break_kind;
    const bool accept_before = context.story_intro_state.name_accept_pending;
    const AppState state_before = context.app_state;
    handle_story_intro_input(
        context.window,
        context.edge_state,
        context.story_runtime,
        context.story_hub_state,
        context.story_intro_state,
        context.options,
        context.controls,
        context.match_flow,
        context.simulation,
        context.rng,
        context.app_state,
        context.authored_transition_request,
        key_to_name_char,
        trim_copy,
        sanitize_player_name,
        save_story_career);
    if (context.story_intro_state.phase != phase_before ||
        context.story_intro_state.break_kind != break_before ||
        context.story_intro_state.name_accept_pending != accept_before ||
        context.app_state != state_before) {
        context.audio_engine.push_event(AudioEventId::MenuConfirm);
    }
}

void handle_story_scene_branch(RuntimeInputPhaseContext& context) {
    RuntimeInputBranchEffects effects {};
    const StorySceneBodyLayout body_layout =
        compute_story_scene_body_layout_for_window(context.window, context.story_scene_state);
    context.story_scene_state.scroll_lines_from_bottom = clamp_story_scene_scroll_from_bottom(
        body_layout,
        context.story_scene_state.scroll_lines_from_bottom);

    const bool scroll_up =
        consume_key_press(context.window, GLFW_KEY_UP, context.edge_state.up) ||
        consume_menu_up_press(context.window, context.edge_state, context.controls);
    const bool scroll_down =
        consume_key_press(context.window, GLFW_KEY_DOWN, context.edge_state.down) ||
        consume_menu_down_press(context.window, context.edge_state, context.controls);
    if (scroll_up != scroll_down) {
        const int requested_scroll = context.story_scene_state.scroll_lines_from_bottom + (scroll_up ? 1 : -1);
        const int clamped_scroll = clamp_story_scene_scroll_from_bottom(body_layout, requested_scroll);
        if (clamped_scroll != context.story_scene_state.scroll_lines_from_bottom) {
            context.story_scene_state.scroll_lines_from_bottom = clamped_scroll;
            ++effects.menu_move_events;
        }
    }

    if (!context.story_scene_state.dialogue_writing && context.story_scene_state.has_binary_choice) {
        const bool toggle_choice =
            consume_key_press(context.window, GLFW_KEY_LEFT, context.edge_state.left) ||
            consume_key_press(context.window, GLFW_KEY_RIGHT, context.edge_state.right);
        if (toggle_choice) {
            context.story_scene_state.binary_choice_yes_selected = !context.story_scene_state.binary_choice_yes_selected;
            ++effects.menu_move_events;
        }
    }
    if (consume_confirm_press(context.window, context.edge_state)) {
        ++effects.menu_confirm_events;
        if (context.story_scene_state.scroll_lines_from_bottom > 0 && body_layout.max_scroll_lines > 0) {
            context.story_scene_state.scroll_lines_from_bottom = 0;
        } else {
            handle_story_scene_confirm(
                context.story_scene_state,
                context.story_runtime,
                context.story_hub_state,
                context.options,
                context.match_flow,
                context.simulation,
                context.rng,
                context.app_state,
                context.authored_transition_request,
                save_story_career);
        }
    }
    apply_runtime_input_branch_effects(effects, context);
}

void handle_story_hub_branch(RuntimeInputPhaseContext& context) {
    const int row_before = context.story_hub_state.selected_row;
    const int week_before = context.story_runtime.career.week;
    const AppState state_before = context.app_state;
    handle_story_hub_input(
        context.window,
        context.edge_state,
        context.story_runtime,
        context.story_hub_state,
        context.options,
        context.controls,
        context.match_flow,
        context.app_state,
        context.simulation,
        context.rng,
        save_story_career);
    if (state_before == AppState::StoryHub && context.app_state == AppState::PaddleTuning) {
        begin_story_player_paddle_tuning(context);
    }
    RuntimeInputBranchEffects effects {};
    if (context.story_hub_state.selected_row != row_before) {
        ++effects.menu_move_events;
    }
    if (context.app_state != state_before || context.story_runtime.career.week != week_before) {
        ++effects.menu_confirm_events;
    }
    apply_runtime_input_branch_effects(effects, context);
}

void handle_paddle_tuning_branch(RuntimeInputPhaseContext& context) {
    RuntimeInputBranchEffects effects {};
    const bool changed =
        consume_paddle_tuning_component_selection(context) || consume_paddle_tuning_horizontal_adjustment(context);
    if (changed) {
        clamp_paddle_tuning_to_limits(
            context.paddle_tuning_state.working,
            context.paddle_tuning_state.max_skills,
            context.paddle_tuning_state.max_budget);
        ++effects.menu_move_events;
    }

    if (consume_confirm_press(context.window, context.edge_state)) {
        commit_paddle_tuning(context, effects);
    }

    apply_runtime_input_branch_effects(effects, context);
}

void handle_paused_branch(RuntimeInputPhaseContext& context) {
    const MatchExitPolicy exit_policy = compute_runtime_match_exit_policy(
        context.simulation,
        context.app_state,
        context.pause_return_state,
        context.match_flow,
        context.story_runtime,
        context.story_intro_state);
    const PauseInputFeedback pause_feedback = handle_runtime_pause_input(
        context.window,
        context.edge_state,
        context.controls,
        exit_policy,
        context.pause_menu_state,
        context.app_state,
        context.pause_return_state,
        context.story_runtime,
        context.story_hub_state,
        context.story_intro_state,
        context.story_scene_state,
        context.authored_transition_request,
        context.match_flow,
        context.simulation,
        context.story_official_games_to_win,
        sanitize_player_name,
        save_story_career);
    RuntimeInputBranchEffects effects {};
    if (pause_feedback.play_menu_move) {
        ++effects.menu_move_events;
    }
    if (pause_feedback.play_menu_confirm) {
        ++effects.menu_confirm_events;
    }
    apply_runtime_input_branch_effects(effects, context);
}

void handle_runtime_global_input(RuntimeInputPhaseContext& context, bool& show_dev_info) {
    handle_runtime_escape_hotkey(context);
    apply_story_scene_pending_gate(context);
    handle_runtime_menu_shortcut(context);
    handle_runtime_dev_shortcuts(context, show_dev_info);
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
