#include "runtime_step_input.hpp"

#ifdef WHACKER_HAS_GLFW

#include <GLFW/glfw3.h>

namespace whacker::app {

RuntimeStepInputSnapshot sample_runtime_step_input(GLFWwindow* window) {
    RuntimeStepInputSnapshot snapshot {};
    snapshot.text_fast_held =
        glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_KP_ENTER) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    return snapshot;
}

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
