#include <cmath>
#include <cstdlib>
#include <vector>

#include <GLFW/glfw3.h>

#include "play_control.hpp"

namespace {

namespace app = whacker::app;
namespace sim = whacker::sim;

struct TraceFrame {
    float left_target_y = 0.0f;
    float right_target_y = 0.0f;
    float left_feedforward = 0.0f;
    float right_feedforward = 0.0f;

    bool left_has_plan = false;
    bool right_has_plan = false;
    int left_candidate_id = -1;
    int right_candidate_id = -1;
    float left_score = 0.0f;
    float right_score = 0.0f;
    std::uint64_t left_signature = 0ULL;
    std::uint64_t right_signature = 0ULL;
};

bool approx_equal(const float a, const float b, const float eps = 1.0e-6f) {
    return std::fabs(a - b) <= eps;
}

void require(const bool condition) {
    if (!condition) {
        std::abort();
    }
}

std::vector<TraceFrame> run_trace_capture() {
    sim::Simulation simulation {};
    auto& state = simulation.mutable_state();
    state.ball.position.x = 420.0f;
    state.ball.position.y = 142.0f;
    state.ball.velocity.x = -280.0f;
    state.ball.velocity.y = 65.0f;
    state.ball.spin = 1.4f;
    state.ball.speed_scalar = 1.08f;
    state.left.center_y = 220.0f;
    state.right.center_y = 160.0f;

    app::MatchOptions options {};
    options.left_mode = app::PaddleMode::AI;
    options.right_mode = app::PaddleMode::AI;
    options.left_paddle_skills = {.edge = 0.57f, .power = 0.57f, .spin_inject = 0.56f};
    options.right_paddle_skills = {.edge = 0.34f, .power = 0.33f, .spin_inject = 0.33f};

    app::ControlBindings controls {};
    app::RuntimeAiState left_ai {};
    app::RuntimeAiState right_ai {};

    std::vector<TraceFrame> trace;
    trace.reserve(1200);
    for (int i = 0; i < 1200; ++i) {
        app::update_targets_for_play(
            nullptr,
            simulation,
            options,
            controls,
            left_ai,
            right_ai,
            sim::kFixedDt,
            nullptr);

        const auto& snapshot = simulation.state();
        trace.push_back(TraceFrame {
            .left_target_y = snapshot.left.target_y,
            .right_target_y = snapshot.right.target_y,
            .left_feedforward = snapshot.left.feedforward_velocity_y,
            .right_feedforward = snapshot.right.feedforward_velocity_y,
            .left_has_plan = left_ai.plan.has_plan,
            .right_has_plan = right_ai.plan.has_plan,
            .left_candidate_id = left_ai.plan.candidate_id,
            .right_candidate_id = right_ai.plan.candidate_id,
            .left_score = static_cast<float>(snapshot.left_score),
            .right_score = static_cast<float>(snapshot.right_score),
            .left_signature = left_ai.plan.state_signature,
            .right_signature = right_ai.plan.state_signature,
        });

        (void)simulation.step(sim::kFixedDt);
    }
    return trace;
}

void test_deterministic_trace_replay() {
    const std::vector<TraceFrame> a = run_trace_capture();
    const std::vector<TraceFrame> b = run_trace_capture();

    require(a.size() == b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        require(approx_equal(a[i].left_target_y, b[i].left_target_y));
        require(approx_equal(a[i].right_target_y, b[i].right_target_y));
        require(approx_equal(a[i].left_feedforward, b[i].left_feedforward));
        require(approx_equal(a[i].right_feedforward, b[i].right_feedforward));

        require(a[i].left_has_plan == b[i].left_has_plan);
        require(a[i].right_has_plan == b[i].right_has_plan);
        require(a[i].left_candidate_id == b[i].left_candidate_id);
        require(a[i].right_candidate_id == b[i].right_candidate_id);
        require(a[i].left_signature == b[i].left_signature);
        require(a[i].right_signature == b[i].right_signature);

        require(approx_equal(a[i].left_score, b[i].left_score));
        require(approx_equal(a[i].right_score, b[i].right_score));
    }
}

}  // namespace

extern "C" int glfwGetKey(GLFWwindow* /*window*/, const int /*key*/) {
    return GLFW_RELEASE;
}

int main() {
    test_deterministic_trace_replay();
    return 0;
}
