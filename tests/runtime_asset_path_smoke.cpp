#include "runtime_asset_path.hpp"

#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>

namespace {

void require(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << "runtime_asset_path_smoke failed: " << message << "\n";
        std::exit(1);
    }
}

class ScopedCurrentPath {
public:
    explicit ScopedCurrentPath(const std::filesystem::path& path)
        : old_path_(std::filesystem::current_path()) {
        std::filesystem::current_path(path);
    }

    ~ScopedCurrentPath() {
        std::error_code ignored;
        std::filesystem::current_path(old_path_, ignored);
    }

    ScopedCurrentPath(const ScopedCurrentPath&) = delete;
    ScopedCurrentPath& operator=(const ScopedCurrentPath&) = delete;

private:
    std::filesystem::path old_path_ {};
};

class ScopedEnvVar {
public:
    ScopedEnvVar(const char* name, const std::filesystem::path& value)
        : name_(name) {
        if (const char* existing = std::getenv(name_)) {
            old_value_ = existing;
            had_value_ = true;
        }
        setenv(name_, value.string().c_str(), 1);
    }

    ~ScopedEnvVar() {
        if (had_value_) {
            setenv(name_, old_value_.c_str(), 1);
        } else {
            unsetenv(name_);
        }
    }

    ScopedEnvVar(const ScopedEnvVar&) = delete;
    ScopedEnvVar& operator=(const ScopedEnvVar&) = delete;

private:
    const char* name_ = nullptr;
    std::string old_value_ {};
    bool had_value_ = false;
};

class ScopedUnsetEnvVar {
public:
    explicit ScopedUnsetEnvVar(const char* name)
        : name_(name) {
        if (const char* existing = std::getenv(name_)) {
            old_value_ = existing;
            had_value_ = true;
        }
        unsetenv(name_);
    }

    ~ScopedUnsetEnvVar() {
        if (had_value_) {
            setenv(name_, old_value_.c_str(), 1);
        } else {
            unsetenv(name_);
        }
    }

    ScopedUnsetEnvVar(const ScopedUnsetEnvVar&) = delete;
    ScopedUnsetEnvVar& operator=(const ScopedUnsetEnvVar&) = delete;

private:
    const char* name_ = nullptr;
    std::string old_value_ {};
    bool had_value_ = false;
};

std::filesystem::path unique_temp_root(const char* suffix) {
    const std::filesystem::path root =
        std::filesystem::temp_directory_path() / (std::string("whacker_asset_path_") + suffix);
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
    std::filesystem::create_directories(root, ignored);
    return root;
}

void write_empty_file(const std::filesystem::path& path) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream file(path, std::ios::binary);
    file << "x";
}

void test_runtime_cwd_asset_root_wins_when_present() {
    ScopedUnsetEnvVar asset_root {"WHACKER_ASSET_ROOT"};
    ScopedCurrentPath cwd {std::filesystem::path(WHACKER_SOURCE_DIR)};
    const std::filesystem::path path = whacker::app::runtime_asset_path(
        std::filesystem::path("story") / "art" / "magical_star_wipe_frame_source.png");
    require(std::filesystem::exists(path), "asset should resolve from cwd story tree");
    require(path.lexically_normal().generic_string().find("story/art/magical_star_wipe_frame_source.png") !=
                std::string::npos,
        "resolved path keeps requested relative asset path");
}

void test_source_tree_fallback_when_cwd_has_no_story_tree() {
    ScopedUnsetEnvVar asset_root {"WHACKER_ASSET_ROOT"};
    ScopedCurrentPath cwd {std::filesystem::temp_directory_path()};
    const std::filesystem::path path = whacker::app::runtime_asset_path(
        std::filesystem::path("story") / "art" / "magical_star_wipe_frame_source.png");
    require(std::filesystem::exists(path), "asset should fall back to compile-time source tree");
    require(path.lexically_normal().generic_string().find("story/art/magical_star_wipe_frame_source.png") !=
                std::string::npos,
        "fallback path keeps requested relative asset path");
}

void test_deployed_story_tree_masks_source_fallback_when_asset_is_missing() {
    ScopedUnsetEnvVar asset_root {"WHACKER_ASSET_ROOT"};
    const std::filesystem::path root = unique_temp_root("missing_asset");
    std::filesystem::create_directories(root / "story");
    {
        ScopedCurrentPath cwd {root};
        const std::filesystem::path relative_path =
            std::filesystem::path("story") / "art" / "magical_star_wipe_frame_source.png";
        const std::filesystem::path path = whacker::app::runtime_asset_path(relative_path);

        require(path.lexically_normal() == (root / relative_path).lexically_normal(),
            "deployed story tree should win over source fallback even when the requested asset is missing");
    }
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
}

void test_explicit_asset_root_wins() {
    const std::filesystem::path root = unique_temp_root("explicit_root");
    const std::filesystem::path relative_path = std::filesystem::path("story") / "art" / "test_asset.png";
    write_empty_file(root / relative_path);
    {
        ScopedEnvVar asset_root {"WHACKER_ASSET_ROOT", root};
        ScopedCurrentPath cwd {std::filesystem::path(WHACKER_SOURCE_DIR)};
        const std::filesystem::path path = whacker::app::runtime_asset_path(relative_path);
        require(path.lexically_normal() == (root / relative_path).lexically_normal(),
            "WHACKER_ASSET_ROOT should override cwd and source roots");
    }
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
}

}  // namespace

int main() {
    test_runtime_cwd_asset_root_wins_when_present();
    test_source_tree_fallback_when_cwd_has_no_story_tree();
    test_deployed_story_tree_masks_source_fallback_when_asset_is_missing();
    test_explicit_asset_root_wins();
    return 0;
}
