#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include "story_save.hpp"

namespace {

[[maybe_unused]] bool approx_equal(const float a, const float b, const float eps = 1.0e-4f) {
    return std::fabs(a - b) <= eps;
}

struct ScopedCurrentPath {
    explicit ScopedCurrentPath(const std::filesystem::path& path)
        : old_path(std::filesystem::current_path()) {
        std::filesystem::current_path(path);
    }

    ~ScopedCurrentPath() {
        std::error_code ignored;
        std::filesystem::current_path(old_path, ignored);
    }

    std::filesystem::path old_path;
};

void write_story_save_file(const std::filesystem::path& root, const std::string& contents) {
    std::filesystem::create_directories(root / "saves");
    std::ofstream output(root / "saves" / "career_save.json");
    assert(output.is_open());
    output << contents;
}

std::string read_story_save_file(const std::filesystem::path& root) {
    std::ifstream input(root / "saves" / "career_save.json");
    assert(input.is_open());
    return std::string((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
}

std::filesystem::path prepare_story_save_test_root(const char* name) {
    const std::filesystem::path root = std::filesystem::temp_directory_path() / name;
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
    std::filesystem::create_directories(root);
    return root;
}

void cleanup_story_save_test_root(const std::filesystem::path& root) {
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
}

void test_reset_story_career_initializes_skills_and_caps_to_starter_values() {
    whacker::app::StoryCareerData career {};
    career.player_skills = {.edge = 0.70f, .power = 0.50f, .spin_inject = 0.30f};
    career.player_skill_caps = {.edge = 0.20f, .power = 0.10f, .spin_inject = 0.05f};

    whacker::app::reset_story_career(career);

    assert(approx_equal(career.player_skills.edge, 0.10f));
    assert(approx_equal(career.player_skills.power, 0.10f));
    assert(approx_equal(career.player_skills.spin_inject, 0.10f));
    assert(approx_equal(career.player_skill_caps.edge, 0.10f));
    assert(approx_equal(career.player_skill_caps.power, 0.10f));
    assert(approx_equal(career.player_skill_caps.spin_inject, 0.10f));
}

void test_load_story_career_without_cap_fields_migrates_caps_from_current_values() {
    const std::filesystem::path root = prepare_story_save_test_root("whacker_story_save_smoke_missing_caps");
    {
        ScopedCurrentPath scoped_path(root);
        write_story_save_file(
            root,
            "{\n"
            "  \"version\": 1,\n"
            "  \"week\": 3,\n"
            "  \"player_name\": \"alex\",\n"
            "  \"skill_edge\": 0.42,\n"
            "  \"skill_power\": 0.33,\n"
            "  \"skill_spin\": 0.21\n"
            "}\n");
        whacker::app::StoryCareerData loaded {};
        std::string error_message;
        assert(whacker::app::load_story_career(loaded, &error_message));
        assert(error_message.empty());
        assert(approx_equal(loaded.player_skills.edge, 0.42f));
        assert(approx_equal(loaded.player_skills.power, 0.33f));
        assert(approx_equal(loaded.player_skills.spin_inject, 0.21f));
        assert(approx_equal(loaded.player_skill_caps.edge, loaded.player_skills.edge));
        assert(approx_equal(loaded.player_skill_caps.power, loaded.player_skills.power));
        assert(approx_equal(loaded.player_skill_caps.spin_inject, loaded.player_skills.spin_inject));
    }
    cleanup_story_save_test_root(root);
}

void test_load_story_career_with_cap_fields_clamps_current_to_caps() {
    const std::filesystem::path root = prepare_story_save_test_root("whacker_story_save_smoke_with_caps");
    {
        ScopedCurrentPath scoped_path(root);
        write_story_save_file(
            root,
            "{\n"
            "  \"version\": 1,\n"
            "  \"week\": 4,\n"
            "  \"player_name\": \"player\",\n"
            "  \"skill_edge\": 0.60,\n"
            "  \"skill_power\": 0.40,\n"
            "  \"skill_spin\": 0.20,\n"
            "  \"skill_cap_edge\": 0.30,\n"
            "  \"skill_cap_power\": 0.20,\n"
            "  \"skill_cap_spin\": 0.10\n"
            "}\n");
        whacker::app::StoryCareerData loaded {};
        std::string error_message;
        assert(whacker::app::load_story_career(loaded, &error_message));
        assert(error_message.empty());
        assert(approx_equal(loaded.player_skill_caps.edge, 0.30f));
        assert(approx_equal(loaded.player_skill_caps.power, 0.20f));
        assert(approx_equal(loaded.player_skill_caps.spin_inject, 0.10f));
        assert(approx_equal(loaded.player_skills.edge, 0.30f));
        assert(approx_equal(loaded.player_skills.power, 0.20f));
        assert(approx_equal(loaded.player_skills.spin_inject, 0.10f));
    }
    cleanup_story_save_test_root(root);
}

void test_load_story_career_joined_club_migrates_missing_progression_node() {
    const std::filesystem::path root = prepare_story_save_test_root("whacker_story_save_smoke_missing_node");
    {
        ScopedCurrentPath scoped_path(root);
        write_story_save_file(
            root,
            "{\n"
            "  \"version\": 1,\n"
            "  \"week\": 4,\n"
            "  \"joined_club\": 1,\n"
            "  \"player_name\": \"player\"\n"
            "}\n");
        whacker::app::StoryCareerData loaded {};
        std::string error_message;
        assert(whacker::app::load_story_career(loaded, &error_message));
        assert(error_message.empty());
        assert(loaded.joined_club);
        assert(loaded.progression_node_id == "club_week_01");
    }
    cleanup_story_save_test_root(root);
}

void test_save_story_career_persists_skill_cap_fields() {
    const std::filesystem::path root = prepare_story_save_test_root("whacker_story_save_smoke_round_trip");
    {
        ScopedCurrentPath scoped_path(root);
        whacker::app::StoryCareerData career {};
        career.player_name = "player";
        career.joined_club = true;
        career.progression_node_id = "club_week_01";
        career.player_skills = {.edge = 0.20f, .power = 0.15f, .spin_inject = 0.05f};
        career.player_skill_caps = {.edge = 0.30f, .power = 0.20f, .spin_inject = 0.10f};
        career.tix_1967_seen = true;
        career.tix_1967_player_won = true;
        career.tix_1967_score_for = 11;
        career.tix_1967_score_against = 9;
        career.tix_midweek_scene_seen = true;
        career.tix_lunch_match_accepted = true;
        career.tix_lunch_match_declined = false;
        career.tix_lunch_match_completed = true;
        career.training_used = 3;
        career.reactivity.training_used_last_week = 2;
        std::string save_error;
        assert(whacker::app::save_story_career(career, &save_error));
        assert(save_error.empty());

        const std::string file_contents = read_story_save_file(root);
        assert(file_contents.find("\"skill_cap_edge\"") != std::string::npos);
        assert(file_contents.find("\"skill_cap_power\"") != std::string::npos);
        assert(file_contents.find("\"skill_cap_spin\"") != std::string::npos);
        assert(file_contents.find("\"progression_node_id\"") != std::string::npos);
        assert(file_contents.find("\"tix_1967_seen\"") != std::string::npos);
        assert(file_contents.find("\"tix_1967_player_won\"") != std::string::npos);
        assert(file_contents.find("\"tix_1967_score_for\"") != std::string::npos);
        assert(file_contents.find("\"tix_1967_score_against\"") != std::string::npos);
        assert(file_contents.find("\"tix_midweek_scene_seen\"") != std::string::npos);
        assert(file_contents.find("\"tix_lunch_match_accepted\"") != std::string::npos);
        assert(file_contents.find("\"tix_lunch_match_declined\"") != std::string::npos);
        assert(file_contents.find("\"tix_lunch_match_completed\"") != std::string::npos);
        assert(file_contents.find("\"training_used\"") != std::string::npos);
        assert(file_contents.find("\"training_used_last_week\"") != std::string::npos);

        whacker::app::StoryCareerData loaded {};
        std::string load_error;
        assert(whacker::app::load_story_career(loaded, &load_error));
        assert(load_error.empty());
        assert(approx_equal(loaded.player_skills.edge, 0.20f));
        assert(approx_equal(loaded.player_skills.power, 0.15f));
        assert(approx_equal(loaded.player_skills.spin_inject, 0.05f));
        assert(approx_equal(loaded.player_skill_caps.edge, 0.30f));
        assert(approx_equal(loaded.player_skill_caps.power, 0.20f));
        assert(approx_equal(loaded.player_skill_caps.spin_inject, 0.10f));
        assert(loaded.progression_node_id == "club_week_01");
        assert(loaded.tix_1967_seen);
        assert(loaded.tix_1967_player_won);
        assert(loaded.tix_1967_score_for == 11);
        assert(loaded.tix_1967_score_against == 9);
        assert(loaded.tix_midweek_scene_seen);
        assert(loaded.tix_lunch_match_accepted);
        assert(!loaded.tix_lunch_match_declined);
        assert(loaded.tix_lunch_match_completed);
        assert(loaded.training_used == 3);
        assert(loaded.reactivity.training_used_last_week == 2);
    }
    cleanup_story_save_test_root(root);
}

void test_save_story_career_atomic_replace_does_not_leave_temp_file() {
    const std::filesystem::path root = prepare_story_save_test_root("whacker_story_save_smoke_atomic_replace");
    {
        ScopedCurrentPath scoped_path(root);
        whacker::app::StoryCareerData career {};
        career.player_name = "player";
        career.joined_club = true;
        career.progression_node_id = "club_week_01";

        std::string save_error;
        assert(whacker::app::save_story_career(career, &save_error));
        assert(save_error.empty());
        assert(std::filesystem::exists(root / "saves" / "career_save.json"));
        assert(!std::filesystem::exists(root / "saves" / "career_save.json.tmp"));
    }
    cleanup_story_save_test_root(root);
}

void test_load_story_career_rejects_non_finite_numbers() {
    const std::filesystem::path root = prepare_story_save_test_root("whacker_story_save_smoke_non_finite");
    {
        ScopedCurrentPath scoped_path(root);
        write_story_save_file(
            root,
            "{\n"
            "  \"version\": nan,\n"
            "  \"week\": inf,\n"
            "  \"player_name\": \"player\",\n"
            "  \"skill_edge\": nan,\n"
            "  \"skill_power\": -inf,\n"
            "  \"skill_spin\": inf,\n"
            "  \"skill_cap_edge\": nan,\n"
            "  \"skill_cap_power\": inf,\n"
            "  \"skill_cap_spin\": -inf,\n"
            "  \"rating\": nan\n"
            "}\n");
        whacker::app::StoryCareerData loaded {};
        std::string error_message;
        assert(whacker::app::load_story_career(loaded, &error_message));
        assert(error_message.empty());
        assert(std::isfinite(loaded.player_skills.edge));
        assert(std::isfinite(loaded.player_skills.power));
        assert(std::isfinite(loaded.player_skills.spin_inject));
        assert(std::isfinite(loaded.player_skill_caps.edge));
        assert(std::isfinite(loaded.player_skill_caps.power));
        assert(std::isfinite(loaded.player_skill_caps.spin_inject));
        assert(std::isfinite(loaded.reputation.rating));
        assert(approx_equal(loaded.player_skills.edge, 0.10f));
        assert(approx_equal(loaded.player_skills.power, 0.10f));
        assert(approx_equal(loaded.player_skills.spin_inject, 0.10f));
    }
    cleanup_story_save_test_root(root);
}

}  // namespace

int main() {
    test_reset_story_career_initializes_skills_and_caps_to_starter_values();
    test_load_story_career_without_cap_fields_migrates_caps_from_current_values();
    test_load_story_career_with_cap_fields_clamps_current_to_caps();
    test_load_story_career_joined_club_migrates_missing_progression_node();
    test_save_story_career_persists_skill_cap_fields();
    test_save_story_career_atomic_replace_does_not_leave_temp_file();
    test_load_story_career_rejects_non_finite_numbers();
    return 0;
}
