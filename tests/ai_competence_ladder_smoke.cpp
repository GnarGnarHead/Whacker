#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <limits>

#include <GLFW/glfw3.h>

#include "play_control.hpp"
#include "sim/collision.hpp"
#include "sim/math.hpp"
#include "sim/spin.hpp"

namespace {

namespace app = whacker::app;
namespace prog = whacker::progression;
namespace sim = whacker::sim;

struct MatchResult {
    int left_score = 0;
    int right_score = 0;
    int points_scored = 0;
    int contact_events = 0;
    int tracking_frames = 0;
    int anti_tracking_frames = 0;
};

struct DuelResult {
    int stronger_wins = 0;
    int weaker_wins = 0;
    int ties = 0;
    int stronger_points = 0;
    int weaker_points = 0;
    int total_points_scored = 0;
    int contact_events = 0;
    int games = 0;
    int tracking_frames = 0;
    int anti_tracking_frames = 0;

    [[nodiscard]] int decisive_games() const {
        return stronger_wins + weaker_wins;
    }

    [[nodiscard]] float stronger_win_rate() const {
        const int decisive = decisive_games();
        if (decisive <= 0) {
            return 0.0f;
        }
        return static_cast<float>(stronger_wins) / static_cast<float>(decisive);
    }

    [[nodiscard]] float contacts_per_game() const {
        if (games <= 0) {
            return 0.0f;
        }
        return static_cast<float>(contact_events) / static_cast<float>(games);
    }

    [[nodiscard]] float total_points_per_game() const {
        if (games <= 0) {
            return 0.0f;
        }
        return static_cast<float>(total_points_scored) / static_cast<float>(games);
    }

    [[nodiscard]] float contacts_per_point() const {
        if (total_points_scored <= 0) {
            return std::numeric_limits<float>::infinity();
        }
        return static_cast<float>(contact_events) / static_cast<float>(total_points_scored);
    }

    [[nodiscard]] float tie_ratio() const {
        if (games <= 0) {
            return 0.0f;
        }
        return static_cast<float>(ties) / static_cast<float>(games);
    }

