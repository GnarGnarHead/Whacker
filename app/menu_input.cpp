#include "menu_input.hpp"

#ifdef WHACKER_HAS_GLFW

namespace whacker::app {

namespace {
int g_last_pressed_key = GLFW_KEY_UNKNOWN;
}  // namespace

void on_key_event(
    GLFWwindow* /*window*/,
    const int key,
    const int /*scancode*/,
    const int action,
    const int /*mods*/) {
    if (action == GLFW_PRESS) {
        g_last_pressed_key = key;
    }
}

void clear_last_pressed_key() {
    g_last_pressed_key = GLFW_KEY_UNKNOWN;
}

int consume_last_pressed_key() {
    const int key = g_last_pressed_key;
    g_last_pressed_key = GLFW_KEY_UNKNOWN;
    return key;
}

bool is_bindable_key(const int key) {
    if (key < GLFW_KEY_SPACE || key > GLFW_KEY_LAST) {
        return false;
    }
    return key != GLFW_KEY_ENTER && key != GLFW_KEY_KP_ENTER && key != GLFW_KEY_ESCAPE;
}

bool key_to_name_char(const int key, char& out_char) {
    if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
        out_char = static_cast<char>('A' + (key - GLFW_KEY_A));
        return true;
    }
    if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
        out_char = static_cast<char>('0' + (key - GLFW_KEY_0));
        return true;
    }
    if (key == GLFW_KEY_SPACE) {
        out_char = ' ';
        return true;
    }
    if (key == GLFW_KEY_MINUS) {
        out_char = '-';
        return true;
    }
    return false;
}

const char* key_name(const int key) {
    static thread_local char single[2];
    if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
        single[0] = static_cast<char>('A' + (key - GLFW_KEY_A));
        single[1] = '\0';
        return single;
    }
    if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
        single[0] = static_cast<char>('0' + (key - GLFW_KEY_0));
        single[1] = '\0';
        return single;
    }
    switch (key) {
        case GLFW_KEY_UP:
            return "UP";
        case GLFW_KEY_DOWN:
            return "DOWN";
        case GLFW_KEY_LEFT:
            return "LEFT";
        case GLFW_KEY_RIGHT:
            return "RIGHT";
        case GLFW_KEY_SPACE:
            return "SPACE";
        case GLFW_KEY_TAB:
            return "TAB";
        case GLFW_KEY_LEFT_SHIFT:
            return "LSHIFT";
        case GLFW_KEY_RIGHT_SHIFT:
            return "RSHIFT";
        case GLFW_KEY_LEFT_CONTROL:
            return "LCTRL";
        case GLFW_KEY_RIGHT_CONTROL:
            return "RCTRL";
        case GLFW_KEY_LEFT_ALT:
            return "LALT";
        case GLFW_KEY_RIGHT_ALT:
            return "RALT";
        case GLFW_KEY_BACKSPACE:
            return "BACKSPACE";
        case GLFW_KEY_COMMA:
            return ",";
        case GLFW_KEY_PERIOD:
            return ".";
        case GLFW_KEY_SLASH:
            return "/";
        case GLFW_KEY_SEMICOLON:
            return ";";
        case GLFW_KEY_APOSTROPHE:
            return "'";
        case GLFW_KEY_LEFT_BRACKET:
            return "[";
        case GLFW_KEY_RIGHT_BRACKET:
            return "]";
        case GLFW_KEY_MINUS:
            return "-";
        case GLFW_KEY_EQUAL:
            return "=";
        default:
            return "?";
    }
}

bool consume_key_press(GLFWwindow* window, const int key, bool& previous_down) {
    const bool is_down = glfwGetKey(window, key) == GLFW_PRESS;
    const bool pressed = is_down && !previous_down;
    previous_down = is_down;
    return pressed;
}

bool consume_confirm_press(GLFWwindow* window, KeyEdgeState& edge_state) {
    const bool enter_pressed = consume_key_press(window, GLFW_KEY_ENTER, edge_state.enter);
    const bool kp_enter_pressed = consume_key_press(window, GLFW_KEY_KP_ENTER, edge_state.kp_enter);
    const bool space_pressed = consume_key_press(window, GLFW_KEY_SPACE, edge_state.space);
    return enter_pressed || kp_enter_pressed || space_pressed;
}

bool consume_menu_up_press(
    GLFWwindow* window,
    KeyEdgeState& edge_state,
    const ControlBindings& controls) {
    bool pressed = false;
    pressed = consume_key_press(window, GLFW_KEY_UP, edge_state.up) || pressed;
    if (controls.p1_up != GLFW_KEY_UP) {
        pressed = consume_key_press(window, controls.p1_up, edge_state.p1_up) || pressed;
    }
    if (controls.p2_up != GLFW_KEY_UP && controls.p2_up != controls.p1_up) {
        pressed = consume_key_press(window, controls.p2_up, edge_state.p2_up) || pressed;
    }
    return pressed;
}

bool consume_menu_down_press(
    GLFWwindow* window,
    KeyEdgeState& edge_state,
    const ControlBindings& controls) {
    bool pressed = false;
    pressed = consume_key_press(window, GLFW_KEY_DOWN, edge_state.down) || pressed;
    if (controls.p1_down != GLFW_KEY_DOWN) {
        pressed = consume_key_press(window, controls.p1_down, edge_state.p1_down) || pressed;
    }
    if (controls.p2_down != GLFW_KEY_DOWN && controls.p2_down != controls.p1_down) {
        pressed = consume_key_press(window, controls.p2_down, edge_state.p2_down) || pressed;
    }
    return pressed;
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
