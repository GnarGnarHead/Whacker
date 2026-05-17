#include "story_scene_overlay.hpp"

#ifdef WHACKER_HAS_GLFW

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include <GLFW/glfw3.h>

#include "overlay_layout_math.hpp"
#include "pixel_font.hpp"
#include "story_chat_layout.hpp"
#include "story_chat_portrait_overlay.hpp"
#include "story_panel_layout.hpp"
#include "story_scene.hpp"
#include "story_scene_text_layout.hpp"
#include "text_wrap.hpp"
#include "ui_text.hpp"

namespace whacker::app {

void render_story_scene_overlay(
    GLFWwindow* window,
    const StorySceneState& scene_state) {
    int fb_width = 0;
    int fb_height = 0;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    if (fb_width <= 0 || fb_height <= 0) {
        return;
    }
    if (!story_scene_has_content(scene_state)) {
        return;
    }

    const StoryPanelLayoutSpec panel_spec = story_dialogue_panel_layout_spec();
    const StoryPanelLayout panel = make_story_panel_layout(fb_width, fb_height, panel_spec);
    draw_story_panel_background(fb_width, fb_height, panel, panel_spec, story_panel_palette());
    const StoryChatPortraitLayout chat_layout = make_story_chat_portrait_layout(panel, panel_spec);

    constexpr float kChatInnerGuardPx = 8.0f;
    constexpr int kChatEarlyWrapChars = 2;
    constexpr float kHeaderScale = 2.4f;
    constexpr float kBodyScale = 2.0f;
    constexpr float kFooterScale = 1.9f;
    const OverlayVerticalLayout vertical = make_overlay_vertical_layout(
        panel.panel_y,
        panel.panel_h,
        40.0f,
        text_line_height_pixels(kFooterScale),
        8.0f,
        10.0f,
        8.0f);

    const StoryPortraitId speaker_portrait = story_scene_current_portrait(scene_state);
    const bool player_is_right = scene_state.player_is_right;
    const StoryPortraitId left_portrait = player_is_right ? speaker_portrait : StoryPortraitId::Player;
    const StoryPortraitId right_portrait = player_is_right ? StoryPortraitId::Player : speaker_portrait;
    StoryChatPortraitActiveLane active_lane = StoryChatPortraitActiveLane::None;
    const StorySceneSpeaker speaker = story_scene_current_speaker(scene_state);
    if (speaker == StorySceneSpeaker::Player) {
        active_lane = player_is_right ? StoryChatPortraitActiveLane::Right : StoryChatPortraitActiveLane::Left;
    } else if (speaker == StorySceneSpeaker::Rival) {
        active_lane = player_is_right ? StoryChatPortraitActiveLane::Left : StoryChatPortraitActiveLane::Right;
    }
    draw_story_chat_portrait_lanes(
        fb_width,
        fb_height,
        chat_layout,
        left_portrait,
        right_portrait,
        active_lane);

    const float base_text_x = chat_layout.text_w > 0.0f ? chat_layout.text_x : panel.text_x;
    const float base_text_w = chat_layout.text_w > 0.0f ? chat_layout.text_w : panel.text_w;
    const float text_x = base_text_x + kChatInnerGuardPx;
    const float text_w = inset_text_width(base_text_w, kChatInnerGuardPx);
    draw_text_pixels(
        fb_width,
        fb_height,
        text_x,
        vertical.header_y + 6.0f,
        kHeaderScale,
        fit_text_to_single_line_ellipsis(
            scene_state.header,
            std::max(
                4,
                max_chars_for_safe_text_width(
                    base_text_w,
                    kHeaderScale,
                    0,
                    kChatEarlyWrapChars,
                    kChatInnerGuardPx))),
        Color {0.88f, 0.93f, 1.0f});

    const float body_x = text_x;
    const float body_y = vertical.body_y;
    const float line_step = text_line_height_pixels(kBodyScale) + 2.0f;
    const StorySceneBodyLayout body_layout =
        compute_story_scene_body_layout_for_framebuffer(fb_width, fb_height, scene_state);
    const int scroll_from_bottom =
        clamp_story_scene_scroll_from_bottom(body_layout, scene_state.scroll_lines_from_bottom);
    const int first_visible_line = first_visible_story_scene_line_index(body_layout, scroll_from_bottom);
    const int total_lines = static_cast<int>(body_layout.wrapped_lines.size());
    const int visible_lines = std::max(
        0,
        std::min(body_layout.visible_line_capacity, total_lines - first_visible_line));
    for (int i = 0; i < visible_lines; ++i) {
        draw_text_pixels(
            fb_width,
            fb_height,
            body_x,
            body_y + static_cast<float>(i) * line_step,
            kBodyScale,
            body_layout.wrapped_lines[static_cast<std::size_t>(first_visible_line + i)],
            Color {0.94f, 0.96f, 0.99f});
    }

    const bool show_binary_choice = !scene_state.dialogue_writing && scene_state.has_binary_choice;
    if (show_binary_choice) {
        const float button_h = text_line_height_pixels(1.7f) + 6.0f;
        const float button_w = std::min(110.0f, std::max(82.0f, 0.25f * text_w));
        const float button_gap = 12.0f;
        const float choice_y = std::max(
            vertical.body_y + 4.0f,
            vertical.footer_y - button_h - 6.0f);
        const float choice_block_w = (2.0f * button_w) + button_gap;
        const float choice_start_x = text_x + std::max(0.0f, 0.5f * (text_w - choice_block_w));
        const float no_x = choice_start_x;
        const float yes_x = no_x + button_w + button_gap;
        const bool yes_selected = scene_state.binary_choice_yes_selected;
        const bool no_selected = !yes_selected;

        draw_rect_pixels(
            fb_width,
            fb_height,
            no_x,
            choice_y,
            button_w,
            button_h,
            no_selected ? 0.24f : 0.12f,
            no_selected ? 0.30f : 0.17f,
            no_selected ? 0.36f : 0.22f);
        draw_text_centered(
            fb_width,
            fb_height,
            no_x,
            choice_y,
            button_w,
            button_h,
            1.7f,
            ui_text::selected_option_label(ui_text::no_label(), no_selected),
            Color {0.94f, 0.96f, 0.99f});

        draw_rect_pixels(
            fb_width,
            fb_height,
            yes_x,
            choice_y,
            button_w,
            button_h,
            yes_selected ? 0.28f : 0.12f,
            yes_selected ? 0.34f : 0.17f,
            yes_selected ? 0.18f : 0.22f);
        draw_text_centered(
            fb_width,
            fb_height,
            yes_x,
            choice_y,
            button_w,
            button_h,
            1.7f,
            ui_text::selected_option_label(ui_text::yes_label(), yes_selected),
            Color {0.97f, 0.95f, 0.90f});
    }

    const std::string footer = scene_state.dialogue_writing
        ? choose_best_fitting_variant(
            {ui_text::story_dialogue_footer_writing(), ui_text::story_dialogue_footer_writing_short()},
            std::max(
                4,
                max_chars_for_safe_text_width(
                    base_text_w,
                    kFooterScale,
                    0,
                    kChatEarlyWrapChars,
                    kChatInnerGuardPx)))
        : (show_binary_choice
            ? choose_best_fitting_variant(
                {ui_text::story_dialogue_footer_choice(), ui_text::story_dialogue_footer_choice_short()},
                std::max(
                    4,
                    max_chars_for_safe_text_width(
                        base_text_w,
                        kFooterScale,
                        0,
                        kChatEarlyWrapChars,
                        kChatInnerGuardPx)))
            : choose_best_fitting_variant(
                {ui_text::story_dialogue_footer_continue(), ui_text::story_dialogue_footer_continue_short()},
                std::max(
                    4,
                    max_chars_for_safe_text_width(
                        base_text_w,
                    kFooterScale,
                    0,
                    kChatEarlyWrapChars,
                    kChatInnerGuardPx))));
    draw_text_pixels(
        fb_width,
        fb_height,
        text_x,
        vertical.footer_y,
        kFooterScale,
        footer,
        Color {0.75f, 0.83f, 0.91f});
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
