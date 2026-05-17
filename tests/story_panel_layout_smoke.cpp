#include <cmath>
#include <cstdlib>

#include "story_panel_layout.hpp"

namespace {

void require(const bool condition) {
    if (!condition) {
        std::abort();
    }
}

bool nearly_equal(const float a, const float b, const float epsilon = 1.0e-5f) {
    return std::fabs(a - b) <= epsilon;
}

void test_story_dialogue_panel_layout_spec_defaults_match_expected_values() {
    const whacker::app::StoryPanelLayoutSpec spec = whacker::app::story_dialogue_panel_layout_spec();
    require(nearly_equal(spec.x_fraction, 0.12f));
    require(nearly_equal(spec.y_fraction, 0.66f));
    require(nearly_equal(spec.width_fraction, 0.76f));
    require(nearly_equal(spec.height_fraction, 0.26f));
    require(nearly_equal(spec.border_inset_px, 4.0f));
    require(nearly_equal(spec.header_height_px, 38.0f));
    require(nearly_equal(spec.text_padding_x_px, 14.0f));
    require(nearly_equal(spec.footer_padding_bottom_px, 24.0f));
}

void test_make_story_panel_layout_resolves_geometry_from_spec() {
    const whacker::app::StoryPanelLayoutSpec spec = whacker::app::story_dialogue_panel_layout_spec();
    const whacker::app::StoryPanelLayout layout = whacker::app::make_story_panel_layout(1000, 800, spec);

    require(nearly_equal(layout.panel_x, 120.0f));
    require(nearly_equal(layout.panel_y, 528.0f));
    require(nearly_equal(layout.panel_w, 760.0f));
    require(nearly_equal(layout.panel_h, 208.0f));
    require(nearly_equal(layout.text_x, 134.0f));
    require(nearly_equal(layout.text_w, 732.0f));
    require(nearly_equal(layout.footer_y, 712.0f));
}

void test_make_story_panel_layout_honors_custom_spec() {
    whacker::app::StoryPanelLayoutSpec spec {};
    spec.x_fraction = 0.25f;
    spec.y_fraction = 0.20f;
    spec.width_fraction = 0.50f;
    spec.height_fraction = 0.40f;
    spec.text_padding_x_px = 20.0f;
    spec.footer_padding_bottom_px = 10.0f;

    const whacker::app::StoryPanelLayout layout = whacker::app::make_story_panel_layout(640, 360, spec);

    require(nearly_equal(layout.panel_x, 160.0f));
    require(nearly_equal(layout.panel_y, 72.0f));
    require(nearly_equal(layout.panel_w, 320.0f));
    require(nearly_equal(layout.panel_h, 144.0f));
    require(nearly_equal(layout.text_x, 180.0f));
    require(nearly_equal(layout.text_w, 280.0f));
    require(nearly_equal(layout.footer_y, 206.0f));
}

void test_story_panel_palette_defaults_are_stable() {
    const whacker::app::StoryPanelPalette palette = whacker::app::story_panel_palette();
    require(nearly_equal(palette.panel_outer.r, 0.05f));
    require(nearly_equal(palette.panel_outer.g, 0.09f));
    require(nearly_equal(palette.panel_outer.b, 0.14f));
    require(nearly_equal(palette.panel_header.r, 0.09f));
    require(nearly_equal(palette.panel_header.g, 0.16f));
    require(nearly_equal(palette.panel_header.b, 0.24f));
}

void test_make_story_panel_layout_handles_tiny_framebuffer() {
    const whacker::app::StoryPanelLayoutSpec spec = whacker::app::story_dialogue_panel_layout_spec();
    const whacker::app::StoryPanelLayout layout = whacker::app::make_story_panel_layout(100, 100, spec);

    require(layout.panel_x >= 0.0f);
    require(layout.panel_y >= 0.0f);
    require(layout.panel_w >= 0.0f);
    require(layout.panel_h >= 0.0f);
    require(layout.text_w >= 0.0f);
    require(layout.footer_y >= layout.panel_y);
}

}  // namespace

int main() {
    test_story_dialogue_panel_layout_spec_defaults_match_expected_values();
    test_make_story_panel_layout_resolves_geometry_from_spec();
    test_make_story_panel_layout_honors_custom_spec();
    test_story_panel_palette_defaults_are_stable();
    test_make_story_panel_layout_handles_tiny_framebuffer();
    return 0;
}
