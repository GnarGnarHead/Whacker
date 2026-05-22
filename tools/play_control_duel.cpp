#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "play_control.hpp"

namespace {

namespace app = whacker::app;
namespace sim = whacker::sim;
namespace prog = whacker::progression;

constexpr float kSkillBudgetCap = prog::kSkillBudgetCap;

struct Options {
    prog::SkillState left_skills {.edge = 0.12f, .power = 0.12f, .spin_inject = 0.12f};
    prog::SkillState right_skills {.edge = 0.56f, .power = 0.56f, .spin_inject = 0.56f};
    int games = 20;
    int win_score = 11;
    int max_steps = 120000;
    bool swap_sides = true;
    bool alternate_serve = true;
    bool first_serve_to_right = true;
    bool trace_hash = false;
};

struct MatchResult {
    int left_score = 0;
    int right_score = 0;
};

float clampf(const float value, const float lo, const float hi) {
    return std::max(lo, std::min(value, hi));
}

bool parse_positive_int(const std::string& text, int& out) {
    try {
        std::size_t pos = 0;
        const int value = std::stoi(text, &pos);
        if (pos != text.size() || value <= 0) {
            return false;
        }
        out = value;
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_skill_triplet(const std::string& text, prog::SkillState& out) {
    std::stringstream ss(text);
    std::string token;
    std::array<float, 3> values {0.0f, 0.0f, 0.0f};
    std::size_t index = 0;
    while (std::getline(ss, token, ',')) {
        if (index >= values.size()) {
            return false;
        }
        try {
            std::size_t pos = 0;
            const float parsed = std::stof(token, &pos);
            if (pos != token.size()) {
                return false;
            }
            values[index++] = clampf(parsed, 0.0f, 1.0f);
        } catch (...) {
            return false;
        }
    }
    if (index != values.size()) {
        return false;
    }
    const float sum = values[0] + values[1] + values[2];
    if (sum > kSkillBudgetCap + 1.0e-5f) {
        return false;
    }
    out = prog::SkillState {.edge = values[0], .power = values[1], .spin_inject = values[2]};
    prog::clamp_skills(out);
    return true;
}

void print_usage() {
    std::cout
        << "Usage: play_control_duel [options]\n"
        << "Options:\n"
        << "  --left A,B,C         Left skills (edge,power,spin), default 0.12,0.12,0.12\n"
        << "  --right A,B,C        Right skills (edge,power,spin), default 0.56,0.56,0.56\n"
        << "                       Note: each skill in [0,1], total must be <= 1.70\n"
        << "  --games N            Number of games (default 20)\n"
        << "  --win N              Points to win a game (default 11)\n"
        << "  --steps N            Max sim steps per game (default 120000)\n"
        << "  --fixed-serve-right  Disable serve alternation; always open to right\n"
        << "  --fixed-serve-left   Disable serve alternation; always open to left\n"
        << "  --no-swap-sides      Disable side swapping between games\n"
        << "  --trace-hash         Print deterministic hash of game outcomes\n"
        << "  --help               Show this help\n";
}

bool parse_args(const int argc, char** argv, Options& options) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        auto require_value = [&](const char* flag) -> const char* {
            if ((i + 1) >= argc) {
                std::cerr << "Missing value for " << flag << "\n";
                return nullptr;
            }
            return argv[++i];
        };

        if (arg == "--help" || arg == "-h") {
            print_usage();
            return false;
        }
        if (arg == "--left") {
            const char* value = require_value("--left");
            if (value == nullptr || !parse_skill_triplet(value, options.left_skills)) {
                std::cerr << "Invalid --left triplet\n";
                return false;
            }
            continue;
        }
        if (arg == "--right") {
            const char* value = require_value("--right");
            if (value == nullptr || !parse_skill_triplet(value, options.right_skills)) {
                std::cerr << "Invalid --right triplet\n";
                return false;
            }
            continue;
        }
        if (arg == "--games") {
            const char* value = require_value("--games");
            if (value == nullptr || !parse_positive_int(value, options.games)) {
                std::cerr << "Invalid --games value\n";
                return false;
            }
            continue;
        }
        if (arg == "--win") {
            const char* value = require_value("--win");
            if (value == nullptr || !parse_positive_int(value, options.win_score)) {
                std::cerr << "Invalid --win value\n";
                return false;
            }
            continue;
        }
        if (arg == "--steps") {
            const char* value = require_value("--steps");
            if (value == nullptr || !parse_positive_int(value, options.max_steps)) {
                std::cerr << "Invalid --steps value\n";
                return false;
            }
            continue;
        }
        if (arg == "--fixed-serve-right") {
            options.alternate_serve = false;
            options.first_serve_to_right = true;
            continue;
        }
        if (arg == "--fixed-serve-left") {
            options.alternate_serve = false;
            options.first_serve_to_right = false;
            continue;
        }
        if (arg == "--no-swap-sides") {
            options.swap_sides = false;
            continue;
        }
        if (arg == "--trace-hash") {
            options.trace_hash = true;
            continue;
        }
        std::cerr << "Unknown argument: " << arg << "\n";
        return false;
    }
    return true;
}

MatchResult run_game(
    const prog::SkillState& left_skills,
    const prog::SkillState& right_skills,
    const bool opening_serve_to_right,
    const int win_score,
    const int max_steps) {
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

    for (int step = 0; step < max_steps; ++step) {
        app::update_targets_for_play(
            simulation,
            options,
            left_ai,
            right_ai,
            sim::kFixedDt,
            app::quick_match_control_plan(),
            app::InputSlotAxes {},
            nullptr);
        (void)simulation.step(sim::kFixedDt);
        const auto& state = simulation.state();
        const int score_gap = std::abs(state.left_score - state.right_score);
        if ((state.left_score >= win_score || state.right_score >= win_score) && score_gap >= 2) {
            break;
        }
    }

    const auto& state = simulation.state();
    return MatchResult {.left_score = state.left_score, .right_score = state.right_score};
}

float skill_total(const prog::SkillState& skills) {
    return skills.edge + skills.power + skills.spin_inject;
}

uint64_t hash_combine_u64(const uint64_t seed, const uint64_t value) {
    const uint64_t mixed = value + 0x9e3779b97f4a7c15ULL + (seed << 6U) + (seed >> 2U);
    return seed ^ mixed;
}

}  // namespace

