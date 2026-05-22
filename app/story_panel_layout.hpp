#pragma once

#include "app_types.hpp"

namespace whacker::app {

struct StoryPanelLayoutSpec {
    float x_fraction = 0.12f;
    float y_fraction = 0.66f;
    float width_fraction = 0.76f;
    float height_fraction = 0.26f;
    float border_inset_px = 4.0f;
    float header_height_px = 38.0f;
    float text_padding_x_px = 14.0f;
    float footer_padding_bottom_px = 24.0f;
};

struct StoryPanelLayout {
    float panel_x = 0.0f;
    float panel_y = 0.0f;
    float panel_w = 0.0f;
    float panel_h = 0.0f;
    float text_x = 0.0f;
    float text_w = 0.0f;
    float footer_y = 0.0f;
};

struct StoryPanelPalette {
    Color panel_outer {0.05f, 0.09f, 0.14f};
    Color panel_header {0.09f, 0.16f, 0.24f};
};

StoryPanelLayoutSpec story_dialogue_panel_layout_spec();
StoryPanelPalette story_panel_palette();
StoryPanelLayout make_story_panel_layout(int fb_width, int fb_height, const StoryPanelLayoutSpec& spec);

#if defined(WHACKER_PLATFORM_SDL2)
void draw_story_panel_background(
    int fb_width,
    int fb_height,
    const StoryPanelLayout& layout,
    const StoryPanelLayoutSpec& spec,
    const StoryPanelPalette& palette);
#endif

}  // namespace whacker::app
