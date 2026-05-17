#include "story_save.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

#include "text_utils.hpp"
#include "progression/skills.hpp"
#include "progression/tags.hpp"
#include "story_script_catalog.hpp"
#include "story_skill_limits.hpp"

namespace {

std::vector<std::string> story_save_paths() {
    std::vector<std::string> paths;
    paths.emplace_back("saves/career_save.json");
    paths.emplace_back("../saves/career_save.json");
#ifdef WHACKER_SOURCE_DIR
    paths.emplace_back(std::string(WHACKER_SOURCE_DIR) + "/saves/career_save.json");
#endif
    return paths;
}

bool parse_float_value(const std::string& text, float& out_value) {
    char* end = nullptr;
    const float value = std::strtof(text.c_str(), &end);
    if (end == text.c_str()) {
        return false;
    }
    while (*end != '\0' && std::isspace(static_cast<unsigned char>(*end)) != 0) {
        ++end;
    }
    if (*end != '\0') {
        return false;
    }
    if (!std::isfinite(value)) {
        return false;
    }
    out_value = value;
    return true;
}

std::string primary_story_save_path() {
    const std::vector<std::string> paths = story_save_paths();
    return paths.empty() ? std::string("saves/career_save.json") : paths.front();
}

std::string locate_story_save_path() {
    for (const std::string& path : story_save_paths()) {
        std::ifstream input(path);
        if (input.is_open()) {
            return path;
        }
    }
    return std::string();
}

std::filesystem::path temporary_story_save_path(const std::filesystem::path& path) {
    std::filesystem::path temp_path = path;
    temp_path += ".tmp";
    return temp_path;
}

bool replace_story_save_atomically(
    const std::filesystem::path& temp_path,
    const std::filesystem::path& final_path,
    std::string* error_message) {
    std::error_code fs_error;
    std::filesystem::rename(temp_path, final_path, fs_error);
    if (!fs_error) {
        return true;
    }

#if defined(_WIN32)
    std::error_code remove_error;
    (void)std::filesystem::remove(final_path, remove_error);
    fs_error.clear();
    std::filesystem::rename(temp_path, final_path, fs_error);
    if (!fs_error) {
        return true;
    }
#endif

    if (error_message != nullptr) {
        *error_message =
            "Failed to atomically replace story save '" + final_path.string() + "': " + fs_error.message();
    }
    std::error_code cleanup_error;
    (void)std::filesystem::remove(temp_path, cleanup_error);
    return false;
}

std::string escape_json_string(const std::string& raw) {
    std::string escaped;
    escaped.reserve(raw.size() + 4);
    for (const char ch : raw) {
        if (ch == '\\' || ch == '"') {
            escaped.push_back('\\');
            escaped.push_back(ch);
            continue;
        }
        escaped.push_back(ch);
    }
    return escaped;
}

int clamp_tag_value(const int value, const int max_value) {
    if (value < -1) {
        return -1;
    }
    return std::min(value, max_value);
}

bool parse_json_string_value(const std::string& raw, std::string& out_value) {
    const std::string value = whacker::app::trim_copy(raw);
    if (value.size() < 2 || value.front() != '"' || value.back() != '"') {
        return false;
    }
    std::string decoded;
    decoded.reserve(value.size() - 2);
    bool escaping = false;
    for (std::size_t i = 1; i + 1 < value.size(); ++i) {
        const char ch = value[i];
        if (escaping) {
            decoded.push_back(ch);
            escaping = false;
            continue;
        }
        if (ch == '\\') {
            escaping = true;
            continue;
        }
        decoded.push_back(ch);
    }
    if (escaping) {
        return false;
    }
    out_value = decoded;
    return true;
}

}  // namespace