    [[nodiscard]] float anti_tracking_ratio() const {
        if (tracking_frames <= 0) {
            return 0.0f;
        }
        return static_cast<float>(anti_tracking_frames) / static_cast<float>(tracking_frames);
    }
};

float clampf(const float value, const float lo, const float hi) {
    return std::max(lo, std::min(value, hi));
}

float signf(const float value) {
    if (value > 0.0f) {
        return 1.0f;
    }
    if (value < 0.0f) {
        return -1.0f;
    }
    return 0.0f;
}

void require(const bool condition, const char* message) {
    if (!condition) {
        std::fputs(message, stderr);
        std::fputc('\n', stderr);
        std::abort();
    }
}

float paddle_contact_plane_x(const sim::SimulationConfig& config, const bool for_left_paddle) {
    const float paddle_x =
        for_left_paddle ? config.paddle_x_margin : (config.court_width - config.paddle_x_margin);
    return for_left_paddle ? (paddle_x + config.paddle_half_width + config.ball_radius)
                           : (paddle_x - config.paddle_half_width - config.ball_radius);
}

struct InterceptEstimate {
    bool predicted = false;
    float t_hit = -1.0f;
    float intercept_y = 0.0f;
};

InterceptEstimate predict_inbound_intercept(
    const sim::RallyState& state,
    const sim::SimulationConfig& config,
    const bool for_left) {
    InterceptEstimate out {};
    const float vx = state.ball.velocity.x;
    if (for_left ? (vx >= -1.0e-4f) : (vx <= 1.0e-4f)) {
        return out;
    }

    sim::BallState probe = state.ball;
    const float plane_x = paddle_contact_plane_x(config, for_left);
    constexpr int kMaxPredictSteps = 240;

    for (int i = 0; i < kMaxPredictSteps; ++i) {
        const sim::Vec2 prev = probe.position;
        sim::decay_speed_scalar(probe, config, sim::kFixedDt);
        sim::apply_spin_curve(probe, config, sim::kFixedDt);
        const float target_speed = config.ball_base_speed * probe.speed_scalar;
        sim::renormalize_velocity(probe, target_speed, probe.velocity.x >= 0.0f ? 1.0f : -1.0f);
        probe.position.x += probe.velocity.x * sim::kFixedDt;
        probe.position.y += probe.velocity.y * sim::kFixedDt;
        sim::decay_spin(probe, config, sim::kFixedDt);

        if (sim::handle_wall_bounce(probe, config)) {
            const float post_wall_speed = config.ball_base_speed * probe.speed_scalar;
            sim::renormalize_velocity(probe, post_wall_speed, probe.velocity.x >= 0.0f ? 1.0f : -1.0f);
        }

        if (sim::handle_scoring(probe, config) != sim::ScoreEvent::None) {
            return out;
        }

        const bool crossed =
            for_left ? ((prev.x > plane_x) && (probe.position.x <= plane_x))
                     : ((prev.x < plane_x) && (probe.position.x >= plane_x));
        if (!crossed) {
            continue;
        }

        const float dx = probe.position.x - prev.x;
        const float t = std::abs(dx) > 1.0e-6f ? clampf((plane_x - prev.x) / dx, 0.0f, 1.0f) : 1.0f;
        out.predicted = true;
        out.t_hit = (static_cast<float>(i) + t) * sim::kFixedDt;
        out.intercept_y = clampf(
            prev.y + ((probe.position.y - prev.y) * t),
            config.ball_radius,
            config.court_height - config.ball_radius);
        return out;
    }

    return out;
}

void accumulate_tracking_metrics(const sim::RallyState& state, const sim::SimulationConfig& config, MatchResult& result) {
    auto sample_side = [&](const bool for_left) {
        const InterceptEstimate intercept = predict_inbound_intercept(state, config, for_left);
        if (!intercept.predicted || intercept.t_hit < 0.14f || intercept.t_hit > 0.90f) {
            return;
        }

        const auto& paddle = for_left ? state.left : state.right;
        const float error = intercept.intercept_y - paddle.center_y;
        const float command = paddle.target_y - paddle.center_y;

        if (std::fabs(error) < 12.0f || std::fabs(command) < 2.0f) {
            return;
        }

        ++result.tracking_frames;
        if (signf(error) != signf(command)) {
            ++result.anti_tracking_frames;
        }
    };

    sample_side(true);
    sample_side(false);
}

MatchResult run_game(
    const prog::SkillState& left_skills,
    const prog::SkillState& right_skills,
    const bool opening_serve_to_right,
    const int win_score,
    const int max_steps) {
    MatchResult result {};
    sim::Simulation simulation {};
    auto& initial_state = simulation.mutable_state();
    const auto& config = simulation.config();
    initial_state.ball.position.x = 0.5f * config.court_width;
    initial_state.ball.position.y = 0.5f * config.court_height;
    initial_state.ball.velocity.x = opening_serve_to_right ? config.ball_base_speed : -config.ball_base_speed;
    initial_state.ball.velocity.y = 0.0f;
    initial_state.ball.spin = 0.0f;
    initial_state.ball.speed_scalar = 1.0f;
    initial_state.rally_hits = 0;

    app::MatchOptions options {};
    options.left_mode = app::PaddleMode::AI;
    options.right_mode = app::PaddleMode::AI;
    options.left_paddle_skills = left_skills;
    options.right_paddle_skills = right_skills;

    app::RuntimeAiState left_ai {};
    app::RuntimeAiState right_ai {};
    app::ControlBindings controls {};

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

        const sim::RallyState planned = simulation.state();
        accumulate_tracking_metrics(planned, simulation.config(), result);

        const sim::ScoreEvent score_event = simulation.step(sim::kFixedDt);
        (void)score_event;
        const auto& state = simulation.state();
        const bool contact = (state.rally_hits == (before.rally_hits + 1));
        if (contact) {
            ++result.contact_events;
        }

        const int score_gap = std::abs(state.left_score - state.right_score);
        if ((state.left_score >= win_score || state.right_score >= win_score) && score_gap >= 2) {
            break;
        }
    }

    const auto& final_state = simulation.state();
    result.left_score = final_state.left_score;
    result.right_score = final_state.right_score;
    result.points_scored = final_state.left_score + final_state.right_score;
    return result;
}