int main(int argc, char** argv) {
    Options options {};
    if (!parse_args(argc, argv, options)) {
        return 1;
    }

    const float left_total = skill_total(options.left_skills);
    const float right_total = skill_total(options.right_skills);
    const bool left_is_stronger = left_total >= right_total;

    int stronger_wins = 0;
    int weaker_wins = 0;
    int left_wins = 0;
    int right_wins = 0;
    int ties = 0;
    int stronger_points = 0;
    int weaker_points = 0;
    int stronger_left_wins = 0;
    int stronger_left_losses = 0;
    int stronger_left_ties = 0;
    int stronger_right_wins = 0;
    int stronger_right_losses = 0;
    int stronger_right_ties = 0;
    uint64_t trace_hash = 0xCBF29CE484222325ULL;

    for (int game = 0; game < options.games; ++game) {
        bool stronger_on_left = left_is_stronger;
        if (options.swap_sides && (game % 2 == 1)) {
            stronger_on_left = !stronger_on_left;
        }

        const prog::SkillState stronger = left_is_stronger ? options.left_skills : options.right_skills;
        const prog::SkillState weaker = left_is_stronger ? options.right_skills : options.left_skills;
        const prog::SkillState left_skills = stronger_on_left ? stronger : weaker;
        const prog::SkillState right_skills = stronger_on_left ? weaker : stronger;
        // When side swapping is enabled every game, alternate serve on a 2-game cadence
        // so each starting side sees both opening-serve directions.
        const int serve_phase = options.swap_sides ? ((game / 2) % 2) : (game % 2);
        const bool opening_serve_to_right = options.alternate_serve
            ? ((serve_phase == 0) ? options.first_serve_to_right : !options.first_serve_to_right)
            : options.first_serve_to_right;

        const MatchResult result = run_game(
            left_skills,
            right_skills,
            opening_serve_to_right,
            options.win_score,
            options.max_steps);
        if (stronger_on_left) {
            stronger_points += result.left_score;
            weaker_points += result.right_score;
        } else {
            stronger_points += result.right_score;
            weaker_points += result.left_score;
        }
        if (result.left_score == result.right_score) {
            ++ties;
            if (stronger_on_left) {
                ++stronger_left_ties;
            } else {
                ++stronger_right_ties;
            }
            continue;
        }
        const bool left_won = result.left_score > result.right_score;
        if (left_won) {
            ++left_wins;
        } else {
            ++right_wins;
        }
        const bool stronger_won = stronger_on_left ? left_won : !left_won;
        if (stronger_won) {
            ++stronger_wins;
            if (stronger_on_left) {
                ++stronger_left_wins;
            } else {
                ++stronger_right_wins;
            }
        } else {
            ++weaker_wins;
            if (stronger_on_left) {
                ++stronger_left_losses;
            } else {
                ++stronger_right_losses;
            }
        }

        if (options.trace_hash) {
            trace_hash = hash_combine_u64(trace_hash, static_cast<uint64_t>(game));
            trace_hash = hash_combine_u64(trace_hash, stronger_on_left ? 1ULL : 2ULL);
            trace_hash = hash_combine_u64(trace_hash, static_cast<uint64_t>(static_cast<uint32_t>(result.left_score)));
            trace_hash = hash_combine_u64(trace_hash, static_cast<uint64_t>(static_cast<uint32_t>(result.right_score)));
        }
    }

    const int decisive_games = stronger_wins + weaker_wins;
    const float stronger_win_rate = decisive_games > 0
        ? static_cast<float>(stronger_wins) / static_cast<float>(decisive_games)
        : 0.0f;
    const float stronger_points_per_game =
        static_cast<float>(stronger_points) / static_cast<float>(std::max(options.games, 1));
    const float weaker_points_per_game =
        static_cast<float>(weaker_points) / static_cast<float>(std::max(options.games, 1));
    const int stronger_left_decisive = stronger_left_wins + stronger_left_losses;
    const int stronger_right_decisive = stronger_right_wins + stronger_right_losses;
    const float stronger_left_rate = stronger_left_decisive > 0
        ? static_cast<float>(stronger_left_wins) / static_cast<float>(stronger_left_decisive)
        : 0.0f;
    const float stronger_right_rate = stronger_right_decisive > 0
        ? static_cast<float>(stronger_right_wins) / static_cast<float>(stronger_right_decisive)
        : 0.0f;
    const bool equal_totals = std::fabs(left_total - right_total) <= 1.0e-5f;
    auto print_side_breakdown = [&](const char* prefix) {
        std::cout << prefix << " by starting side (left-start, right-start): ";
        if (stronger_left_decisive > 0) {
            std::cout << stronger_left_rate;
        } else {
            std::cout << "n/a";
        }
        std::cout << ", ";
        if (stronger_right_decisive > 0) {
            std::cout << stronger_right_rate;
        } else {
            std::cout << "n/a";
        }
        std::cout << "\n";
    };
    auto print_side_ties = [&](const char* prefix) {
        std::cout << prefix << " by starting side (left-start, right-start): ";
        if (stronger_left_decisive > 0 || stronger_left_ties > 0) {
            std::cout << stronger_left_ties;
        } else {
            std::cout << "n/a";
        }
        std::cout << ", ";
        if (stronger_right_decisive > 0 || stronger_right_ties > 0) {
            std::cout << stronger_right_ties;
        } else {
            std::cout << "n/a";
        }
        std::cout << "\n";
    };

    std::cout << std::fixed << std::setprecision(3);
    if (equal_totals) {
        const char* serve_mode = options.alternate_serve
            ? "alternating"
            : (options.first_serve_to_right ? "fixed-right" : "fixed-left");
        std::cout << "Profile A total: " << left_total << "  Profile B total: " << right_total << "\n";
        std::cout << "Profile A wins: " << stronger_wins << "  Profile B wins: " << weaker_wins << "  Ties: " << ties << "\n";
        std::cout << "Side wins (left, right): " << left_wins << ", " << right_wins << "\n";
        std::cout << "Profile A points: " << stronger_points << "  Profile B points: " << weaker_points << "\n";
        std::cout << "Points/game (profile A, profile B): " << stronger_points_per_game
                  << ", " << weaker_points_per_game << "\n";
        std::cout << "Profile A win rate (decisive games): " << stronger_win_rate << "\n";
        print_side_breakdown("Profile A win rate");
        print_side_ties("Profile A ties");
        std::cout << "Run context: swap_sides=" << (options.swap_sides ? "on" : "off")
                  << ", serve_mode=" << serve_mode << "\n";
        std::cout << "Note: aggregate Profile A/B totals are the neutral comparison; starting-side split is reported for parity diagnostics.\n";
    } else {
        std::cout << "Left total: " << left_total << "  Right total: " << right_total << "\n";
        std::cout << "Stronger wins: " << stronger_wins << "  Weaker wins: " << weaker_wins << "  Ties: " << ties << "\n";
        std::cout << "Stronger points: " << stronger_points << "  Weaker points: " << weaker_points << "\n";
        std::cout << "Points/game (stronger, weaker): " << stronger_points_per_game
                  << ", " << weaker_points_per_game << "\n";
        std::cout << "Stronger win rate (decisive games): " << stronger_win_rate << "\n";
        print_side_breakdown("Stronger win rate");
        print_side_ties("Stronger ties");
    }
    if (options.trace_hash) {
        std::cout << std::hex << std::showbase;
        std::cout << "Trace hash: " << trace_hash << "\n";
        std::cout << std::dec << std::noshowbase;
    }
    return 0;
}
