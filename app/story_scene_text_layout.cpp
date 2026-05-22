#include "story_scene_text_layout.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

#include "overlay_layout_math.hpp"
#include "story_chat_layout.hpp"
#include "story_panel_layout.hpp"
#include "story_scene.hpp"
#include "text_wrap.hpp"

namespace whacker::app {

namespace {

constexpr int kFallbackFramebufferWidth = 960;
constexpr int kFallbackFramebufferHeight = 540;

constexpr float kChatInnerGuardPx = 8.0f;
constexpr int kChatEarlyWrapChars = 2;
constexpr float kBodyScale = 2.0f;
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

bool story_scene_line_available(const StorySceneState& scene_state) {
    return scene_state.id != StorySceneId::None && scene_state.line_count > 0;
}

std::string story_scene_current_line_visible_no_link(const StorySceneState& scene_state) {
    if (!story_scene_line_available(scene_state)) {
        return {};
    }
    const int clamped_index = std::clamp(scene_state.line_index, 0, scene_state.line_count - 1);
    const std::string& full = scene_state.lines[static_cast<std::size_t>(clamped_index)];
    if (scene_state.visible_chars >= full.size()) {
        return full;
    }
    return full.substr(0, scene_state.visible_chars);
}

}  // namespace

StorySceneBodyLayout compute_story_scene_body_layout_for_framebuffer(
    int fb_width,
    int fb_height,
    const StorySceneState& scene_state) {
    if (fb_width <= 0 || fb_height <= 0) {
        fb_width = kFallbackFramebufferWidth;
        fb_height = kFallbackFramebufferHeight;
    }

    const StoryPanelLayoutSpec panel_spec {};
    const StoryPanelLayout panel = make_story_panel_layout_local(fb_width, fb_height, panel_spec);
    const StoryChatPortraitLayout chat_layout = make_story_chat_portrait_layout(panel, panel_spec);
    const OverlayVerticalLayout vertical = make_overlay_vertical_layout(
        panel.panel_y,
        panel.panel_h,
        40.0f,
        text_line_height_pixels_local(kFooterScale),
        8.0f,
        10.0f,
        8.0f);

    const float base_text_w = chat_layout.text_w > 0.0f ? chat_layout.text_w : panel.text_w;
    const float text_w = inset_text_width(base_text_w, kChatInnerGuardPx);
    const float line_step = text_line_height_pixels_local(kBodyScale) + 2.0f;
    const float body_y = vertical.body_y;
    const float body_bottom = vertical.body_y + std::max(0.0f, vertical.body_h - 2.0f);
    const float body_budget = std::max(0.0f, body_bottom - body_y);
    const int visible_line_capacity =
        std::max(1, static_cast<int>(std::floor(body_budget / std::max(1.0f, line_step))));
    const int max_chars_per_line = std::max(
        8,
        max_chars_for_safe_text_width(
            text_w,
            kBodyScale,
            0,
            kChatEarlyWrapChars,
            0.0f));

    const std::vector<std::string> wrapped_lines = wrap_text_to_char_lines(
        story_scene_current_line_visible_no_link(scene_state),
        max_chars_per_line,
        std::numeric_limits<int>::max());

    StorySceneBodyLayout layout {};
    layout.wrapped_lines = wrapped_lines;
    layout.visible_line_capacity = visible_line_capacity;
    const int total_lines = static_cast<int>(wrapped_lines.size());
    layout.max_scroll_lines = std::max(0, total_lines - visible_line_capacity);
    return layout;
}

int clamp_story_scene_scroll_from_bottom(
    const StorySceneBodyLayout& layout,
    const int requested_scroll_from_bottom) {
    return std::clamp(requested_scroll_from_bottom, 0, std::max(0, layout.max_scroll_lines));
}

int first_visible_story_scene_line_index(
    const StorySceneBodyLayout& layout,
    const int scroll_from_bottom) {
    const int safe_visible_capacity = std::max(1, layout.visible_line_capacity);
    const int total_lines = static_cast<int>(layout.wrapped_lines.size());
    if (total_lines <= safe_visible_capacity) {
        return 0;
    }
    const int safe_scroll = clamp_story_scene_scroll_from_bottom(layout, scroll_from_bottom);
    return std::max(0, total_lines - safe_visible_capacity - safe_scroll);
}

}  // namespace whacker::app
