#include <cassert>

#include <GLFW/glfw3.h>

#include "runtime_step_input.hpp"

namespace {

int g_enter_state = GLFW_RELEASE;
int g_kp_enter_state = GLFW_RELEASE;
int g_space_state = GLFW_RELEASE;

void reset_states() {
    g_enter_state = GLFW_RELEASE;
    g_kp_enter_state = GLFW_RELEASE;
    g_space_state = GLFW_RELEASE;
}

void test_no_keys_pressed_reports_false() {
    reset_states();
    const whacker::app::RuntimeStepInputSnapshot snapshot =
        whacker::app::sample_runtime_step_input(nullptr);
    const bool held = snapshot.text_fast_held;
    (void)held;
    assert(!held);
}

void test_enter_pressed_reports_true() {
    reset_states();
    g_enter_state = GLFW_PRESS;
    const whacker::app::RuntimeStepInputSnapshot snapshot =
        whacker::app::sample_runtime_step_input(nullptr);
    const bool held = snapshot.text_fast_held;
    (void)held;
    assert(held);
}

void test_keypad_enter_pressed_reports_true() {
    reset_states();
    g_kp_enter_state = GLFW_PRESS;
    const whacker::app::RuntimeStepInputSnapshot snapshot =
        whacker::app::sample_runtime_step_input(nullptr);
    const bool held = snapshot.text_fast_held;
    (void)held;
    assert(held);
}

void test_space_pressed_reports_true() {
    reset_states();
    g_space_state = GLFW_PRESS;
    const whacker::app::RuntimeStepInputSnapshot snapshot =
        whacker::app::sample_runtime_step_input(nullptr);
    const bool held = snapshot.text_fast_held;
    (void)held;
    assert(held);
}

void test_any_combination_pressed_reports_true() {
    reset_states();
    g_enter_state = GLFW_PRESS;
    g_kp_enter_state = GLFW_PRESS;
    g_space_state = GLFW_PRESS;
    const whacker::app::RuntimeStepInputSnapshot snapshot =
        whacker::app::sample_runtime_step_input(nullptr);
    const bool held = snapshot.text_fast_held;
    (void)held;
    assert(held);
}

}  // namespace

extern "C" int glfwGetKey(GLFWwindow* /*window*/, const int key) {
    if (key == GLFW_KEY_ENTER) {
        return g_enter_state;
    }
    if (key == GLFW_KEY_KP_ENTER) {
        return g_kp_enter_state;
    }
    if (key == GLFW_KEY_SPACE) {
        return g_space_state;
    }
    return GLFW_RELEASE;
}

int main() {
    test_no_keys_pressed_reports_false();
    test_enter_pressed_reports_true();
    test_keypad_enter_pressed_reports_true();
    test_space_pressed_reports_true();
    test_any_combination_pressed_reports_true();
    return 0;
}
