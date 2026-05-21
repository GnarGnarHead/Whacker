#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

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
    std::puts("whacker: SDL2 platform scaffold is enabled. Window/render handoff arrives in checkpoint 2.");
    const auto& state = simulation.state();
    std::printf("sim initialized: score %d:%d\n", state.left_score, state.right_score);
    return 0;
#endif
}
