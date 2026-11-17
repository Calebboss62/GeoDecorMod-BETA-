#pragma once

#include "../core/LevelString.hpp"

#include <filesystem>
#include <string>

namespace geodecor {

struct GmdLoadResult {
    bool ok = false;
    LevelDocument document;
    std::string error;
};

// Imports .gmd, .lvl, or .gmd2 through HJfod's cross-platform GMD-API,
// then decodes the level's URL-safe Base64 + zlib string for the core parser.
GmdLoadResult loadGmdForAnalysis(const std::filesystem::path& path);

} // namespace geodecor
