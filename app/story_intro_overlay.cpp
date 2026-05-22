#include "story_intro_overlay.hpp"

#include <algorithm>
#include <string>

#include "overlay_layout_math.hpp"
#include "pixel_font.hpp"
#include "story_chat_layout.hpp"
#include "story_chat_portrait_overlay.hpp"
#include "story_intro_text_layout.hpp"
#include "story_panel_layout.hpp"
#include "story_portraits.hpp"
#include "text_wrap.hpp"
#include "ui_text.hpp"

namespace whacker::app {

void render_story_intro_overlay(
    const RenderContext& context,
    const StoryRuntimeState& story_runtime,
    const StoryIntroState& story_intro_state,
    const ControlHintBindings& controls,
    const StoryIntroKeyNameFn key_name_fn,
    const StoryIntroSanitizeNameFn sanitize_name_fn) {
    const int fb_width = context.framebuffer_width;
    const int fb_height = context.framebuffer_height;
    if (fb_width <= 0 || fb_height <= 0) {
        return;
    }

    if (story_intro_state.phase == StoryIntroPhase::PlayMatch) {
        (void)story_runtime;
        return;
    }

    const StoryIntroDialogue dialogue =
        compose_story_intro_dialogue(story_intro_state, controls, key_name_fn, sanitize_name_fn);
    const std::size_t total_chars = story_intro_dialogue_char_count(dialogue);
    std::size_t chars_left = std::min(story_intro_state.visible_chars, total_chars);
    auto reveal_next = [&chars_left](const std::string& full) {
        const std::size_t count = std::min(chars_left, full.size());
        chars_left -= count;
        return full.substr(0, count);
    };

    StoryPanelLayoutSpec panel_spec = story_dialogue_panel_layout_spec();
    panel_spec.height_fraction = 0.30f;
    const StoryPanelLayout panel = make_story_panel_layout(fb_width, fb_height, panel_spec);
    draw_story_panel_background(fb_width, fb_height, panel, panel_spec, story_panel_palette());
    const StoryChatPortraitLayout chat_layout = make_story_chat_portrait_layout(panel, panel_spec);

    const StoryPortraitId rival_portrait = story_portrait_for_rival_id(story_intro_state.rival_id);
    const bool player_is_right = story_intro_state.player_is_right;
    const StoryPortraitId left_portrait = player_is_right ? rival_portrait : StoryPortraitId::Player;
    const StoryPortraitId right_portrait = player_is_right ? StoryPortraitId::Player : rival_portrait;
    StoryChatPortraitActiveLane active_lane = StoryChatPortraitActiveLane::None;
    if (story_intro_state.phase == StoryIntroPhase::NameEntry) {
        active_lane = player_is_right ? StoryChatPortraitActiveLane::Right : StoryChatPortraitActiveLane::Left;
    } else {
        active_lane = player_is_right ? StoryChatPortraitActiveLane::Left : StoryChatPortraitActiveLane::Right;
    }
    draw_story_chat_portrait_lanes(
        fb_width,
        fb_height,
        chat_layout,
        left_portrait,
        right_portrait,
        active_lane);

    constexpr float kChatInnerGuardPx = 8.0f;
    constexpr int kChatEarlyWrapChars = 2;
    constexpr float kFooterScale = 1.9f;
    const OverlayVerticalLayout vertical = make_overlay_vertical_layout(
        panel.panel_y,
        panel.panel_h,
        40.0f,
        text_line_height_pixels(kFooterScale),
        8.0f,
        10.0f,
        8.0f);

    const float base_text_x = chat_layout.text_w > 0.0f ? chat_layout.text_x : panel.text_x;
    const float base_text_w = chat_layout.text_w > 0.0f ? chat_layout.text_w : panel.text_w;
    const float text_x = base_text_x + kChatInnerGuardPx;
    const float text_w = inset_text_width(base_text_w, kChatInnerGuardPx);
    const auto fit_header_or_footer =
        [text_w](const std::string& text, const float scale, const int reserved_chars) -> std::string {
        return fit_text_to_single_line_ellipsis(
            text,
            std::max(4, max_chars_for_safe_text_width(text_w, scale, reserved_chars, kChatEarlyWrapChars, 0.0f)));
    };

    const std::string header = fit_header_or_footer(reveal_next(dialogue.header), 2.4f, 0);
    const StoryIntroBodyLayout body_layout = compute_story_intro_body_layout_for_framebuffer(
        fb_width,
        fb_height,
        story_intro_state,
        controls,
        key_name_fn,
        sanitize_name_fn);
    const int scroll_from_bottom = clamp_story_intro_scroll_from_bottom(
        body_layout,
        story_intro_state.scroll_lines_from_bottom);
    const int first_visible_row =
        first_visible_story_intro_row_index(body_layout, scroll_from_bottom);

    draw_text_pixels(fb_width, fb_height, text_x, vertical.header_y + 6.0f, 2.4f, header, Color {0.88f, 0.93f, 1.00f});

    float cursor_y = vertical.body_y;
    const float body_bottom = vertical.body_y + std::max(0.0f, vertical.body_h - 2.0f);
    bool drew_any_row = false;
    for (int row_index = first_visible_row; row_index < static_cast<int>(body_layout.rows.size()); ++row_index) {
        const StoryIntroBodyRow& row = body_layout.rows[static_cast<std::size_t>(row_index)];
        const float advance = std::max(0.0f, row.advance_px);
        if (advance <= 0.0f) {
            continue;
        }
        if (drew_any_row && (cursor_y + advance) > body_bottom) {
            break;
        }
        if (!row.text.empty() && row.scale > 0.0f) {
            draw_text_pixels(fb_width, fb_height, text_x, cursor_y, row.scale, row.text, row.color);
        }
        cursor_y += advance;
        drew_any_row = true;
    }

    const std::string footer = story_intro_state.dialogue_writing
        ? choose_best_fitting_variant(
            {ui_text::story_dialogue_footer_writing(), ui_text::story_dialogue_footer_writing_short()},
            std::max(4, max_chars_for_safe_text_width(text_w, kFooterScale, 0, kChatEarlyWrapChars, 0.0f)))
        : choose_best_fitting_variant(
            {ui_text::story_dialogue_footer_continue(), ui_text::story_dialogue_footer_continue_short()},
            std::max(4, max_chars_for_safe_text_width(text_w, kFooterScale, 0, kChatEarlyWrapChars, 0.0f)));
    draw_text_pixels(
        fb_width,
        fb_height,
        text_x,
        vertical.footer_y,
        kFooterScale,
        footer,
        Color {0.75f, 0.83f, 0.91f});

    (void)story_runtime;
}

}  // namespace whacker::app
