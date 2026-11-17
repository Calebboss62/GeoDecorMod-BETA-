#pragma once

#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace geodecor {

struct RGB {
    int r = 0;
    int g = 0;
    int b = 0;
};

struct ColorChannel {
    int id = -1;
    RGB from;
    RGB to;
    double opacity = 1.0;
};

struct LevelObject {
    std::map<int, std::string> properties;

    std::optional<double> number(int key) const;
    std::optional<int> integer(int key) const;
};

struct LevelSettings {
    std::map<std::string, std::string> properties;
    std::vector<ColorChannel> colors;
    int backgroundTexture = -1;
    int groundTexture = -1;
};

struct LevelDocument {
    LevelSettings settings;
    std::vector<LevelObject> objects;
};

struct ParseResult {
    bool ok = false;
    LevelDocument document;
    std::size_t segment = 0;
    std::string error;
};

// Parses the decoded inner level string, not the Base64/zlib wrapper.
// The input is read-only and no original object is ever rewritten.
ParseResult parseInnerLevelString(const std::string& decoded);

} // namespace geodecor
