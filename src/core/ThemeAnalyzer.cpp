#include "ThemeAnalyzer.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>

namespace geodecor {
namespace {
RGB blendColor(const RGB& a, const RGB& b, double t) {
    const auto mix = [t](int x, int y) {
        return std::clamp(static_cast<int>(std::lround(x + (y - x) * t)), 0, 255);
    };
    return {mix(a.r, b.r), mix(a.g, b.g), mix(a.b, b.b)};
}

std::optional<RGB> blendOptional(const std::optional<RGB>& a,
                                 const std::optional<RGB>& b, double t) {
    if (a && b) return blendColor(*a, *b, t);
    if (b) return b;
    return a;
}

double luminance(const RGB& color) {
    const auto linear = [](int component) {
        const double c = component / 255.0;
        return c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * linear(color.r) + 0.7152 * linear(color.g) + 0.0722 * linear(color.b);
}

double saturation(const RGB& color) {
    const double r = color.r / 255.0;
    const double g = color.g / 255.0;
    const double b = color.b / 255.0;
    const double hi = std::max({r, g, b});
    const double lo = std::min({r, g, b});
    if (hi <= 0.0) return 0.0;
    return (hi - lo) / hi;
}

std::optional<RGB> findChannelColor(const std::vector<ColorChannel>& colors, int id) {
    const auto it = std::find_if(colors.begin(), colors.end(), [id](const auto& c) { return c.id == id; });
    if (it == colors.end()) return std::nullopt;
    return it->from;
}
} // namespace

ThemeProfile analyzeTheme(const LevelDocument& level) {
    ThemeProfile profile;
    profile.objectCount = level.objects.size();
    profile.backgroundTexture = level.settings.backgroundTexture;
    profile.groundTexture = level.settings.groundTexture;
    profile.background = findChannelColor(level.settings.colors, 1000);
    profile.ground = findChannelColor(level.settings.colors, 1001);

    std::map<int, std::size_t> channelUsage;
    std::vector<RGB> samples;
    for (const auto& color : level.settings.colors) {
        samples.push_back(color.from);
    }

    double minX = 0.0, maxX = 0.0, minY = 0.0, maxY = 0.0;
    bool havePosition = false;
    for (const auto& object : level.objects) {
        const auto x = object.number(2);
        const auto y = object.number(3);
        if (x && y) {
            if (!havePosition) {
                minX = maxX = *x;
                minY = maxY = *y;
                havePosition = true;
            } else {
                minX = std::min(minX, *x);
                maxX = std::max(maxX, *x);
                minY = std::min(minY, *y);
                maxY = std::max(maxY, *y);
            }
        }

        const auto legacy = object.integer(19);
        const auto main = object.integer(21);
        const auto secondary = object.integer(22);
        if (legacy && *legacy > 0 && *legacy < 1000) ++channelUsage[*legacy];
        else {
            if (main && *main > 0 && *main < 1000) ++channelUsage[*main];
            if (secondary && *secondary > 0 && *secondary < 1000) ++channelUsage[*secondary];
        }

        const auto r = object.integer(7);
        const auto g = object.integer(8);
        const auto b = object.integer(9);
        if (r && g && b && *r >= 0 && *r <= 255 && *g >= 0 && *g <= 255 && *b >= 0 && *b <= 255) {
            samples.push_back({*r, *g, *b});
            ++profile.explicitTriggerColorCount;
        }
        if (object.integer(96).value_or(0) == 1) ++profile.glowDisabledObjectCount;
    }

    if (havePosition) {
        profile.width = std::max(0.0, maxX - minX);
        profile.height = std::max(0.0, maxY - minY);
    }
    const double effectiveWidth = std::max(profile.width, 100.0);
    profile.objectsPer1000Units = profile.objectCount * 1000.0 / effectiveWidth;
    if (profile.objectsPer1000Units >= 90.0) profile.density = "very dense";
    else if (profile.objectsPer1000Units >= 35.0) profile.density = "dense";
    else if (profile.objectsPer1000Units >= 10.0) profile.density = "medium";
    else profile.density = "sparse";

    std::vector<std::pair<int, std::size_t>> ranked(channelUsage.begin(), channelUsage.end());
    std::sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) {
        if (a.second != b.second) return a.second > b.second;
        return a.first < b.first;
    });
    for (std::size_t i = 0; i < ranked.size() && i < 5; ++i) {
        if (ranked[i].second > 0) profile.mostUsedChannels.push_back(ranked[i].first);
    }
    if (!profile.mostUsedChannels.empty()) {
        profile.primary = findChannelColor(level.settings.colors, profile.mostUsedChannels[0]);
    }
    if (profile.mostUsedChannels.size() > 1) {
        profile.secondary = findChannelColor(level.settings.colors, profile.mostUsedChannels[1]);
    }