DuelResult run_fixed_side_duel(
    const prog::SkillState& stronger,
    const prog::SkillState& weaker,
    const bool stronger_on_left,
    const int games,
    const int win_score,
    const int max_steps) {
    DuelResult result {};

    const prog::SkillState left_skills = stronger_on_left ? stronger : weaker;
    const prog::SkillState right_skills = stronger_on_left ? weaker : stronger;

    for (int game = 0; game < games; ++game) {
        const bool opening_serve_to_right = ((game % 2) == 0);
        const MatchResult match = run_game(left_skills, right_skills, opening_serve_to_right, win_score, max_steps);
        const bool left_won = match.left_score > match.right_score;
        const bool tie = match.left_score == match.right_score;

        if (stronger_on_left) {
            result.stronger_points += match.left_score;
            result.weaker_points += match.right_score;
        } else {
            result.stronger_points += match.right_score;
            result.weaker_points += match.left_score;
        }

        result.contact_events += match.contact_events;
        result.total_points_scored += match.points_scored;
        result.tracking_frames += match.tracking_frames;
        result.anti_tracking_frames += match.anti_tracking_frames;
        ++result.games;

        if (tie) {
            ++result.ties;
            continue;
        }

        const bool stronger_won = stronger_on_left ? left_won : !left_won;
        if (stronger_won) {
            ++result.stronger_wins;
        } else {
            ++result.weaker_wins;
        }
    }

    return result;
}

DuelResult combine(const DuelResult& a, const DuelResult& b) {
    DuelResult out {};
    out.stronger_wins = a.stronger_wins + b.stronger_wins;
    out.weaker_wins = a.weaker_wins + b.weaker_wins;
    out.ties = a.ties + b.ties;
    out.stronger_points = a.stronger_points + b.stronger_points;
    out.weaker_points = a.weaker_points + b.weaker_points;
    out.total_points_scored = a.total_points_scored + b.total_points_scored;
    out.contact_events = a.contact_events + b.contact_events;
    out.games = a.games + b.games;
    out.tracking_frames = a.tracking_frames + b.tracking_frames;
    out.anti_tracking_frames = a.anti_tracking_frames + b.anti_tracking_frames;
    return out;
}

DuelResult run_mirror_duel(
    const prog::SkillState& skills,
    const int games,
    const int win_score,
    const int max_steps) {
    return run_fixed_side_duel(skills, skills, true, games, win_score, max_steps);
}

struct SideWinStats {
    int left_wins = 0;
    int right_wins = 0;
    int ties = 0;
    int games = 0;

    [[nodiscard]] int decisive_games() const {
        return left_wins + right_wins;
    }

    [[nodiscard]] float left_win_rate() const {
        const int decisive = decisive_games();
        if (decisive <= 0) {
            return 0.0f;
        }
        return static_cast<float>(left_wins) / static_cast<float>(decisive);
    }

    [[nodiscard]] float tie_ratio() const {
        if (games <= 0) {
            return 0.0f;
        }
        return static_cast<float>(ties) / static_cast<float>(games);
    }
};

SideWinStats run_equal_fixed_side_outcomes(
    const prog::SkillState& skills,
    const int games,
    const int win_score,
    const int max_steps,
    const bool alternate_serve,
    const bool first_serve_to_right) {
    SideWinStats stats {};
    for (int game = 0; game < games; ++game) {
        const bool opening_serve_to_right = alternate_serve
            ? ((game % 2) == 0 ? first_serve_to_right : !first_serve_to_right)
            : first_serve_to_right;
        const MatchResult match = run_game(skills, skills, opening_serve_to_right, win_score, max_steps);
        ++stats.games;
        if (match.left_score == match.right_score) {
            ++stats.ties;
        } else if (match.left_score > match.right_score) {
            ++stats.left_wins;
        } else {
            ++stats.right_wins;
        }
    }
    return stats;
}

struct LadderMetrics {
    DuelResult kai_vs_player_left {};
    DuelResult kai_vs_player_right {};
    DuelResult kai_vs_player {};
    DuelResult strong_vs_weak_left {};
    DuelResult strong_vs_weak_right {};
    DuelResult strong_vs_weak {};
    DuelResult weak_mirror {};
    DuelResult expert_mirror {};
    SideWinStats equal_fixed_alt {};
    SideWinStats equal_fixed_serve_right {};
    SideWinStats equal_fixed_serve_left {};

