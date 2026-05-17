#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "app_runtime.hpp"
#include "sim/config_io.hpp"
#include "sim/physics.hpp"

#ifdef WHACKER_HAS_GLFW
#include <GLFW/glfw3.h>

#include "menu_input.hpp"
#include "story_portrait_render.hpp"
#include "runtime_visual_transition_render.hpp"
#endif

namespace {

whacker::sim::SimulationConfig load_startup_config() {
    whacker::sim::SimulationConfig config {};
    std::vector<std::string> candidates;
    candidates.emplace_back("config/default.json");
    candidates.emplace_back("../config/default.json");
#ifdef WHACKER_SOURCE_DIR
    candidates.emplace_back(std::string(WHACKER_SOURCE_DIR) + "/config/default.json");
#endif

    std::string last_error;
    for (const std::string& path : candidates) {
        std::string error;
        if (whacker::sim::load_config_from_json_file(path, config, &error)) {
            std::printf("Loaded config: %s\n", path.c_str());
            return config;
        }
        last_error = error;
    }

    std::fprintf(stderr, "Warning: config file load failed, using defaults. Last error: %s\n", last_error.c_str());
    return config;
}

}  // namespace

int main() {
    const whacker::sim::SimulationConfig startup_config = load_startup_config();
    whacker::sim::Simulation simulation(startup_config);

#ifndef WHACKER_HAS_GLFW
    std::puts("whacker: built without GLFW support. Install glfw3 and rebuild to run the windowed app.");
    const auto& state = simulation.state();
    std::printf("sim initialized: score %d:%d\n", state.left_score, state.right_score);
    return 0;
#else
#if defined(__linux__) && defined(GLFW_PLATFORM_X11)
    // Force X11 on GLFW 3.4+ to avoid Wayland libdecor-gtk crashes.
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif

    if (glfwInit() == GLFW_FALSE) {
        const char* message = nullptr;
        const int error = glfwGetError(&message);
        std::fprintf(stderr, "Failed to initialize GLFW (error %d): %s\n", error, message ? message : "unknown");
        const char* display = std::getenv("DISPLAY");
        const char* wayland = std::getenv("WAYLAND_DISPLAY");
        if (display == nullptr && wayland == nullptr) {
            std::fputs("No DISPLAY/WAYLAND_DISPLAY detected. Run in a desktop session or use a virtual display.\n", stderr);
        }
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* window = glfwCreateWindow(960, 540, "Whacker", nullptr, nullptr);
    if (window == nullptr) {
        const char* message = nullptr;
        const int error = glfwGetError(&message);
        std::fprintf(stderr, "Failed to create window (error %d): %s\n", error, message ? message : "unknown");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, whacker::app::on_key_event);

    const int exit_code = whacker::app::run_app_loop(window, simulation);

    whacker::app::release_story_portrait_resources();
    whacker::app::release_visual_transition_render_resources();
    glfwDestroyWindow(window);
    glfwTerminate();
    return exit_code;
#endif
}
