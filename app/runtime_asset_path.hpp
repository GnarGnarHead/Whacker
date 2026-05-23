#pragma once

#include <filesystem>

namespace whacker::app {

// Resolve a game-root-relative runtime asset path against deployment roots first.
// The compile-time source tree is only a development fallback.
std::filesystem::path runtime_asset_path(const std::filesystem::path& relative_path);

}  // namespace whacker::app
