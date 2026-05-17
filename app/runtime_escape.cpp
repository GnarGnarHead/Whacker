#include "runtime_escape.hpp"

#ifdef WHACKER_HAS_GLFW

#include <GLFW/glfw3.h>

#include "menu_input.hpp"
#include "story_runtime_invariants.hpp"
#include "story_scene.hpp"

namespace whacker::app {

bool handle_runtime_escape_key(
    GLFWwindow* window,
    AppState& app_state,
    AppState& pause_return_state,
    StoryMenuState& story_menu_state,
    OptionsMenuState& options_menu_state,
    PauseMenuState& pause_menu_state,
    StorySceneState& story_scene_state,
    PaddleTuningState& paddle_tuning_state,
    StoryRuntimeState& story_runtime) {
    bool played_confirm = false;

    if (app_state == AppState::StoryMenu && story_menu_state.confirm_overwrite) {
        story_menu_state.confirm_overwrite = false;
        story_menu_state.confirm_selected = 0;
        played_confirm = true;
    } else if (app_state == AppState::OptionsMenu && options_menu_state.waiting_for_key) {
        options_menu_state.waiting_for_key = false;
        clear_last_pressed_key();
        played_confirm = true;
    } else if (app_state == AppState::Paused) {
        if (pause_menu_state.confirm_forfeit) {
            pause_menu_state.confirm_forfeit = false;
            pause_menu_state.confirm_selected = 0;
            played_confirm = true;
        } else {
            app_state = pause_return_state;
            pause_menu_state.selected_row = PauseMenuRowResume;
            played_confirm = true;
        }
    } else if (app_state == AppState::Playing) {
        app_state = AppState::Paused;
        pause_return_state = AppState::Playing;
        pause_menu_state.selected_row = PauseMenuRowResume;
        pause_menu_state.confirm_forfeit = false;
        pause_menu_state.confirm_selected = 0;
        played_confirm = true;
    } else if (app_state == AppState::StoryIntro) {
        app_state = AppState::Paused;
        pause_return_state = AppState::StoryIntro;
        pause_menu_state.selected_row = PauseMenuRowResume;
        pause_menu_state.confirm_forfeit = false;
        pause_menu_state.confirm_selected = 0;
        played_confirm = true;
    } else if (app_state == AppState::StoryScene) {
        clear_story_scene(story_scene_state);
        clear_story_runtime_scene_pending_flags(story_runtime);
        app_state = AppState::StoryMenu;
        played_confirm = true;
    } else if (app_state == AppState::PaddleTuning) {
        paddle_tuning_state.active = false;
        app_state = paddle_tuning_state.return_state;
        played_confirm = true;
    } else if (app_state == AppState::MainMenu) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    } else {
        app_state = AppState::MainMenu;
        played_confirm = true;
    }

    return played_confirm;
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
