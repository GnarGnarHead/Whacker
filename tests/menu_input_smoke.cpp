#include <cstdlib>

#include <GLFW/glfw3.h>

#include "menu_input.hpp"

namespace {

int g_key_states[GLFW_KEY_LAST + 1] = {};

void reset_key_states() {
    for (int i = 0; i <= GLFW_KEY_LAST; ++i) {
        g_key_states[i] = GLFW_RELEASE;
    }
}

void require(const bool condition) {
    if (!condition) {
        std::abort();
    }
}

void set_key_state(const int key, const int state) {
    if (key < 0 || key > GLFW_KEY_LAST) {
        return;
    }
    g_key_states[key] = state;
}

void test_menu_up_press_single_edge_with_default_overlapping_bindings() {
    reset_key_states();
    whacker::app::KeyEdgeState edge_state {};
    whacker::app::ControlBindings controls {};

    set_key_state(GLFW_KEY_UP, GLFW_PRESS);
    require(whacker::app::consume_menu_up_press(nullptr, edge_state, controls));
    require(!whacker::app::consume_menu_up_press(nullptr, edge_state, controls));

    set_key_state(GLFW_KEY_UP, GLFW_RELEASE);
    require(!whacker::app::consume_menu_up_press(nullptr, edge_state, controls));

    set_key_state(GLFW_KEY_UP, GLFW_PRESS);
    require(whacker::app::consume_menu_up_press(nullptr, edge_state, controls));
}

void test_menu_down_press_single_edge_with_default_overlapping_bindings() {
    reset_key_states();
    whacker::app::KeyEdgeState edge_state {};
    whacker::app::ControlBindings controls {};

    set_key_state(GLFW_KEY_DOWN, GLFW_PRESS);
    require(whacker::app::consume_menu_down_press(nullptr, edge_state, controls));
    require(!whacker::app::consume_menu_down_press(nullptr, edge_state, controls));

    set_key_state(GLFW_KEY_DOWN, GLFW_RELEASE);
    require(!whacker::app::consume_menu_down_press(nullptr, edge_state, controls));

    set_key_state(GLFW_KEY_DOWN, GLFW_PRESS);
    require(whacker::app::consume_menu_down_press(nullptr, edge_state, controls));
}

void test_menu_up_press_deduplicates_custom_binding_collisions() {
    reset_key_states();
    whacker::app::KeyEdgeState edge_state {};
    whacker::app::ControlBindings controls {};
    controls.p1_up = GLFW_KEY_I;
    controls.p2_up = GLFW_KEY_I;

    set_key_state(GLFW_KEY_I, GLFW_PRESS);
    require(whacker::app::consume_menu_up_press(nullptr, edge_state, controls));
    require(!whacker::app::consume_menu_up_press(nullptr, edge_state, controls));
}

}  // namespace

extern "C" int glfwGetKey(GLFWwindow* /*window*/, int key) {
    if (key < 0 || key > GLFW_KEY_LAST) {
        return GLFW_RELEASE;
    }
    return g_key_states[key];
}

int main() {
    test_menu_up_press_single_edge_with_default_overlapping_bindings();
    test_menu_down_press_single_edge_with_default_overlapping_bindings();
    test_menu_up_press_deduplicates_custom_binding_collisions();
    return 0;
}