    if (!samples.empty()) {
        double totalSat = 0.0;
        double totalLum = 0.0;
        for (const auto& sample : samples) {
            totalSat += saturation(sample);
            totalLum += luminance(sample);
        }
        profile.averageSaturation = totalSat / samples.size();
        profile.averageLuminance = totalLum / samples.size();
        if (!profile.primary) profile.primary = samples.front();
        if (!profile.secondary && samples.size() > 1) profile.secondary = samples.back();

        if (profile.averageSaturation < 0.12) profile.visualStyle = "monochrome";
        else if (profile.averageLuminance < 0.28 && profile.averageSaturation > 0.45) profile.visualStyle = "dark neon";
        else if (profile.averageLuminance > 0.68 && profile.averageSaturation < 0.42) profile.visualStyle = "pastel";
        else if (profile.averageSaturation > 0.55) profile.visualStyle = "vivid / neon";
        else profile.visualStyle = "balanced";

        if (profile.averageLuminance < 0.28) profile.atmosphere = "dark";
        else if (profile.averageLuminance > 0.72) profile.atmosphere = "bright";
        else if (profile.averageSaturation < 0.25) profile.atmosphere = "soft";
        else profile.atmosphere = "energetic";
    } else {
        profile.visualStyle = "color data unavailable";
        profile.atmosphere = "unknown";
    }

    if (profile.glowDisabledObjectCount > 0) {
        profile.glowSignal = std::to_string(profile.glowDisabledObjectCount) +
            " objects explicitly disable glow; actual glow requires sprite metadata";
    }
    return profile;
}

ThemeProfile mixThemes(const ThemeProfile& a, const ThemeProfile& b, double blend) {
    const double t = std::clamp(blend, 0.0, 1.0);
    ThemeProfile result;
    result.background = blendOptional(a.background, b.background, t);
    result.ground = blendOptional(a.ground, b.ground, t);
    result.primary = blendOptional(a.primary, b.primary, t);
    result.secondary = blendOptional(a.secondary, b.secondary, t);
    result.backgroundTexture = t < 0.5 ? a.backgroundTexture : b.backgroundTexture;
    result.groundTexture = t < 0.5 ? a.groundTexture : b.groundTexture;
    result.objectCount = a.objectCount;
    result.width = a.width;
    result.height = a.height;
    result.objectsPer1000Units = a.objectsPer1000Units;
    result.averageSaturation = a.averageSaturation * (1.0 - t) + b.averageSaturation * t;
    result.averageLuminance = a.averageLuminance * (1.0 - t) + b.averageLuminance * t;
    result.visualStyle = t < 0.5 ? a.visualStyle : b.visualStyle;
    result.density = a.density;
    result.atmosphere = t < 0.5 ? a.atmosphere : b.atmosphere;
    result.glowSignal = "not inferred by theme mixing";
    result.explicitTriggerColorCount = a.explicitTriggerColorCount;
    result.glowDisabledObjectCount = a.glowDisabledObjectCount;
    result.mostUsedChannels = t < 0.5 ? a.mostUsedChannels : b.mostUsedChannels;
    return result;
}

} // namespace geodecor
