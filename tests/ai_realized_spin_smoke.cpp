#include <cmath>
#include <cstdlib>
#include <cstdio>

#include <GLFW/glfw3.h>

#include "play_control.hpp"

namespace {

namespace app = whacker::app;
namespace prog = whacker::progression;
namespace sim = whacker::sim;

struct SpinMetrics {
    int contacts = 0;
    int misses = 0;
    int spin_contacts = 0;
    float sum_abs_spin_delta = 0.0f;

    [[nodiscard]] float spin_on_success_ratio() const {
        if (contacts <= 0) {
            return 0.0f;
        }
        return static_cast<float>(spin_contacts) / static_cast<float>(contacts);
    }

    [[nodiscard]] float avg_abs_spin_delta() const {
        if (contacts <= 0) {
            return 0.0f;
        }
        return sum_abs_spin_delta / static_cast<float>(contacts);
    }

    [[nodiscard]] float miss_rate() const {
        const int attempts = contacts + misses;
        if (attempts <= 0) {
            return 0.0f;
        }
        return static_cast<float>(misses) / static_cast<float>(attempts);
    }
};

void require(const bool condition, const char* message) {
    if (!condition) {
        std::fputs(message, stderr);
        std::fputc('\n', stderr);
        std::abort();
    }
}

SpinMetrics run_style_series(
    const prog::SkillState& left_skills,
    const prog::SkillState& right_skills,
    const bool track_left_side,
    const int games,
    const int win_score,
    const int max_steps) {
    SpinMetrics metrics {};
    app::ControlBindings controls {};

    for (int game = 0; game < games; ++game) {
        sim::Simulation simulation {};
        app::MatchOptions options {};
        options.left_mode = app::PaddleMode::AI;
        options.right_mode = app::PaddleMode::AI;
        options.left_paddle_skills = left_skills;
        options.right_paddle_skills = right_skills;

        app::RuntimeAiState left_ai {};
        app::RuntimeAiState right_ai {};

        for (int step = 0; step < max_steps; ++step) {
            const sim::RallyState before = simulation.state();
            app::update_targets_for_play(
                nullptr,
                simulation,
                options,
                controls,
                left_ai,
                right_ai,
                sim::kFixedDt,
                nullptr);

            (void)simulation.step(sim::kFixedDt);
            const sim::RallyState after = simulation.state();

            const bool contact = (after.rally_hits == (before.rally_hits + 1));
            const bool tracked_contact = contact &&
                (track_left_side ? (after.ball.velocity.x > 0.0f) : (after.ball.velocity.x < 0.0f));
            if (tracked_contact) {
                ++metrics.contacts;
                const float spin_delta = after.ball.spin - before.ball.spin;
                const float abs_delta = std::fabs(spin_delta);
                metrics.sum_abs_spin_delta += abs_delta;
                if (abs_delta >= 0.16f) {
                    ++metrics.spin_contacts;
                }
            }

            const int conceded = track_left_side
                ? (after.right_score - before.right_score)
                : (after.left_score - before.left_score);
            if (conceded > 0) {
                metrics.misses += conceded;
            }

            const int score_gap = std::abs(after.left_score - after.right_score);
            if ((after.left_score >= win_score || after.right_score >= win_score) && score_gap >= 2) {
                break;
            }
        }
    }

    return metrics;
}

void validate_probe(const char* label, const SpinMetrics& spin_metrics, const SpinMetrics& power_metrics) {
    require(spin_metrics.contacts >= 16, "spin sample count too low");
    require(power_metrics.contacts >= 16, "power sample count too low");

    const float spin_avg = spin_metrics.avg_abs_spin_delta();
    const float power_avg = power_metrics.avg_abs_spin_delta();
    const float spin_ratio = spin_metrics.spin_on_success_ratio();
    const float power_ratio = power_metrics.spin_on_success_ratio();
    const float spin_miss = spin_metrics.miss_rate();

    const bool avg_ok = spin_avg >= (power_avg + 0.10f);
    const bool ratio_ok = spin_ratio >= (power_ratio + 0.10f);
    const bool miss_ok = spin_miss <= 0.65f;
    if (!(avg_ok && ratio_ok && miss_ok)) {
        std::fprintf(
            stderr,
            "ai_realized_spin_smoke %s metrics: spin_contacts=%d spin_misses=%d power_contacts=%d power_misses=%d spin_avg=%.3f power_avg=%.3f spin_ratio=%.3f power_ratio=%.3f spin_miss=%.3f\n",
            label,
            spin_metrics.contacts,
            spin_metrics.misses,
            power_metrics.contacts,
            power_metrics.misses,
            static_cast<double>(spin_avg),
            static_cast<double>(power_avg),
            static_cast<double>(spin_ratio),
            static_cast<double>(power_ratio),
            static_cast<double>(spin_miss));
        std::fflush(stderr);
    }

    require(avg_ok, "spin-focused avg realized spin not higher than power-focused");
    require(ratio_ok, "spin-focused spin-contact ratio not distinct from power-focused");
    require(miss_ok, "spin-focused miss rate too high");
}

void test_spin_focused_ai_converts_realized_spin() {
    constexpr int kGames = 16;
    constexpr int kWinScore = 5;
    constexpr int kMaxSteps = 5500;

    const prog::SkillState spin_focused {.edge = 0.02f, .power = 0.04f, .spin_inject = 0.40f};
    const prog::SkillState power_focused {.edge = 0.02f, .power = 0.40f, .spin_inject = 0.04f};
    const prog::SkillState reference_opponent {.edge = 0.18f, .power = 0.18f, .spin_inject = 0.18f};

    const SpinMetrics spin_left = run_style_series(
        spin_focused,
        reference_opponent,
        true,
        kGames,
        kWinScore,
        kMaxSteps);
    const SpinMetrics power_left = run_style_series(
        power_focused,
        reference_opponent,
        true,
        kGames,
        kWinScore,
        kMaxSteps);

    const SpinMetrics spin_right = run_style_series(
        reference_opponent,
        spin_focused,
        false,
        kGames,
        kWinScore,
        kMaxSteps);
    const SpinMetrics power_right = run_style_series(
        reference_opponent,
        power_focused,
        false,
        kGames,
        kWinScore,
        kMaxSteps);

    validate_probe("left", spin_left, power_left);
    validate_probe("right", spin_right, power_right);
}

}  // namespace

extern "C" int glfwGetKey(GLFWwindow* /*window*/, const int /*key*/) {
    return GLFW_RELEASE;
}

int main() {
    test_spin_focused_ai_converts_realized_spin();
    return 0;
}
