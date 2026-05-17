#include <cstdlib>
#include <cmath>

#include "runtime_frame_timing.hpp"

namespace {

bool nearly_equal(const double a, const double b) {
    return std::abs(a - b) < 1.0e-9;
}

void require(const bool condition) {
    if (!condition) {
        std::abort();
    }
}

void test_advance_runtime_frame_timing_clamps_large_dt_and_updates_counters() {
    whacker::app::RuntimeFrameTimingState timing {};
    timing.previous_time = 10.0;
    timing.accumulator = 0.03;
    timing.title_cooldown = 0.20;
    timing.menu_input_lockout = 0.05;

    const double frame_dt = whacker::app::advance_runtime_frame_timing(timing, 10.20, 0.25);

    require(nearly_equal(frame_dt, 0.10));
    require(nearly_equal(timing.previous_time, 10.20));
    require(nearly_equal(timing.accumulator, 0.13));
    require(nearly_equal(timing.title_cooldown, 0.30));
    require(nearly_equal(timing.menu_input_lockout, 0.0));
}

void test_advance_runtime_frame_timing_clamps_negative_dt_to_zero() {
    whacker::app::RuntimeFrameTimingState timing {};
    timing.previous_time = 5.0;
    timing.accumulator = 0.12;
    timing.title_cooldown = 0.40;
    timing.menu_input_lockout = 0.07;

    const double frame_dt = whacker::app::advance_runtime_frame_timing(timing, 4.50, 0.20);

    require(nearly_equal(frame_dt, 0.0));
    require(nearly_equal(timing.previous_time, 4.50));
    require(nearly_equal(timing.accumulator, 0.12));
    require(nearly_equal(timing.title_cooldown, 0.40));
    require(nearly_equal(timing.menu_input_lockout, 0.07));
}

}  // namespace

int main() {
    test_advance_runtime_frame_timing_clamps_large_dt_and_updates_counters();
    test_advance_runtime_frame_timing_clamps_negative_dt_to_zero();
    return 0;
}
