#include "runtime_loop_policy.hpp"

#include "sim/physics.hpp"

namespace whacker::app {

RuntimeLoopPolicy build_runtime_loop_policy() {
    RuntimeLoopPolicy policy {};
    policy.max_accumulated_sim_seconds = static_cast<double>(whacker::sim::kFixedDt) * policy.max_sim_steps_per_frame;
    return policy;
}

}  // namespace whacker::app
