#include "story_intro.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

#include "story_text.hpp"

namespace whacker::app {

void reset_story_intro_typewriter(StoryIntroState& story_intro_state) {
    story_intro_state.visible_chars = 0;
    story_intro_state.type_accum = 0.0f;
    story_intro_state.scroll_lines_from_bottom = 0;
    story_intro_state.typed_phase = story_intro_state.phase;
    story_intro_state.typed_break = story_intro_state.break_kind;
    story_intro_state.dialogue_writing = true;
}

void reveal_story_intro_typewriter(StoryIntroState& story_intro_state) {
    story_intro_state.visible_chars = std::numeric_limits<std::size_t>::max();
    story_intro_state.type_accum = 0.0f;
    story_intro_state.scroll_lines_from_bottom = 0;
    story_intro_state.dialogue_writing = false;
}

StoryIntroDialogue compose_story_intro_dialogue(
    const StoryIntroState& story_intro_state,
    const ControlBindings& controls,
    const StoryIntroKeyNameFn key_name_fn,
    const StoryIntroSanitizeNameFn sanitize_name_fn) {
    const auto safe_key_name = [key_name_fn](const int key) -> const char* {
        return key_name_fn != nullptr ? key_name_fn(key) : "?";
    };
    const auto safe_sanitize_name = [sanitize_name_fn](const std::string& raw) -> std::string {
        return sanitize_name_fn != nullptr ? sanitize_name_fn(raw) : raw;
    };

    StoryIntroDialogue dialogue {};
    dialogue.header = story_text::intro_header_line(story_intro_state.rival_name);

    if (story_intro_state.phase == StoryIntroPhase::Invite) {
        dialogue.line_1 = story_text::intro_invite_line_1();
        dialogue.line_2 = story_text::intro_invite_line_2();
        dialogue.line_3 = story_text::intro_invite_line_3();
    } else if (story_intro_state.phase == StoryIntroPhase::BetweenBalls) {
        if (story_intro_state.break_kind == StoryIntroBreak::SwapSides) {
            dialogue.line_1 = story_text::intro_swap_sides_line_1();
            dialogue.line_2 = story_text::intro_swap_sides_line_2();
            dialogue.options = {story_text::intro_swap_option_stay_left(), story_text::intro_swap_option_swap_right()};
            dialogue.option_count = 2;
        } else if (story_intro_state.break_kind == StoryIntroBreak::Controls) {
            const int up_key = story_intro_state.player_is_right ? controls.p2_up : controls.p1_up;
            const int down_key = story_intro_state.player_is_right ? controls.p2_down : controls.p1_down;
            dialogue.line_1 = story_text::intro_controls_line_1();
            dialogue.line_2 = story_text::intro_controls_line_2(safe_key_name(up_key), safe_key_name(down_key));
            dialogue.line_3 = story_text::intro_controls_line_3();
        } else if (story_intro_state.break_kind == StoryIntroBreak::Rules) {
            dialogue.line_1 = story_text::intro_rules_line_1();
            dialogue.line_2 = story_text::intro_rules_line_2();
            dialogue.line_3 = story_text::intro_rules_line_3();
        } else {
            dialogue.line_1 = story_text::intro_ready_next_ball_line_1();
            dialogue.line_2 = story_text::intro_ready_next_ball_line_2();
        }
    } else if (story_intro_state.phase == StoryIntroPhase::NameEntry) {
        if (story_intro_state.name_accept_pending) {
            dialogue.line_1 = story_text::intro_name_confirm_line_1();
            dialogue.line_2 = story_text::intro_name_confirm_line_2();
            dialogue.line_3 = safe_sanitize_name(story_intro_state.entered_name);
        } else {
            dialogue.line_1 = story_text::intro_name_prompt_line_1();
            dialogue.line_2 = story_text::intro_name_prompt_line_2(story_intro_state.name_missing_prompt);
            dialogue.line_3 = story_intro_state.entered_name.empty()
                ? story_text::intro_name_placeholder_line_3()
                : story_intro_state.entered_name;
        }
    } else if (story_intro_state.phase == StoryIntroPhase::RivalIntro) {
        dialogue.line_1 = story_text::intro_rival_intro_line_1(
            safe_sanitize_name(story_intro_state.entered_name),
            story_intro_state.rival_name);
        dialogue.line_2 = story_text::intro_rival_intro_line_2(
            story_intro_state.final_left_score,
            story_intro_state.final_right_score,
            story_text::intro_performance_line(story_intro_state),
            story_intro_state.player_forfeited);
        dialogue.line_3 = story_text::intro_rival_intro_line_3(
            story_text::intro_style_line(story_intro_state));
    }

    return dialogue;
}

std::size_t story_intro_dialogue_char_count(const StoryIntroDialogue& dialogue) {
    std::size_t total = 0;
    total += dialogue.header.size();
    total += dialogue.line_1.size();
    total += dialogue.line_2.size();
    total += dialogue.line_3.size();
    for (int i = 0; i < dialogue.option_count; ++i) {
        total += dialogue.options[static_cast<std::size_t>(i)].size();
    }
    return total;
}

void update_story_intro_typewriter(
    StoryIntroState& story_intro_state,
    const ControlBindings& controls,
    const float dt,
    const float speed_multiplier,
    const StoryIntroKeyNameFn key_name_fn,
    const StoryIntroSanitizeNameFn sanitize_name_fn) {
    if (story_intro_state.phase == StoryIntroPhase::PlayMatch) {
        story_intro_state.dialogue_writing = false;
        story_intro_state.scroll_lines_from_bottom = 0;
        return;
    }

    if (story_intro_state.typed_phase != story_intro_state.phase ||
        story_intro_state.typed_break != story_intro_state.break_kind) {
        reset_story_intro_typewriter(story_intro_state);
    }

    const StoryIntroDialogue dialogue =
        compose_story_intro_dialogue(story_intro_state, controls, key_name_fn, sanitize_name_fn);
    std::size_t total_chars = story_intro_dialogue_char_count(dialogue);
    if (story_intro_state.phase == StoryIntroPhase::NameEntry) {
        total_chars = total_chars >= dialogue.line_3.size() ? (total_chars - dialogue.line_3.size()) : 0;
    }
    if (total_chars == 0) {
        story_intro_state.dialogue_writing = false;
        story_intro_state.scroll_lines_from_bottom = 0;
        return;
    }

    constexpr float kStoryIntroCharsPerSecond = 36.0f;
    const float capped_multiplier = std::clamp(speed_multiplier, 1.0f, 12.0f);
    if (story_intro_state.visible_chars >= total_chars) {
        story_intro_state.visible_chars = total_chars;
        story_intro_state.dialogue_writing = false;
        return;
    }
    story_intro_state.type_accum += std::max(0.0f, dt) * kStoryIntroCharsPerSecond * capped_multiplier;
    const auto add_chars = static_cast<std::size_t>(story_intro_state.type_accum);
    if (add_chars > 0) {
        story_intro_state.visible_chars = std::min(
            total_chars,
            story_intro_state.visible_chars + add_chars);
        story_intro_state.type_accum -= static_cast<float>(add_chars);
    }
    story_intro_state.dialogue_writing = story_intro_state.visible_chars < total_chars;
    if (story_intro_state.dialogue_writing) {
        story_intro_state.scroll_lines_from_bottom = 0;
    }
}

}  // namespace whacker::app
