#pragma once

#include <string>

#include "sim/config.hpp"

namespace whacker::sim {

bool load_config_from_json_file(const std::string& path, SimulationConfig& config, std::string* error_message);

}  // namespace whacker::sim

