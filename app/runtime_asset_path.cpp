#include "runtime_asset_path.hpp"

#include <cstdlib>
#include <system_error>
#include <vector>

#if defined(__linux__)
#include <array>
#include <unistd.h>
#endif

#ifndef WHACKER_SOURCE_DIR
#define WHACKER_SOURCE_DIR "."
#endif

namespace whacker::app {

namespace {

constexpr const char* kAssetRootEnv = "WHACKER_ASSET_ROOT";

std::filesystem::path normalized_path(const std::filesystem::path& path) {
    std::error_code ignored;
    const std::filesystem::path normalized = std::filesystem::weakly_canonical(path, ignored);
    return ignored ? path.lexically_normal() : normalized;
}

void add_candidate(std::vector<std::filesystem::path>& candidates, const std::filesystem::path& candidate) {
    if (candidate.empty()) {
        return;
    }
    const std::filesystem::path normalized = normalized_path(candidate);
    for (const std::filesystem::path& existing : candidates) {
        if (existing == normalized) {
            return;
        }
    }
    candidates.push_back(normalized);
}

std::filesystem::path executable_directory() {
#if defined(__linux__)
    std::array<char, 4096> buffer {};
    const ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1u);
    if (length <= 0) {
        return {};
    }
    buffer[static_cast<std::size_t>(length)] = '\0';
    return std::filesystem::path(buffer.data()).parent_path();
#else
    return {};
#endif
}

std::vector<std::filesystem::path> deployment_root_candidates() {
    std::vector<std::filesystem::path> candidates {};
    std::error_code ignored;
    add_candidate(candidates, std::filesystem::current_path(ignored));
    add_candidate(candidates, executable_directory());
    return candidates;
}

bool has_story_tree(const std::filesystem::path& root) {
    std::error_code ignored;
    return std::filesystem::is_directory(root / "story", ignored);
}

std::filesystem::path first_matching_deployment_path(
    const std::vector<std::filesystem::path>& candidates,
    const std::filesystem::path& relative_path) {
    for (const std::filesystem::path& root : candidates) {
        const std::filesystem::path candidate = root / relative_path;
        std::error_code ignored;
        if (std::filesystem::exists(candidate, ignored)) {
            return candidate;
        }
    }

    for (const std::filesystem::path& root : candidates) {
        if (has_story_tree(root)) {
            return root / relative_path;
        }
    }
    return {};
}

}  // namespace

std::filesystem::path runtime_asset_path(const std::filesystem::path& relative_path) {
    if (relative_path.empty() || relative_path.is_absolute()) {
        return relative_path;
    }

    const std::filesystem::path normalized_relative = relative_path.lexically_normal();
    if (const char* explicit_root = std::getenv(kAssetRootEnv)) {
        const std::filesystem::path root = normalized_path(explicit_root);
        if (!root.empty()) {
            const std::filesystem::path candidate = root / normalized_relative;
            std::error_code ignored;
            if (std::filesystem::exists(candidate, ignored)) {
                return candidate;
            }
            return candidate;
        }
    }

    const std::filesystem::path deployed_path =
        first_matching_deployment_path(deployment_root_candidates(), normalized_relative);
    if (!deployed_path.empty()) {
        return deployed_path;
    }

    const std::filesystem::path source_root = normalized_path(std::filesystem::path(WHACKER_SOURCE_DIR));
    if (!source_root.empty()) {
        const std::filesystem::path candidate = source_root / normalized_relative;
        std::error_code ignored;
        if (std::filesystem::exists(candidate, ignored)) {
            return candidate;
        }
        return candidate;
    }
    return normalized_relative;
}

}  // namespace whacker::app