    float strong_left_rate = 0.0f;
    float strong_right_rate = 0.0f;
    float weak_contacts_per_game = 0.0f;
    float strong_weak_contacts_per_game = 0.0f;
    float weak_anti_ratio = 0.0f;
    float strong_weak_anti_ratio = 0.0f;
    float kai_vs_player_points_per_game = 0.0f;
    float kai_vs_player_contacts_per_point = 0.0f;
    float kai_vs_player_tie_ratio = 0.0f;
    float expert_points_per_game = 0.0f;
    float expert_contacts_per_point = 0.0f;
    float expert_tie_ratio = 0.0f;
    float equal_fixed_alt_left_rate = 0.0f;
    float equal_fixed_alt_tie_ratio = 0.0f;
    float equal_fixed_serve_right_left_rate = 0.0f;
    float equal_fixed_serve_right_tie_ratio = 0.0f;
    float equal_fixed_serve_left_left_rate = 0.0f;
    float equal_fixed_serve_left_tie_ratio = 0.0f;
    float fixed_serve_complement_error = 0.0f;
};

LadderMetrics collect_ladder_metrics() {
    constexpr int kGamesPerSide = 8;
    constexpr int kWinScore = 5;
    constexpr int kMaxSteps = 12000;
    constexpr int kExpertMaxSteps = 14000;
    constexpr int kParityGames = 16;

    const prog::SkillState kPlayer {.edge = 0.10f, .power = 0.10f, .spin_inject = 0.10f};
    const prog::SkillState kKai {.edge = 0.12f, .power = 0.12f, .spin_inject = 0.12f};
    const prog::SkillState kWeak {.edge = 0.12f, .power = 0.12f, .spin_inject = 0.12f};
    const prog::SkillState kStrong {.edge = 0.57f, .power = 0.57f, .spin_inject = 0.56f};
    const prog::SkillState kExpert {.edge = 0.50f, .power = 0.80f, .spin_inject = 0.40f};

    LadderMetrics metrics {};
    metrics.kai_vs_player_left = run_fixed_side_duel(kKai, kPlayer, true, kGamesPerSide, kWinScore, kMaxSteps);
    metrics.kai_vs_player_right = run_fixed_side_duel(kKai, kPlayer, false, kGamesPerSide, kWinScore, kMaxSteps);
    metrics.kai_vs_player = combine(metrics.kai_vs_player_left, metrics.kai_vs_player_right);

    metrics.strong_vs_weak_left = run_fixed_side_duel(kStrong, kWeak, true, kGamesPerSide, kWinScore, kMaxSteps);
    metrics.strong_vs_weak_right = run_fixed_side_duel(kStrong, kWeak, false, kGamesPerSide, kWinScore, kMaxSteps);
    metrics.strong_vs_weak = combine(metrics.strong_vs_weak_left, metrics.strong_vs_weak_right);

    metrics.weak_mirror = run_mirror_duel(kWeak, kGamesPerSide * 2, kWinScore, kMaxSteps);
    metrics.expert_mirror = run_mirror_duel(kExpert, kGamesPerSide * 2, kWinScore, kExpertMaxSteps);
    metrics.equal_fixed_alt = run_equal_fixed_side_outcomes(kKai, kParityGames, kWinScore, kMaxSteps, true, true);
    metrics.equal_fixed_serve_right =
        run_equal_fixed_side_outcomes(kKai, kParityGames, kWinScore, kMaxSteps, false, true);
    metrics.equal_fixed_serve_left =
        run_equal_fixed_side_outcomes(kKai, kParityGames, kWinScore, kMaxSteps, false, false);

    metrics.strong_left_rate = metrics.strong_vs_weak_left.stronger_win_rate();
    metrics.strong_right_rate = metrics.strong_vs_weak_right.stronger_win_rate();
    metrics.weak_contacts_per_game = metrics.weak_mirror.contacts_per_game();
    metrics.strong_weak_contacts_per_game = metrics.strong_vs_weak.contacts_per_game();
    metrics.weak_anti_ratio = metrics.weak_mirror.anti_tracking_ratio();
    metrics.strong_weak_anti_ratio = metrics.strong_vs_weak.anti_tracking_ratio();
    metrics.kai_vs_player_points_per_game = metrics.kai_vs_player.total_points_per_game();
    metrics.kai_vs_player_contacts_per_point = metrics.kai_vs_player.contacts_per_point();
    metrics.kai_vs_player_tie_ratio = metrics.kai_vs_player.tie_ratio();
    metrics.expert_points_per_game = metrics.expert_mirror.total_points_per_game();
    metrics.expert_contacts_per_point = metrics.expert_mirror.contacts_per_point();
    metrics.expert_tie_ratio = metrics.expert_mirror.tie_ratio();
    metrics.equal_fixed_alt_left_rate = metrics.equal_fixed_alt.left_win_rate();
    metrics.equal_fixed_alt_tie_ratio = metrics.equal_fixed_alt.tie_ratio();
    metrics.equal_fixed_serve_right_left_rate = metrics.equal_fixed_serve_right.left_win_rate();
    metrics.equal_fixed_serve_right_tie_ratio = metrics.equal_fixed_serve_right.tie_ratio();
    metrics.equal_fixed_serve_left_left_rate = metrics.equal_fixed_serve_left.left_win_rate();
    metrics.equal_fixed_serve_left_tie_ratio = metrics.equal_fixed_serve_left.tie_ratio();
    metrics.fixed_serve_complement_error =
        std::fabs(metrics.equal_fixed_serve_right_left_rate - (1.0f - metrics.equal_fixed_serve_left_left_rate));
    return metrics;
}

