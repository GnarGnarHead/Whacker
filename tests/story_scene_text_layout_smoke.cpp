#include <cassert>
#include <string>
#include <vector>

#include "story_scene.hpp"
#include "story_scene_text_layout.hpp"

namespace {

whacker::app::StorySceneState make_scene_with_text(const std::string& text) {
    whacker::app::StorySceneState scene {};
    scene.id = whacker::app::StorySceneId::OnboardingClubIntro;
    scene.line_count = 1;
    scene.line_index = 0;
    scene.lines[0] = text;
    scene.visible_chars = text.size();
    scene.dialogue_writing = false;
    return scene;
}

void test_long_scene_line_produces_positive_max_scroll() {
    const std::string text =
        "AYA: LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG "
        "LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG "
        "LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG "
        "LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG LONG.";
    const whacker::app::StorySceneState scene = make_scene_with_text(text);
    const whacker::app::StorySceneBodyLayout layout =
        whacker::app::compute_story_scene_body_layout_for_framebuffer(960, 540, scene);

    assert(layout.visible_line_capacity > 0);
    assert(!layout.wrapped_lines.empty());
    assert(static_cast<int>(layout.wrapped_lines.size()) > layout.visible_line_capacity);
    assert(layout.max_scroll_lines > 0);
}

void test_short_scene_line_has_zero_max_scroll() {
    const whacker::app::StorySceneState scene = make_scene_with_text("AYA: READY?");
    const whacker::app::StorySceneBodyLayout layout =
        whacker::app::compute_story_scene_body_layout_for_framebuffer(960, 540, scene);

    assert(layout.visible_line_capacity > 0);
    assert(layout.max_scroll_lines == 0);
}

void test_scroll_clamp_respects_layout_bounds() {
    whacker::app::StorySceneBodyLayout layout {};
    layout.visible_line_capacity = 3;
    layout.wrapped_lines = std::vector<std::string>(10, "LINE");
    layout.max_scroll_lines = 7;

    assert(whacker::app::clamp_story_scene_scroll_from_bottom(layout, -2) == 0);
    assert(whacker::app::clamp_story_scene_scroll_from_bottom(layout, 3) == 3);
    assert(whacker::app::clamp_story_scene_scroll_from_bottom(layout, 999) == 7);
}

void test_first_visible_line_index_tracks_scroll_from_bottom() {
    whacker::app::StorySceneBodyLayout layout {};
    layout.visible_line_capacity = 3;
    layout.wrapped_lines = std::vector<std::string>(10, "LINE");
    layout.max_scroll_lines = 7;

    assert(whacker::app::first_visible_story_scene_line_index(layout, 0) == 7);
    assert(whacker::app::first_visible_story_scene_line_index(layout, 2) == 5);
    assert(whacker::app::first_visible_story_scene_line_index(layout, 7) == 0);
}

}  // namespace

int main() {
    test_long_scene_line_produces_positive_max_scroll();
    test_short_scene_line_has_zero_max_scroll();
    test_scroll_clamp_respects_layout_bounds();
    test_first_visible_line_index_tracks_scroll_from_bottom();
    return 0;
}

