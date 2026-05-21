#pragma once

#include "sim/physics.hpp"

namespace whacker::app {

class SdlPlatform;

int run_app_loop(SdlPlatform& platform, whacker::sim::Simulation& simulation);

}  // namespace whacker::app