void dump_ladder_metrics(const LadderMetrics& metrics, const char* gate_label) {
    std::fprintf(
        stderr,
        "ai_competence_ladder_smoke [%s]: strong_win=%.3f strong_left=%.3f strong_right=%.3f weak_contacts=%.3f strong_weak_contacts=%.3f weak_anti=%.3f strong_weak_anti=%.3f kai_ppg=%.3f kai_cpp=%.3f kai_tie=%.3f expert_ppg=%.3f expert_cpp=%.3f expert_tie=%.3f equal_alt_decisive=%d equal_alt_left=%.3f equal_alt_tie=%.3f serve_right_decisive=%d serve_right_left=%.3f serve_right_tie=%.3f serve_left_decisive=%d serve_left_left=%.3f serve_left_tie=%.3f complement_err=%.3f\n",
        gate_label,
        static_cast<double>(metrics.strong_vs_weak.stronger_win_rate()),
        static_cast<double>(metrics.strong_left_rate),
        static_cast<double>(metrics.strong_right_rate),
        static_cast<double>(metrics.weak_contacts_per_game),
        static_cast<double>(metrics.strong_weak_contacts_per_game),
        static_cast<double>(metrics.weak_anti_ratio),
        static_cast<double>(metrics.strong_weak_anti_ratio),
        static_cast<double>(metrics.kai_vs_player_points_per_game),
        static_cast<double>(metrics.kai_vs_player_contacts_per_point),
        static_cast<double>(metrics.kai_vs_player_tie_ratio),
        static_cast<double>(metrics.expert_points_per_game),
        static_cast<double>(metrics.expert_contacts_per_point),
        static_cast<double>(metrics.expert_tie_ratio),
        metrics.equal_fixed_alt.decisive_games(),
        static_cast<double>(metrics.equal_fixed_alt_left_rate),
        static_cast<double>(metrics.equal_fixed_alt_tie_ratio),
        metrics.equal_fixed_serve_right.decisive_games(),
        static_cast<double>(metrics.equal_fixed_serve_right_left_rate),
        static_cast<double>(metrics.equal_fixed_serve_right_tie_ratio),
        metrics.equal_fixed_serve_left.decisive_games(),
        static_cast<double>(metrics.equal_fixed_serve_left_left_rate),
        static_cast<double>(metrics.equal_fixed_serve_left_tie_ratio),
        static_cast<double>(metrics.fixed_serve_complement_error));
}

void require_skill_order_gate(const LadderMetrics& metrics) {
    if (metrics.strong_vs_weak.stronger_win_rate() < 0.70f) {
        dump_ladder_metrics(metrics, "skill_order");
    }
    require(metrics.strong_vs_weak.stronger_win_rate() >= 0.70f, "skill-order gate failed");
}