namespace whacker::app {

std::string sanitize_player_name(const std::string& raw_name) {
    std::string cleaned;
    cleaned.reserve(std::min<std::size_t>(raw_name.size(), 16u));
    for (const char ch : raw_name) {
        const unsigned char uch = static_cast<unsigned char>(ch);
        if (std::isalnum(uch) == 0 && ch != ' ' && ch != '-' && ch != '_') {
            continue;
        }
        if (cleaned.size() >= 16u) {
            break;
        }
        cleaned.push_back(static_cast<char>(std::toupper(uch)));
    }
    cleaned = trim_copy(cleaned);
    if (cleaned.empty()) {
        cleaned = "PLAYER";
    }
    return cleaned;
}

void reset_story_career(StoryCareerData& career) {
    career = StoryCareerData {};
    career.player_skills = kStoryPlayerStarterSkills;
    career.player_skill_caps = kStoryPlayerStarterSkills;
    normalize_story_player_skill_progress(career.player_skills, career.player_skill_caps);
}

bool story_save_exists() {
    for (const std::string& path : story_save_paths()) {
        std::ifstream input(path);
        if (input.is_open()) {
            return true;
        }
    }
    return false;
}

bool save_story_career(const StoryCareerData& career_in, std::string* error_message) {
    StoryCareerData career = career_in;
    normalize_story_player_skill_progress(career.player_skills, career.player_skill_caps);
    career.week = std::max(1, career.week);
    career.player_name = sanitize_player_name(career.player_name);
    career.training_used = std::max(0, career.training_used);
    career.training_matches_played = std::max(0, career.training_matches_played);
    career.reactivity.training_used_last_week = std::max(0, career.reactivity.training_used_last_week);
    career.crew_affinity.grind_systems = std::max(0, career.crew_affinity.grind_systems);
    career.crew_affinity.heart_social = std::max(0, career.crew_affinity.heart_social);
    career.crew_affinity.chaos_talent = std::max(0, career.crew_affinity.chaos_talent);
    career.reactivity.last_training_tag_1 = clamp_tag_value(
        career.reactivity.last_training_tag_1,
        static_cast<int>(whacker::progression::TrainingTag::Reckless));
    career.reactivity.last_training_tag_2 = clamp_tag_value(
        career.reactivity.last_training_tag_2,
        static_cast<int>(whacker::progression::TrainingTag::Reckless));
    career.reactivity.last_official_tag_1 = clamp_tag_value(
        career.reactivity.last_official_tag_1,
        static_cast<int>(whacker::progression::OfficialTag::NarrowDefeat));
    career.reactivity.last_official_tag_2 = clamp_tag_value(
        career.reactivity.last_official_tag_2,
        static_cast<int>(whacker::progression::OfficialTag::NarrowDefeat));
    career.reactivity.last_official_tag_3 = clamp_tag_value(
        career.reactivity.last_official_tag_3,
        static_cast<int>(whacker::progression::OfficialTag::NarrowDefeat));
    career.reputation.rating = std::max(100.0f, career.reputation.rating);
    if (career.joined_club && !career.story_completed) {
        (void)story_graph_initialize_career_node(career);
    }
    if (!career.joined_club) {
        career.progression_node_id.clear();
        career.story_completed = false;
    }

    const std::filesystem::path fs_path(primary_story_save_path());
    const std::filesystem::path temp_path = temporary_story_save_path(fs_path);
    std::error_code fs_error;
    if (fs_path.has_parent_path()) {
        std::filesystem::create_directories(fs_path.parent_path(), fs_error);
        if (fs_error) {
            if (error_message != nullptr) {
                *error_message = "Failed to create story save directory: " + fs_error.message();
            }
            return false;
        }
    }

    {
        std::error_code cleanup_error;
        (void)std::filesystem::remove(temp_path, cleanup_error);
    }

    std::ofstream output(temp_path, std::ios::trunc);
    if (!output.is_open()) {
        if (error_message != nullptr) {
            *error_message = "Failed to open story save temp file for write: " + temp_path.string();
        }
        return false;
    }

    output << "{\n";
    output << "  \"version\": " << career.version << ",\n";
    output << "  \"week\": " << career.week << ",\n";
    output << "  \"progression_node_id\": \"" << escape_json_string(career.progression_node_id) << "\",\n";
    output << "  \"story_completed\": " << (career.story_completed ? 1 : 0) << ",\n";
    output << "  \"player_name\": \"" << escape_json_string(career.player_name) << "\",\n";
    output << "  \"prefers_right_side\": " << (career.prefers_right_side ? 1 : 0) << ",\n";
    output << "  \"joined_club\": " << (career.joined_club ? 1 : 0) << ",\n";
    output << "  \"training_used\": " << career.training_used << ",\n";
    output << "  \"official_completed\": " << (career.official_completed ? 1 : 0) << ",\n";
    output << "  \"skill_edge\": " << career.player_skills.edge << ",\n";
    output << "  \"skill_power\": " << career.player_skills.power << ",\n";
    output << "  \"skill_spin\": " << career.player_skills.spin_inject << ",\n";
    output << "  \"skill_cap_edge\": " << career.player_skill_caps.edge << ",\n";
    output << "  \"skill_cap_power\": " << career.player_skill_caps.power << ",\n";
    output << "  \"skill_cap_spin\": " << career.player_skill_caps.spin_inject << ",\n";
    output << "  \"rating\": " << career.reputation.rating << ",\n";
    output << "  \"official_wins\": " << career.official_wins << ",\n";
    output << "  \"official_losses\": " << career.official_losses << ",\n";
    output << "  \"training_matches_played\": " << career.training_matches_played << ",\n";
    output << "  \"official_forfeits_total\": " << career.official_forfeits_total << ",\n";
    output << "  \"official_forfeit_streak\": " << career.official_forfeit_streak << ",\n";
    output << "  \"crew_affinity_grind_systems\": " << career.crew_affinity.grind_systems << ",\n";
    output << "  \"crew_affinity_heart_social\": " << career.crew_affinity.heart_social << ",\n";
    output << "  \"crew_affinity_chaos_talent\": " << career.crew_affinity.chaos_talent << ",\n";
    output << "  \"training_used_last_week\": " << career.reactivity.training_used_last_week << ",\n";
    output << "  \"last_training_tag_1\": " << career.reactivity.last_training_tag_1 << ",\n";
    output << "  \"last_training_tag_2\": " << career.reactivity.last_training_tag_2 << ",\n";
    output << "  \"last_official_tag_1\": " << career.reactivity.last_official_tag_1 << ",\n";
    output << "  \"last_official_tag_2\": " << career.reactivity.last_official_tag_2 << ",\n";
    output << "  \"last_official_tag_3\": " << career.reactivity.last_official_tag_3 << ",\n";
    output << "  \"onboarding_step\": " << static_cast<int>(career.onboarding_step) << ",\n";
    output << "  \"onboarding_style_hint\": " << static_cast<int>(career.onboarding_style_hint) << ",\n";
    output << "  \"onboarding_performance_hint\": " << static_cast<int>(career.onboarding_performance_hint) << ",\n";
    output << "  \"onboarding_aya_feedback_available\": " << (career.onboarding_aya_feedback_available ? 1 : 0) << ",\n";
    output << "  \"onboarding_aya_feedback_from_loss\": " << (career.onboarding_aya_feedback_from_loss ? 1 : 0) << ",\n";
    output << "  \"onboarding_aya_feedback_hint\": " << static_cast<int>(career.onboarding_aya_feedback_hint) << ",\n";
    output << "  \"onboarding_aya_forfeited\": " << (career.onboarding_aya_forfeited ? 1 : 0) << ",\n";
    output << "  \"tix_1967_seen\": " << (career.tix_1967_seen ? 1 : 0) << ",\n";
    output << "  \"tix_1967_player_won\": " << (career.tix_1967_player_won ? 1 : 0) << ",\n";
    output << "  \"tix_1967_score_for\": " << career.tix_1967_score_for << ",\n";
    output << "  \"tix_1967_score_against\": " << career.tix_1967_score_against << ",\n";
    output << "  \"tix_midweek_scene_seen\": " << (career.tix_midweek_scene_seen ? 1 : 0) << ",\n";
    output << "  \"tix_lunch_match_accepted\": " << (career.tix_lunch_match_accepted ? 1 : 0) << ",\n";
    output << "  \"tix_lunch_match_declined\": " << (career.tix_lunch_match_declined ? 1 : 0) << ",\n";
    output << "  \"tix_lunch_match_completed\": " << (career.tix_lunch_match_completed ? 1 : 0) << "\n";
    output << "}\n";
    output.flush();
    output.close();
    if (!output.good()) {
        if (error_message != nullptr) {
            *error_message = "Failed while writing story save temp file: " + temp_path.string();
        }
        std::error_code cleanup_error;
        (void)std::filesystem::remove(temp_path, cleanup_error);
        return false;
    }
    if (!replace_story_save_atomically(temp_path, fs_path, error_message)) {
        return false;
    }

    if (error_message != nullptr) {
        error_message->clear();
    }
    return true;
}

bool load_story_career(StoryCareerData& out_career, std::string* error_message) {
    const std::string path = locate_story_save_path();
    if (path.empty()) {
        if (error_message != nullptr) {
            *error_message = "No story save file found.";
        }
        return false;
    }

    std::ifstream input(path);
    if (!input.is_open()) {
        if (error_message != nullptr) {
            *error_message = "Failed to open story save for read: " + path;
        }
        return false;
    }

    StoryCareerData loaded {};
    reset_story_career(loaded);
    bool has_skill_cap_edge = false;
    bool has_skill_cap_power = false;
    bool has_skill_cap_spin = false;

    std::string line;
    while (std::getline(input, line)) {
        const std::string clean = trim_copy(line);
        if (clean.empty() || clean == "{" || clean == "}") {
            continue;
        }

        const std::size_t key_begin = clean.find('"');
        if (key_begin == std::string::npos) {
            continue;
        }
        const std::size_t key_end = clean.find('"', key_begin + 1);
        if (key_end == std::string::npos) {
            continue;
        }
        const std::string key = clean.substr(key_begin + 1, key_end - key_begin - 1);
        const std::size_t colon = clean.find(':', key_end + 1);
        if (colon == std::string::npos) {
            continue;
        }

        std::string value_text = trim_copy(clean.substr(colon + 1));
        if (!value_text.empty() && value_text.back() == ',') {
            value_text.pop_back();
            value_text = trim_copy(value_text);
        }

        if (key == "player_name") {
            std::string parsed_name;
            if (parse_json_string_value(value_text, parsed_name)) {
                loaded.player_name = parsed_name;
            }
            continue;
        }
        if (key == "progression_node_id") {
            std::string node_id;
            if (parse_json_string_value(value_text, node_id)) {
                loaded.progression_node_id = node_id;
            }
            continue;
        }

        float parsed = 0.0f;
        if (!parse_float_value(value_text, parsed)) {
            continue;
        }

        if (key == "version") {
            loaded.version = static_cast<int>(std::lround(parsed));
        } else if (key == "week") {
            loaded.week = static_cast<int>(std::lround(parsed));
        } else if (key == "story_completed") {
            loaded.story_completed = parsed >= 0.5f;
        } else if (key == "prefers_right_side") {
            loaded.prefers_right_side = parsed >= 0.5f;
        } else if (key == "joined_club") {
            loaded.joined_club = parsed >= 0.5f;
        } else if (key == "training_used") {
            loaded.training_used = static_cast<int>(std::lround(parsed));
        } else if (key == "official_completed") {
            loaded.official_completed = parsed >= 0.5f;
        } else if (key == "skill_edge") {
            loaded.player_skills.edge = parsed;
        } else if (key == "skill_power") {
            loaded.player_skills.power = parsed;
        } else if (key == "skill_spin") {
            loaded.player_skills.spin_inject = parsed;
        } else if (key == "skill_cap_edge") {
            loaded.player_skill_caps.edge = parsed;
            has_skill_cap_edge = true;
        } else if (key == "skill_cap_power") {
            loaded.player_skill_caps.power = parsed;
            has_skill_cap_power = true;
        } else if (key == "skill_cap_spin") {
            loaded.player_skill_caps.spin_inject = parsed;
            has_skill_cap_spin = true;
        } else if (key == "rating") {
            loaded.reputation.rating = parsed;
        } else if (key == "official_wins") {
            loaded.official_wins = static_cast<int>(std::lround(parsed));
        } else if (key == "official_losses") {
            loaded.official_losses = static_cast<int>(std::lround(parsed));
        } else if (key == "training_matches_played") {
            loaded.training_matches_played = static_cast<int>(std::lround(parsed));
        } else if (key == "official_forfeits_total") {
            loaded.official_forfeits_total = static_cast<int>(std::lround(parsed));
        } else if (key == "official_forfeit_streak") {
            loaded.official_forfeit_streak = static_cast<int>(std::lround(parsed));
        } else if (key == "crew_affinity_grind_systems") {
            loaded.crew_affinity.grind_systems = static_cast<int>(std::lround(parsed));
        } else if (key == "crew_affinity_heart_social") {
            loaded.crew_affinity.heart_social = static_cast<int>(std::lround(parsed));
        } else if (key == "crew_affinity_chaos_talent") {
            loaded.crew_affinity.chaos_talent = static_cast<int>(std::lround(parsed));
        } else if (key == "training_used_last_week") {
            loaded.reactivity.training_used_last_week = static_cast<int>(std::lround(parsed));
        } else if (key == "last_training_tag_1") {
            loaded.reactivity.last_training_tag_1 = static_cast<int>(std::lround(parsed));
        } else if (key == "last_training_tag_2") {
            loaded.reactivity.last_training_tag_2 = static_cast<int>(std::lround(parsed));
        } else if (key == "last_official_tag_1") {
            loaded.reactivity.last_official_tag_1 = static_cast<int>(std::lround(parsed));
        } else if (key == "last_official_tag_2") {
            loaded.reactivity.last_official_tag_2 = static_cast<int>(std::lround(parsed));
        } else if (key == "last_official_tag_3") {
            loaded.reactivity.last_official_tag_3 = static_cast<int>(std::lround(parsed));
        } else if (key == "onboarding_step") {
            loaded.onboarding_step = static_cast<StoryOnboardingStep>(std::lround(parsed));
        } else if (key == "onboarding_style_hint") {
            loaded.onboarding_style_hint = static_cast<StoryIntroStyleHint>(std::lround(parsed));
        } else if (key == "onboarding_performance_hint") {
            loaded.onboarding_performance_hint = static_cast<StoryIntroPerformanceHint>(std::lround(parsed));
        } else if (key == "onboarding_aya_feedback_available") {
            loaded.onboarding_aya_feedback_available = parsed >= 0.5f;
        } else if (key == "onboarding_aya_feedback_from_loss") {
            loaded.onboarding_aya_feedback_from_loss = parsed >= 0.5f;
        } else if (key == "onboarding_aya_feedback_hint") {
            loaded.onboarding_aya_feedback_hint = static_cast<StoryIntroStyleHint>(std::lround(parsed));
        } else if (key == "onboarding_aya_forfeited") {
            loaded.onboarding_aya_forfeited = parsed >= 0.5f;
        } else if (key == "tix_1967_seen") {
            loaded.tix_1967_seen = parsed >= 0.5f;
        } else if (key == "tix_1967_player_won") {
            loaded.tix_1967_player_won = parsed >= 0.5f;
        } else if (key == "tix_1967_score_for") {
            loaded.tix_1967_score_for = static_cast<int>(std::lround(parsed));
        } else if (key == "tix_1967_score_against") {
            loaded.tix_1967_score_against = static_cast<int>(std::lround(parsed));
        } else if (key == "tix_midweek_scene_seen") {
            loaded.tix_midweek_scene_seen = parsed >= 0.5f;
        } else if (key == "tix_lunch_match_accepted") {
            loaded.tix_lunch_match_accepted = parsed >= 0.5f;
        } else if (key == "tix_lunch_match_declined") {
            loaded.tix_lunch_match_declined = parsed >= 0.5f;
        } else if (key == "tix_lunch_match_completed") {
            loaded.tix_lunch_match_completed = parsed >= 0.5f;
        }
    }

    loaded.week = std::max(1, loaded.week);
    loaded.player_name = sanitize_player_name(loaded.player_name);
    loaded.prefers_right_side = loaded.prefers_right_side ? true : false;
    loaded.joined_club = loaded.joined_club ? true : false;
    loaded.story_completed = loaded.story_completed ? true : false;
    loaded.training_used = std::max(0, loaded.training_used);
    loaded.official_completed = loaded.official_completed ? true : false;
    loaded.official_wins = std::max(0, loaded.official_wins);
    loaded.official_losses = std::max(0, loaded.official_losses);
    loaded.training_matches_played = std::max(0, loaded.training_matches_played);
    loaded.official_forfeits_total = std::max(0, loaded.official_forfeits_total);
    loaded.official_forfeit_streak = std::max(0, loaded.official_forfeit_streak);
    loaded.crew_affinity.grind_systems = std::max(0, loaded.crew_affinity.grind_systems);
    loaded.crew_affinity.heart_social = std::max(0, loaded.crew_affinity.heart_social);
    loaded.crew_affinity.chaos_talent = std::max(0, loaded.crew_affinity.chaos_talent);
    loaded.reactivity.training_used_last_week = std::max(0, loaded.reactivity.training_used_last_week);
    loaded.reactivity.last_training_tag_1 = clamp_tag_value(
        loaded.reactivity.last_training_tag_1,
        static_cast<int>(whacker::progression::TrainingTag::Reckless));
    loaded.reactivity.last_training_tag_2 = clamp_tag_value(
        loaded.reactivity.last_training_tag_2,
        static_cast<int>(whacker::progression::TrainingTag::Reckless));
    loaded.reactivity.last_official_tag_1 = clamp_tag_value(
        loaded.reactivity.last_official_tag_1,
        static_cast<int>(whacker::progression::OfficialTag::NarrowDefeat));
    loaded.reactivity.last_official_tag_2 = clamp_tag_value(
        loaded.reactivity.last_official_tag_2,
        static_cast<int>(whacker::progression::OfficialTag::NarrowDefeat));
    loaded.reactivity.last_official_tag_3 = clamp_tag_value(
        loaded.reactivity.last_official_tag_3,
        static_cast<int>(whacker::progression::OfficialTag::NarrowDefeat));
    {
        const int onboarding_step =
            std::clamp(
                static_cast<int>(loaded.onboarding_step),
                0,
                static_cast<int>(StoryOnboardingStep::PostTixLunchScene));
        loaded.onboarding_step = static_cast<StoryOnboardingStep>(onboarding_step);
    }
    {
        const int style_hint =
            std::clamp(static_cast<int>(loaded.onboarding_style_hint), 0, static_cast<int>(StoryIntroStyleHint::Spin));
        loaded.onboarding_style_hint = static_cast<StoryIntroStyleHint>(style_hint);
    }
    {
        const int perf_hint = std::clamp(
            static_cast<int>(loaded.onboarding_performance_hint),
            0,
            static_cast<int>(StoryIntroPerformanceHint::CloseLoss));
        loaded.onboarding_performance_hint = static_cast<StoryIntroPerformanceHint>(perf_hint);
    }
    {
        const int aya_hint = std::clamp(
            static_cast<int>(loaded.onboarding_aya_feedback_hint),
            0,
            static_cast<int>(StoryIntroStyleHint::Spin));
        loaded.onboarding_aya_feedback_hint = static_cast<StoryIntroStyleHint>(aya_hint);
    }
    if (!has_skill_cap_edge) {
        loaded.player_skill_caps.edge = loaded.player_skills.edge;
    }
    if (!has_skill_cap_power) {
        loaded.player_skill_caps.power = loaded.player_skills.power;
    }
    if (!has_skill_cap_spin) {
        loaded.player_skill_caps.spin_inject = loaded.player_skills.spin_inject;
    }
    loaded.tix_1967_seen = loaded.tix_1967_seen ? true : false;
    loaded.tix_1967_player_won = loaded.tix_1967_player_won ? true : false;
    loaded.tix_1967_score_for = std::max(0, loaded.tix_1967_score_for);
    loaded.tix_1967_score_against = std::max(0, loaded.tix_1967_score_against);
    loaded.tix_midweek_scene_seen = loaded.tix_midweek_scene_seen ? true : false;
    loaded.tix_lunch_match_accepted = loaded.tix_lunch_match_accepted ? true : false;
    loaded.tix_lunch_match_declined = loaded.tix_lunch_match_declined ? true : false;
    loaded.tix_lunch_match_completed = loaded.tix_lunch_match_completed ? true : false;
    if (loaded.tix_lunch_match_completed) {
        loaded.tix_lunch_match_declined = false;
        loaded.tix_lunch_match_accepted = true;
    }
    if (loaded.tix_lunch_match_declined) {
        loaded.tix_lunch_match_accepted = false;
    }
    if (loaded.tix_midweek_scene_seen &&
        !loaded.tix_lunch_match_declined &&
        !loaded.tix_lunch_match_completed) {
        loaded.tix_lunch_match_accepted = true;
    }
    if (loaded.tix_lunch_match_accepted) {
        loaded.tix_lunch_match_declined = false;
    }
    if (loaded.tix_lunch_match_accepted || loaded.tix_lunch_match_declined || loaded.tix_lunch_match_completed) {
        loaded.tix_midweek_scene_seen = true;
    }
    if (!loaded.joined_club) {
        loaded.progression_node_id.clear();
        loaded.story_completed = false;
    } else if (!loaded.story_completed) {
        (void)story_graph_initialize_career_node(loaded);
    }
    normalize_story_player_skill_progress(loaded.player_skills, loaded.player_skill_caps);
    loaded.reputation.rating = std::max(100.0f, loaded.reputation.rating);

    out_career = loaded;
    if (error_message != nullptr) {
        error_message->clear();
    }
    return true;
}

}  // namespace whacker::app
