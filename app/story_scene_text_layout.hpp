#pragma once

#include <string>
#include <vector>

#ifdef WHACKER_HAS_GLFW

struct GLFWwindow;

namespace whacker::app {

struct StorySceneState;

struct StorySceneBodyLayout {
    std::vector<std::string> wrapped_lines {};
    int visible_line_capacity = 1;
    int max_scroll_lines = 0;
};

StorySceneBodyLayout compute_story_scene_body_layout_for_framebuffer(
    int fb_width,
    int fb_height,
    const StorySceneState& scene_state);

StorySceneBodyLayout compute_story_scene_body_layout_for_window(
    GLFWwindow* window,
    const StorySceneState& scene_state);

int clamp_story_scene_scroll_from_bottom(
    const StorySceneBodyLayout& layout,
    int requested_scroll_from_bottom);

int first_visible_story_scene_line_index(
    const StorySceneBodyLayout& layout,
    int scroll_from_bottom);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW

