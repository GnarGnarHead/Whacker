#include <cmath>
#include <cstdlib>

#include "runtime_loop_policy.hpp"
#include "sim/physics.hpp"

namespace {

void require(const bool condition) {
    if (!condition) {
        std::abort();
    }
}

bool nearly_equal(const double a, const double b) {
    return std::abs(a - b) < 1.0e-12;
}

void test_build_runtime_loop_policy_preserves_runtime_loop_constants() {
    const whacker::app::RuntimeLoopPolicy policy = whacker::app::build_runtime_loop_policy();
    const double expected_max_accumulated =
        static_cast<double>(whacker::sim::kFixedDt) * policy.max_sim_steps_per_frame;

    require(policy.story_official_games_to_win == 3);
    require(nearly_equal(policy.menu_input_lockout_seconds, 0.14));
    require(nearly_equal(policy.max_sim_steps_per_frame, 8.0));
    require(nearly_equal(policy.max_accumulated_sim_seconds, expected_max_accumulated));
}

}  // namespace

int main() {
    test_build_runtime_loop_policy_preserves_runtime_loop_constants();
    return 0;
}
