#include <cstdlib>

#include "progression/skills.hpp"

namespace {

void require(const bool condition) {
    if (!condition) {
        std::abort();
    }
}

void test_focused_sixty_contact_session_lands_in_target_gain_band() {
    whacker::progression::SkillState skills {
        .edge = 0.10f,
        .power = 0.10f,
        .spin_inject = 0.10f
    };
    const float power_before = skills.power;

    const whacker::progression::SkillUsageMetrics usage {
        .edge = 0.30f,
        .power = 0.70f,
        .spin_inject = 0.20f,
        .exposure = 60.0f
    };

    whacker::progression::apply_skill_growth(skills, usage);

    const float power_delta = skills.power - power_before;
    require(power_delta >= 0.04f);
    require(power_delta <= 0.06f);
}

}  // namespace

int main() {
    test_focused_sixty_contact_session_lands_in_target_gain_band();
    return 0;
}

