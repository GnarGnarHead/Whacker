#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "game_render.hpp"
#include "main_menu_overlay.hpp"
#include "platform_sdl.hpp"
#include "sim/config_io.hpp"
#include "sim/physics.hpp"

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

const char* main_menu_row_name(const int row) {
    using namespace whacker::app;
    switch (row) {
        case MainMenuRowStory:
            return "STORY MODE";
        case MainMenuRowQuick:
            return "QUICK MATCH";
        case MainMenuRowOptions:
            return "OPTIONS";
        case MainMenuRowQuit:
            return "QUIT";
        default:
            return "?";
    }
}

}  // namespace

int main() {
    const whacker::sim::SimulationConfig startup_config = load_startup_config();
    whacker::sim::Simulation simulation(startup_config);

#ifndef WHACKER_PLATFORM_SDL2
    std::puts("whacker: built without an app platform. Reconfigure with WHACKER_BUILD_APP=ON and SDL2 installed.");
    const auto& state = simulation.state();
    std::printf("sim initialized: score %d:%d\n", state.left_score, state.right_score);
    return 0;
#else
    whacker::app::SdlPlatform platform;
    std::string error_message;
    if (!platform.init(&error_message)) {
        std::fprintf(stderr, "Failed to initialize SDL2 platform: %s\n", error_message.c_str());
        return 1;
    }
    if (!platform.create_window(960, 540, "Whacker", &error_message)) {
        std::fprintf(stderr, "Failed to create SDL2 OpenGL window: %s\n", error_message.c_str());
        return 1;
    }

    whacker::app::MainMenuState main_menu_state {};
    while (!platform.should_close()) {
        platform.poll_events();
        whacker::app::render_scene(&platform, simulation, true);
        whacker::app::render_main_menu_overlay(&platform, main_menu_state, main_menu_row_name);
        platform.swap_buffers();
    }

    platform.destroy_window();
    return 0;
#endif
}
