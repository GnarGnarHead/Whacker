#pragma once

#ifdef WHACKER_HAS_GLFW

#include <GLFW/glfw3.h>

namespace whacker::app {

struct KeyEdgeState {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool p1_up = false;
    bool p1_down = false;
    bool p2_up = false;
    bool p2_down = false;
    bool backspace = false;
    bool enter = false;
    bool kp_enter = false;
    bool space = false;
    bool escape = false;
    bool menu = false;
    bool dev_info = false;
    bool dev_player_ai = false;
};

struct ControlBindings {
    int p1_up = GLFW_KEY_W;
    int p1_down = GLFW_KEY_S;
    int p2_up = GLFW_KEY_UP;
    int p2_down = GLFW_KEY_DOWN;
};

void on_key_event(GLFWwindow* window, int key, int scancode, int action, int mods);
void clear_last_pressed_key();
int consume_last_pressed_key();
bool is_bindable_key(int key);
bool key_to_name_char(int key, char& out_char);
const char* key_name(int key);
bool consume_key_press(GLFWwindow* window, int key, bool& previous_down);
bool consume_confirm_press(GLFWwindow* window, KeyEdgeState& edge_state);
bool consume_menu_up_press(GLFWwindow* window, KeyEdgeState& edge_state, const ControlBindings& controls);
bool consume_menu_down_press(GLFWwindow* window, KeyEdgeState& edge_state, const ControlBindings& controls);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
