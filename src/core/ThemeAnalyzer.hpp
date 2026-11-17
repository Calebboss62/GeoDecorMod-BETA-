#pragma once

#include "LevelString.hpp"

#include <optional>
#include <string>
#include <vector>

namespace geodecor {

struct ThemeProfile {
    std::optional<RGB> background;
    std::optional<RGB> ground;
    std::optional<RGB> primary;
    std::optional<RGB> secondary;
    int backgroundTexture = -1;
    int groundTexture = -1;
    std::size_t objectCount = 0;
    double width = 0.0;
    double height = 0.0;
    double objectsPer1000Units = 0.0;
    double averageSaturation = 0.0;
    double averageLuminance = 0.0;
    std::string visualStyle = "unknown";
    std::string density = "unknown";
    std::string atmosphere = "unknown";
    std::string glowSignal = "not inferable from level strings alone";
    std::vector<int> mostUsedChannels;
    std::size_t explicitTriggerColorCount = 0;
    std::size_t glowDisabledObjectCount = 0;
};

ThemeProfile analyzeTheme(const LevelDocument& level);
ThemeProfile mixThemes(const ThemeProfile& a, const ThemeProfile& b, double blend);

} // namespace geodecor