void require_side_neutrality_gate(const LadderMetrics& metrics) {
    const bool pass =
        std::fabs(metrics.strong_left_rate - metrics.strong_right_rate) <= 0.15f &&
        metrics.equal_fixed_alt.decisive_games() >= 12 &&
        metrics.equal_fixed_alt_left_rate >= 0.48f &&
        metrics.equal_fixed_alt_left_rate <= 0.52f &&
        metrics.equal_fixed_serve_right.decisive_games() >= 12 &&
        metrics.equal_fixed_serve_left.decisive_games() >= 12 &&
        metrics.fixed_serve_complement_error <= 0.03f;
    if (!pass) {
        dump_ladder_metrics(metrics, "side_neutrality");
    }

    require(std::fabs(metrics.strong_left_rate - metrics.strong_right_rate) <= 0.15f, "side-neutrality side-rate gate failed");
    require(metrics.equal_fixed_alt.decisive_games() >= 12, "side-neutrality decisive coverage too low");
    require(metrics.equal_fixed_alt_left_rate >= 0.48f, "side-neutrality equal-fixed left-rate low");
    require(metrics.equal_fixed_alt_left_rate <= 0.52f, "side-neutrality equal-fixed left-rate high");
    require(metrics.equal_fixed_serve_right.decisive_games() >= 12, "side-neutrality fixed-serve-right decisive low");
    require(metrics.equal_fixed_serve_left.decisive_games() >= 12, "side-neutrality fixed-serve-left decisive low");
    require(metrics.fixed_serve_complement_error <= 0.03f, "side-neutrality fixed-serve complement gate failed");
}

void require_rally_band_gate(const LadderMetrics& metrics) {
    const bool pass =
        metrics.weak_contacts_per_game >= 2.0f &&
        metrics.strong_weak_contacts_per_game >= 1.5f &&
        metrics.weak_anti_ratio <= 0.32f &&
        metrics.strong_weak_anti_ratio <= 0.32f &&
        metrics.kai_vs_player_points_per_game >= 2.0f &&
        metrics.kai_vs_player_contacts_per_point >= 6.0f &&
        metrics.kai_vs_player_contacts_per_point <= 11.0f &&
        metrics.expert_points_per_game >= 2.0f &&
        metrics.expert_contacts_per_point >= 10.0f &&
        metrics.expert_contacts_per_point <= 26.0f;
    if (!pass) {
        dump_ladder_metrics(metrics, "rally_band");
    }

    require(metrics.weak_contacts_per_game >= 2.0f, "rally-band weak contact reliability failed");
    require(metrics.strong_weak_contacts_per_game >= 1.5f, "rally-band strong-vs-weak contact reliability failed");
    require(metrics.weak_anti_ratio <= 0.32f, "rally-band weak anti-tracking too high");
    require(metrics.strong_weak_anti_ratio <= 0.32f, "rally-band strong-vs-weak anti-tracking too high");
    require(metrics.kai_vs_player_points_per_game >= 2.0f, "rally-band kai points-per-game failed");
    require(metrics.kai_vs_player_contacts_per_point >= 6.0f, "rally-band kai rally too short");
    require(metrics.kai_vs_player_contacts_per_point <= 11.0f, "rally-band kai rally too long");
    require(metrics.expert_points_per_game >= 2.0f, "rally-band expert points-per-game failed");
    require(metrics.expert_contacts_per_point >= 10.0f, "rally-band expert rally too short");
    require(metrics.expert_contacts_per_point <= 26.0f, "rally-band expert rally too long");
}

void require_tie_rate_gate(const LadderMetrics& metrics) {
    const bool pass =
        metrics.kai_vs_player_tie_ratio <= 0.40f &&
        metrics.expert_tie_ratio <= 0.60f &&
        metrics.equal_fixed_alt_tie_ratio <= 0.20f &&
        metrics.equal_fixed_serve_right_tie_ratio <= 0.20f &&
        metrics.equal_fixed_serve_left_tie_ratio <= 0.20f;
    if (!pass) {
        dump_ladder_metrics(metrics, "tie_rate");
    }

    require(metrics.kai_vs_player_tie_ratio <= 0.40f, "tie-rate kai gate failed");
    require(metrics.expert_tie_ratio <= 0.60f, "tie-rate expert gate failed");
    require(metrics.equal_fixed_alt_tie_ratio <= 0.20f, "tie-rate equal-fixed gate failed");
    require(metrics.equal_fixed_serve_right_tie_ratio <= 0.20f, "tie-rate fixed-serve-right gate failed");
    require(metrics.equal_fixed_serve_left_tie_ratio <= 0.20f, "tie-rate fixed-serve-left gate failed");
}

void test_competence_ladder_and_side_bias() {
    const LadderMetrics metrics = collect_ladder_metrics();
    require_skill_order_gate(metrics);
    require_side_neutrality_gate(metrics);
    require_rally_band_gate(metrics);
    require_tie_rate_gate(metrics);
}

}  // namespace

extern "C" int glfwGetKey(GLFWwindow* /*window*/, const int /*key*/) {
    return GLFW_RELEASE;
}

int main() {
    test_competence_ladder_and_side_bias();
    return 0;
}
