#include "story_intro_text_layout.hpp"

#ifdef WHACKER_HAS_GLFW

#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

#include <GLFW/glfw3.h>

#include "story_chat_layout.hpp"
#include "story_intro.hpp"
#include "story_panel_layout.hpp"
#include "story_text.hpp"
#include "text_wrap.hpp"

namespace whacker::app {

namespace {

constexpr int kFallbackFramebufferWidth = 960;
constexpr int kFallbackFramebufferHeight = 540;

constexpr float kChatInnerGuardPx = 8.0f;
constexpr int kChatEarlyWrapChars = 2;
constexpr float kFooterScale = 1.9f;

float text_line_height_pixels_local(const float scale) {
    return 5.0f * scale;
}

StoryPanelLayout make_story_panel_layout_local(
    const int fb_width,
    const int fb_height,
    const StoryPanelLayoutSpec& spec) {
    const float width = static_cast<float>(std::max(0, fb_width));
    const float height = static_cast<float>(std::max(0, fb_height));

    StoryPanelLayout layout {};
    layout.panel_x = width * spec.x_fraction;
    layout.panel_y = height * spec.y_fraction;
    layout.panel_w = width * spec.width_fraction;
    layout.panel_h = height * spec.height_fraction;
    layout.text_x = layout.panel_x + spec.text_padding_x_px;
    layout.text_w = std::max(0.0f, layout.panel_w - (2.0f * spec.text_padding_x_px));
    layout.footer_y = layout.panel_y + layout.panel_h - spec.footer_padding_bottom_px;
    return layout;
}

float inset_text_width_local(const float width_px, const float inner_guard_px) {
    const float safe_width = std::max(0.0f, width_px);
    const float safe_guard = std::max(0.0f, inner_guard_px);
    return std::max(0.0f, safe_width - (2.0f * safe_guard));
}

int max_chars_for_text_width_local(const float width_px, const float scale, const int reserved_chars = 0) {
    if (width_px <= 0.0f || scale <= 0.0f) {
        return 0;
    }
    const int max_chars = static_cast<int>(std::floor(width_px / (4.0f * scale)));
    return std::max(0, max_chars - std::max(0, reserved_chars));
}

int max_chars_for_safe_text_width_local(
    const float width_px,
    const float scale,
    const int reserved_chars,
    const int early_wrap_chars,
    const float inner_guard_px) {
    const int base_chars = max_chars_for_text_width_local(
        inset_text_width_local(width_px, inner_guard_px),
        scale,
        reserved_chars);
    return std::max(0, base_chars - std::max(0, early_wrap_chars));
}

std::pair<int, int> resolve_intro_layout_framebuffer_size(GLFWwindow* window) {
    int fb_width = 0;
    int fb_height = 0;
#if !defined(WHACKER_TEST_ASSERTIONS_ACTIVE)
    if (window != nullptr) {
        glfwGetFramebufferSize(window, &fb_width, &fb_height);
    }
#else
    (void)window;
#endif
    if (fb_width <= 0 || fb_height <= 0) {
        fb_width = kFallbackFramebufferWidth;
        fb_height = kFallbackFramebufferHeight;
    }
    return {fb_width, fb_height};
}

StoryIntroDialogue compose_intro_dialogue_local(
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

std::size_t intro_dialogue_char_count_local(const StoryIntroDialogue& dialogue) {
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

void append_wrapped_block(
    std::vector<StoryIntroBodyRow>& rows,
    const std::vector<std::string>& lines,
    const float scale,
    const Color color,
    const float paragraph_gap) {
    bool drew_line = false;
    for (const std::string& line : lines) {
        if (line.empty()) {
            continue;
        }
        rows.push_back(StoryIntroBodyRow {
            .text = line,
            .scale = scale,
            .color = color,
            .advance_px = text_line_height_pixels_local(scale) + 2.0f,
        });
        drew_line = true;
    }
    if (drew_line && paragraph_gap > 0.0f) {
        rows.push_back(StoryIntroBodyRow {
            .text = {},
            .scale = 0.0f,
            .color = Color {},
            .advance_px = paragraph_gap,
        });
    }
}

void finalize_layout_scroll_bounds(StoryIntroBodyLayout& layout) {
    layout.latest_start_row = 0;
    layout.max_scroll_rows = 0;
    if (layout.rows.empty()) {
        return;
    }
    const int row_count = static_cast<int>(layout.rows.size());
    const float budget = std::max(0.0f, layout.body_budget_px);
    if (budget <= 0.0f) {
        layout.latest_start_row = row_count - 1;
        layout.max_scroll_rows = layout.latest_start_row;
        return;
    }

    int start_row = row_count - 1;
    float used = 0.0f;
    bool has_visible_row = false;
    for (int row = row_count - 1; row >= 0; --row) {
        const float advance = std::max(0.0f, layout.rows[static_cast<std::size_t>(row)].advance_px);
        if (advance <= 0.0f) {
            continue;
        }
        if (!has_visible_row) {
            has_visible_row = true;
            used = advance;
            start_row = row;
            continue;
        }
        if ((used + advance) > budget) {
            break;
        }
        used += advance;
        start_row = row;
    }
    layout.latest_start_row = has_visible_row ? std::max(0, start_row) : 0;
    layout.max_scroll_rows = layout.latest_start_row;
}

}  // namespace

StoryIntroBodyLayout compute_story_intro_body_layout_for_framebuffer(
    int fb_width,
    int fb_height,
    const StoryIntroState& story_intro_state,
    const ControlBindings& controls,
    const StoryIntroKeyNameFn key_name_fn,
    const StoryIntroSanitizeNameFn sanitize_name_fn) {
    if (fb_width <= 0 || fb_height <= 0) {
        fb_width = kFallbackFramebufferWidth;
        fb_height = kFallbackFramebufferHeight;
    }

    StoryIntroBodyLayout layout {};
    if (story_intro_state.phase == StoryIntroPhase::PlayMatch) {
        return layout;
    }

    StoryPanelLayoutSpec panel_spec {};
    panel_spec.height_fraction = 0.30f;
    const StoryPanelLayout panel = make_story_panel_layout_local(fb_width, fb_height, panel_spec);
    const StoryChatPortraitLayout chat_layout = make_story_chat_portrait_layout(panel, panel_spec);
    const float header_h = 40.0f;
    const float footer_h = text_line_height_pixels_local(kFooterScale);
    const float top_padding = 8.0f;
    const float bottom_padding = 10.0f;
    const float section_gap = 8.0f;
    const float body_y = panel.panel_y + top_padding + header_h + section_gap;
    const float footer_y = std::max(
        body_y,
        panel.panel_y + panel.panel_h - bottom_padding - footer_h);
    const float body_h = std::max(0.0f, footer_y - body_y);
    const float body_bottom = body_y + std::max(0.0f, body_h - 2.0f);
    layout.body_budget_px = std::max(0.0f, body_bottom - body_y);

    const StoryIntroDialogue dialogue =
        compose_intro_dialogue_local(story_intro_state, controls, key_name_fn, sanitize_name_fn);
    const std::size_t total_chars = intro_dialogue_char_count_local(dialogue);
    std::size_t chars_left = std::min(story_intro_state.visible_chars, total_chars);
    auto reveal_next = [&chars_left](const std::string& full) {
        const std::size_t count = std::min(chars_left, full.size());
        chars_left -= count;
        return full.substr(0, count);
    };
    static_cast<void>(reveal_next(dialogue.header));

    const float base_text_w = chat_layout.text_w > 0.0f ? chat_layout.text_w : panel.text_w;
    const float text_w = inset_text_width_local(base_text_w, kChatInnerGuardPx);
    const auto wrap_dialogue_line =
        [text_w](const std::string& text, const float scale, const int max_lines) -> std::vector<std::string> {
        const int max_chars = std::max(
            4,
            max_chars_for_safe_text_width_local(text_w, scale, 0, kChatEarlyWrapChars, 0.0f));
        return wrap_text_to_char_lines(text, max_chars, std::max(1, max_lines));
    };
    const auto fit_option_line = [text_w](const std::string& text) -> std::string {
        const int max_chars = std::max(
            4,
            max_chars_for_safe_text_width_local(text_w, 2.0f, 2, kChatEarlyWrapChars, 0.0f));
        return fit_text_to_single_line_ellipsis(text, max_chars);
    };

    const std::vector<std::string> line_1 = wrap_dialogue_line(reveal_next(dialogue.line_1), 2.2f, 4);
    const std::vector<std::string> line_2 = wrap_dialogue_line(reveal_next(dialogue.line_2), 2.1f, 4);
    const std::vector<std::string> line_3 = wrap_dialogue_line(
        story_intro_state.phase == StoryIntroPhase::NameEntry ? dialogue.line_3 : reveal_next(dialogue.line_3),
        2.2f,
        4);
    std::array<std::string, 2> option_lines {};
    for (int i = 0; i < dialogue.option_count; ++i) {
        option_lines[static_cast<std::size_t>(i)] =
            fit_option_line(reveal_next(dialogue.options[static_cast<std::size_t>(i)]));
    }

    append_wrapped_block(layout.rows, line_1, 2.2f, Color {0.90f, 0.94f, 1.00f}, 2.0f);
    append_wrapped_block(layout.rows, line_2, 2.1f, Color {0.72f, 0.80f, 0.90f}, 2.0f);
    append_wrapped_block(layout.rows, line_3, 2.2f, Color {0.95f, 0.88f, 0.46f}, 2.0f);
    for (int i = 0; i < dialogue.option_count; ++i) {
        const bool selected = i == story_intro_state.swap_choice;
        const std::string option_text = option_lines[static_cast<std::size_t>(i)].empty()
            ? std::string {}
            : (selected ? std::string {"> "} : std::string {"  "}) + option_lines[static_cast<std::size_t>(i)];
        if (option_text.empty()) {
            continue;
        }
        layout.rows.push_back(StoryIntroBodyRow {
            .text = option_text,
            .scale = 2.0f,
            .color = selected ? Color {0.96f, 0.86f, 0.34f} : Color {0.86f, 0.91f, 0.97f},
            .advance_px = text_line_height_pixels_local(2.0f) + 2.0f,
        });
    }
    while (!layout.rows.empty() && layout.rows.back().text.empty()) {
        layout.rows.pop_back();
    }

    finalize_layout_scroll_bounds(layout);
    return layout;
}

StoryIntroBodyLayout compute_story_intro_body_layout_for_window(
    GLFWwindow* window,
    const StoryIntroState& story_intro_state,
    const ControlBindings& controls,
    const StoryIntroKeyNameFn key_name_fn,
    const StoryIntroSanitizeNameFn sanitize_name_fn) {
    const std::pair<int, int> fb = resolve_intro_layout_framebuffer_size(window);
    return compute_story_intro_body_layout_for_framebuffer(
        fb.first,
        fb.second,
        story_intro_state,
        controls,
        key_name_fn,
        sanitize_name_fn);
}

int clamp_story_intro_scroll_from_bottom(
    const StoryIntroBodyLayout& layout,
    const int requested_scroll_from_bottom) {
    return std::clamp(requested_scroll_from_bottom, 0, std::max(0, layout.max_scroll_rows));
}

int first_visible_story_intro_row_index(
    const StoryIntroBodyLayout& layout,
    const int scroll_from_bottom) {
    const int safe_scroll = clamp_story_intro_scroll_from_bottom(layout, scroll_from_bottom);
    return std::max(0, layout.latest_start_row - safe_scroll);
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
