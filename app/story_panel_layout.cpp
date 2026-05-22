#include "story_panel_layout.hpp"

#include <algorithm>

#if defined(WHACKER_PLATFORM_SDL2)
#include "pixel_font.hpp"
#endif

namespace whacker::app {

StoryPanelLayoutSpec story_dialogue_panel_layout_spec() {
    return StoryPanelLayoutSpec {};
}

StoryPanelPalette story_panel_palette() {
    return StoryPanelPalette {};
}

StoryPanelLayout make_story_panel_layout(const int fb_width, const int fb_height, const StoryPanelLayoutSpec& spec) {
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

#if defined(WHACKER_PLATFORM_SDL2)
void draw_story_panel_background(
    const int fb_width,
    const int fb_height,
    const StoryPanelLayout& layout,
    const StoryPanelLayoutSpec& spec,
    const StoryPanelPalette& palette) {
    draw_rect_pixels(
        fb_width,
        fb_height,
        layout.panel_x,
        layout.panel_y,
        layout.panel_w,
        layout.panel_h,
        palette.panel_outer.r,
        palette.panel_outer.g,
        palette.panel_outer.b);
    draw_rect_pixels(
        fb_width,
        fb_height,
        layout.panel_x + spec.border_inset_px,
        layout.panel_y + spec.border_inset_px,
        std::max(0.0f, layout.panel_w - (2.0f * spec.border_inset_px)),
        spec.header_height_px,
        palette.panel_header.r,
        palette.panel_header.g,
        palette.panel_header.b);
}
#endif

}  // namespace whacker::app
