#pragma once

#ifdef WHACKER_HAS_GLFW

struct GLFWwindow;

namespace whacker::app {

struct StorySceneState;

void render_story_scene_overlay(
    GLFWwindow* window,
    const StorySceneState& scene_state);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
