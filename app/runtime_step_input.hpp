#pragma once

#ifdef WHACKER_HAS_GLFW

struct GLFWwindow;

namespace whacker::app {

struct RuntimeStepInputSnapshot {
    bool text_fast_held = false;
};

RuntimeStepInputSnapshot sample_runtime_step_input(GLFWwindow* window);

}  // namespace whacker::app

#endif  // WHACKER_HAS_GLFW
