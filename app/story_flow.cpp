#include "story_flow.hpp"

#ifdef WHACKER_HAS_GLFW

#include <algorithm>

#include <GLFW/glfw3.h>

#include "story_continue_resume.hpp"
#include "story_play_session.hpp"
#include "story_intro_text_layout.hpp"
#include "story_script_catalog.hpp"
#include "story_text.hpp"
#include "text_utils.hpp"

namespace whacker::app {

namespace {

bool story_intro_safe_key_to_name_char(const KeyToNameCharFn key_to_name_char_fn, const int key, char& out_char) {
    return key_to_name_char_fn != nullptr ? key_to_name_char_fn(key, out_char) : false;
}

std::string story_intro_safe_trim_copy(const TrimCopyFn trim_copy_fn, const std::string& value) {
    return trim_copy_fn != nullptr ? trim_copy_fn(value) : trim_copy(value);
}

std::string story_intro_safe_sanitize_name(
    const StorySanitizeNameFn sanitize_name_fn,
    const std::string& value) {
    return sanitize_name_fn != nullptr ? sanitize_name_fn(value) : value;
}

struct StoryIntroInputPhaseContext {
    GLFWwindow* window;
    KeyEdgeState& edge_state;
    StoryRuntimeState& story_runtime;
    StoryHubState& story_hub_state;
    StoryIntroState& story_intro_state;
    MatchOptions& options;
    const ControlBindings& controls;
    MatchFlowState& match_flow;
    whacker::sim::Simulation& simulation;
    std::mt19937_64& rng;
    AppState& app_state;
    RuntimeAuthoredTransitionRequest& authored_transition_request;
    KeyToNameCharFn key_to_name_char_fn;
    TrimCopyFn trim_copy_fn;
    StorySanitizeNameFn sanitize_name_fn;
    StorySaveCareerCallback save_career_fn;
};

bool consume_story_intro_dialogue_confirm(StoryIntroInputPhaseContext& context) {
    if (!context.story_intro_state.dialogue_writing) {
        return false;
    }
    if (consume_confirm_press(context.window, context.edge_state)) {
        reveal_story_intro_typewriter(context.story_intro_state);
    }
    return true;
}

bool consume_story_intro_scroll_confirm(
    StoryIntroInputPhaseContext& context,
    const StoryIntroBodyLayout& body_layout) {
    if (context.story_intro_state.dialogue_writing ||
        body_layout.max_scroll_rows <= 0 ||
        context.story_intro_state.scroll_lines_from_bottom <= 0) {
        return false;
    }
    if (!consume_confirm_press(context.window, context.edge_state)) {
        return false;
    }
    context.story_intro_state.scroll_lines_from_bottom = 0;
    return true;
}

void consume_story_intro_scroll_input(
    StoryIntroInputPhaseContext& context,
    const StoryIntroBodyLayout& body_layout) {
    context.story_intro_state.scroll_lines_from_bottom = clamp_story_intro_scroll_from_bottom(
        body_layout,
        context.story_intro_state.scroll_lines_from_bottom);
    if (context.story_intro_state.dialogue_writing || body_layout.max_scroll_rows <= 0) {
        context.story_intro_state.scroll_lines_from_bottom = 0;
        return;
    }

    const bool scroll_up =
        consume_key_press(context.window, GLFW_KEY_UP, context.edge_state.up) ||
        consume_menu_up_press(context.window, context.edge_state, context.controls);
    const bool scroll_down =
        consume_key_press(context.window, GLFW_KEY_DOWN, context.edge_state.down) ||
        consume_menu_down_press(context.window, context.edge_state, context.controls);
    if (scroll_up == scroll_down) {
        return;
    }
    const int requested_scroll =
        context.story_intro_state.scroll_lines_from_bottom + (scroll_up ? 1 : -1);
    context.story_intro_state.scroll_lines_from_bottom = clamp_story_intro_scroll_from_bottom(
        body_layout,
        requested_scroll);
}

void handle_story_intro_invite_phase(
    StoryIntroInputPhaseContext& context,
    const StoryIntroBodyLayout& body_layout) {
    if (consume_story_intro_dialogue_confirm(context)) {
        return;
    }
    if (consume_story_intro_scroll_confirm(context, body_layout)) {
        return;
    }

    if (!consume_confirm_press(context.window, context.edge_state)) {
        return;
    }

    context.story_intro_state.player_is_right = false;
    context.story_intro_state.phase = StoryIntroPhase::PlayMatch;
    context.story_intro_state.break_kind = StoryIntroBreak::None;
    context.story_intro_state.swap_choice = 0;
    context.story_intro_state.phase_timer = 0.0f;
    context.story_intro_state.name_prompted = false;
    context.story_intro_state.name_accept_pending = false;
    context.story_intro_state.name_missing_prompt = false;
    context.story_intro_state.rules_hint_shown = false;
    context.story_intro_state.player_scored = false;
    context.story_intro_state.player_won = false;
    context.story_intro_state.player_forfeited = false;
    context.story_intro_state.points_played = 0;
    context.story_intro_state.final_left_score = 0;
    context.story_intro_state.final_right_score = 0;
    context.story_intro_state.player_usage = {};
    const StoryRivalSpec& intro_rival = story_script_intro_rival_spec();
    context.story_intro_state.rival_id = intro_rival.id;
    context.story_intro_state.rival_name = intro_rival.name;
    context.story_intro_state.rival_style = intro_rival.style;
    context.story_intro_state.rival_skills = intro_rival.skills;
    start_story_play_session(
        context.options,
        context.simulation,
        context.match_flow,
        context.rng,
        ActiveMatchMode::StoryTraining,
        false,
        intro_rival.style,
        intro_rival.skills,
        context.story_runtime.career.player_skills);
    clear_last_pressed_key();
}

void handle_story_intro_between_balls_phase(
    StoryIntroInputPhaseContext& context,
    const StoryIntroBodyLayout& body_layout) {
    if (context.story_intro_state.break_kind == StoryIntroBreak::SwapSides) {
        if (consume_key_press(context.window, GLFW_KEY_LEFT, context.edge_state.left) ||
            consume_menu_up_press(context.window, context.edge_state, context.controls)) {
            context.story_intro_state.swap_choice = 0;
        }
        if (consume_key_press(context.window, GLFW_KEY_RIGHT, context.edge_state.right) ||
            consume_menu_down_press(context.window, context.edge_state, context.controls)) {
            context.story_intro_state.swap_choice = 1;
        }
    }

    if (consume_story_intro_dialogue_confirm(context)) {
        return;
    }
    if (consume_story_intro_scroll_confirm(context, body_layout)) {
        return;
    }

    if (!consume_confirm_press(context.window, context.edge_state)) {
        return;
    }

    if (context.story_intro_state.break_kind == StoryIntroBreak::SwapSides) {
        const bool previous_player_is_right = context.story_intro_state.player_is_right;
        const bool next_player_is_right = context.story_intro_state.swap_choice == 1;
        context.story_intro_state.player_is_right = next_player_is_right;
        if (next_player_is_right != previous_player_is_right) {
            auto& state = context.simulation.mutable_state();
            std::swap(state.left_score, state.right_score);
        }
    }

    context.story_intro_state.break_kind = StoryIntroBreak::None;
    context.story_intro_state.phase = StoryIntroPhase::PlayMatch;
    context.story_intro_state.phase_timer = 0.0f;
}

void reset_story_intro_name_confirmation_state(StoryIntroInputPhaseContext& context) {
    if (context.story_intro_state.name_accept_pending || context.story_intro_state.name_missing_prompt) {
        context.story_intro_state.name_accept_pending = false;
        context.story_intro_state.name_missing_prompt = false;
        reset_story_intro_typewriter(context.story_intro_state);
    }
}

void handle_story_intro_name_entry_phase(
    StoryIntroInputPhaseContext& context,
    const StoryIntroBodyLayout& body_layout) {
    if (consume_story_intro_dialogue_confirm(context)) {
        return;
    }

    if (consume_key_press(context.window, GLFW_KEY_BACKSPACE, context.edge_state.backspace)) {
        if (!context.story_intro_state.entered_name.empty()) {
            context.story_intro_state.entered_name.pop_back();
        }
        reset_story_intro_name_confirmation_state(context);
    }

    const int key = consume_last_pressed_key();
    char name_char = '\0';
    if (story_intro_safe_key_to_name_char(context.key_to_name_char_fn, key, name_char) &&
        context.story_intro_state.entered_name.size() < 16u) {
        if (!(name_char == ' ' &&
              (context.story_intro_state.entered_name.empty() ||
               context.story_intro_state.entered_name.back() == ' '))) {
            context.story_intro_state.entered_name.push_back(name_char);
            reset_story_intro_name_confirmation_state(context);
        }
    }

    if (consume_story_intro_scroll_confirm(context, body_layout)) {
        return;
    }

    if (!consume_confirm_press(context.window, context.edge_state)) {
        return;
    }

    if (story_intro_safe_trim_copy(context.trim_copy_fn, context.story_intro_state.entered_name).empty()) {
        if (!context.story_intro_state.name_missing_prompt) {
            context.story_intro_state.name_missing_prompt = true;
            context.story_intro_state.name_accept_pending = false;
            reset_story_intro_typewriter(context.story_intro_state);
        }
        return;
    }

    if (!context.story_intro_state.name_accept_pending) {
        context.story_intro_state.name_accept_pending = true;
        context.story_intro_state.name_missing_prompt = false;
        reset_story_intro_typewriter(context.story_intro_state);
        return;
    }

    context.story_intro_state.entered_name =
        story_intro_safe_sanitize_name(context.sanitize_name_fn, context.story_intro_state.entered_name);
    context.story_intro_state.name_accept_pending = false;
    context.story_intro_state.name_missing_prompt = false;
    context.story_intro_state.phase = StoryIntroPhase::PlayMatch;
    context.story_intro_state.phase_timer = 0.0f;
    context.story_intro_state.dialogue_writing = false;
}

void handle_story_intro_rival_intro_phase(
    StoryIntroInputPhaseContext& context,
    const StoryIntroBodyLayout& body_layout) {
    if (consume_story_intro_dialogue_confirm(context)) {
        return;
    }
    if (consume_story_intro_scroll_confirm(context, body_layout)) {
        return;
    }

    if (!consume_confirm_press(context.window, context.edge_state)) {
        return;
    }

    complete_story_intro(
        context.story_runtime,
        context.story_hub_state,
        context.story_intro_state,
        context.match_flow,
        context.simulation,
        context.app_state,
        context.authored_transition_request,
        context.sanitize_name_fn,
        context.save_career_fn);
}

}  // namespace

void handle_story_intro_input(
    GLFWwindow* window,
    KeyEdgeState& edge_state,
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    StoryIntroState& story_intro_state,
    MatchOptions& options,
    const ControlBindings& controls,
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    std::mt19937_64& rng,
    AppState& app_state,
    RuntimeAuthoredTransitionRequest& authored_transition_request,
    const KeyToNameCharFn key_to_name_char_fn,
    const TrimCopyFn trim_copy_fn,
    const StorySanitizeNameFn sanitize_name_fn,
    const StorySaveCareerCallback save_career_fn) {
    StoryIntroInputPhaseContext context {
        .window = window,
        .edge_state = edge_state,
        .story_runtime = story_runtime,
        .story_hub_state = story_hub_state,
        .story_intro_state = story_intro_state,
        .options = options,
        .controls = controls,
        .match_flow = match_flow,
        .simulation = simulation,
        .rng = rng,
        .app_state = app_state,
        .authored_transition_request = authored_transition_request,
        .key_to_name_char_fn = key_to_name_char_fn,
        .trim_copy_fn = trim_copy_fn,
        .sanitize_name_fn = sanitize_name_fn,
        .save_career_fn = save_career_fn,
    };

    const StoryIntroBodyLayout body_layout = compute_story_intro_body_layout_for_window(
        context.window,
        context.story_intro_state,
        context.controls,
        nullptr,
        context.sanitize_name_fn);
    consume_story_intro_scroll_input(context, body_layout);

    if (context.story_intro_state.phase == StoryIntroPhase::Invite) {
        handle_story_intro_invite_phase(context, body_layout);
        return;
    }

    if (context.story_intro_state.phase == StoryIntroPhase::BetweenBalls) {
        handle_story_intro_between_balls_phase(context, body_layout);
        return;
    }

    if (context.story_intro_state.phase == StoryIntroPhase::NameEntry) {
        handle_story_intro_name_entry_phase(context, body_layout);
        return;
    }

    if (context.story_intro_state.phase == StoryIntroPhase::RivalIntro) {
        handle_story_intro_rival_intro_phase(context, body_layout);
    }
}

void handle_story_menu_input(
    GLFWwindow* window,
    KeyEdgeState& edge_state,
    StoryMenuState& story_menu_state,
    StoryRuntimeState& story_runtime,
    StoryHubState& story_hub_state,
    StoryIntroState& story_intro_state,
    MatchOptions& options,
    const ControlBindings& controls,
    MatchFlowState& match_flow,
    whacker::sim::Simulation& simulation,
    AppState& app_state,
    const bool has_save,
    const StoryLoadCareerFn load_career_fn,
    const StoryResetCareerFn reset_career_fn) {
    if (story_menu_state.confirm_overwrite) {
        if (consume_key_press(window, GLFW_KEY_LEFT, edge_state.left) ||
            consume_key_press(window, GLFW_KEY_RIGHT, edge_state.right) ||
            consume_menu_up_press(window, edge_state, controls) ||
            consume_menu_down_press(window, edge_state, controls)) {
            story_menu_state.confirm_selected = 1 - story_menu_state.confirm_selected;
        }
        if (consume_confirm_press(window, edge_state)) {
            if (story_menu_state.confirm_selected == 1) {
                story_menu_state.confirm_overwrite = false;
                story_menu_state.confirm_selected = 0;
                begin_new_story_intro(
                    story_runtime,
                    story_hub_state,
                    story_intro_state,
                    options,
                    match_flow,
                    simulation,
                    app_state,
                    reset_career_fn);
                return;
            }
            story_menu_state.confirm_overwrite = false;
            story_menu_state.confirm_selected = 0;
        }
        return;
    }

    if (consume_menu_up_press(window, edge_state, controls)) {
        story_menu_state.selected_row = (story_menu_state.selected_row + StoryMenuRowCount - 1) % StoryMenuRowCount;
    }
    if (consume_menu_down_press(window, edge_state, controls)) {
        story_menu_state.selected_row = (story_menu_state.selected_row + 1) % StoryMenuRowCount;
    }
    if (consume_confirm_press(window, edge_state)) {
        if (story_menu_state.selected_row == StoryMenuRowBack) {
            app_state = AppState::MainMenu;
            return;
        }
        if (story_menu_state.selected_row == StoryMenuRowContinue && !has_save) {
            return;
        }

        if (story_menu_state.selected_row == StoryMenuRowNewCareer) {
            if (has_save) {
                story_menu_state.confirm_overwrite = true;
                story_menu_state.confirm_selected = 0;
                return;
            }
            begin_new_story_intro(
                story_runtime,
                story_hub_state,
                story_intro_state,
                options,
                match_flow,
                simulation,
                app_state,
                reset_career_fn);
            return;
        }

        if (story_menu_state.selected_row == StoryMenuRowContinue) {
            StoryCareerData loaded {};
            std::string load_error;
            if (load_career_fn == nullptr || !load_career_fn(loaded, &load_error)) {
                story_hub_state.feedback_line_1 = story_text::continue_failed_feedback_line_1();
                story_hub_state.feedback_line_2 = load_error;
                return;
            }
            app_state = apply_continue_loaded_career(story_runtime, loaded);
            story_hub_state.selected_row = StoryHubRowOfficialMatch;
            story_hub_state.feedback_line_1 = story_text::career_loaded_feedback_line_1();
            story_hub_state.feedback_line_2 = story_text::career_loaded_feedback_line_2(story_runtime.career.week);
            return;
        }
    }
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
