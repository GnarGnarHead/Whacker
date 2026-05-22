#include "rgba_texture.hpp"

#include <cstdlib>
#include <iostream>

namespace {

void require(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << "rgba_texture_smoke failed: " << message << "\n";
        std::exit(1);
    }
}

void test_power_of_two_detection() {
    require(!whacker::app::is_power_of_two_dimension(0), "zero is not power of two");
    require(whacker::app::is_power_of_two_dimension(1), "one is power of two");
    require(whacker::app::is_power_of_two_dimension(64), "64 is power of two");
    require(!whacker::app::is_power_of_two_dimension(63), "63 is not power of two");
    require(!whacker::app::is_power_of_two_dimension(65), "65 is not power of two");
}

void test_next_power_of_two() {
    require(whacker::app::next_power_of_two_dimension(0) == 0, "zero has no backing size");
    require(whacker::app::next_power_of_two_dimension(1) == 1, "1 backs to 1");
    require(whacker::app::next_power_of_two_dimension(63) == 64, "63 backs to 64");
    require(whacker::app::next_power_of_two_dimension(64) == 64, "64 stays 64");
    require(whacker::app::next_power_of_two_dimension(65) == 128, "65 backs to 128");
    require(whacker::app::next_power_of_two_dimension(257) == 512, "257 backs to 512");
}

}  // namespace

int main() {
    test_power_of_two_detection();
    test_next_power_of_two();
    return 0;
}
