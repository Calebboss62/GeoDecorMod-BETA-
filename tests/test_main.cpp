#include "LevelString.hpp"
#include "ThemeAnalyzer.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <string>

using namespace geodecor;

int main() {
    const std::string sample =
        "kA1,1,kA6,3,kA7,2,kS38,1_10_2_20_3_30_6_1|1_20_2_30_3_40_6_2|1_5_2_10_3_15_6_1000|1_7_2_8_3_9_6_1001;"
        "1,1,2,0,3,0,21,1,22,2,24,7;"
        "899,1,2,100,3,50,7,255,8,0,9,128,96,1;"
        "1,1,2,200,3,30,21,1;";

    const auto original = sample;
    auto parsed = parseInnerLevelString(sample);
    assert(parsed.ok);
    assert(parsed.document.objects.size() == 3);
    assert(parsed.document.settings.backgroundTexture == 3);
    assert(parsed.document.settings.groundTexture == 2);
    assert(parsed.document.settings.colors.size() == 4);
    assert(sample == original); // analysis must never mutate the source string

    auto profile = analyzeTheme(parsed.document);
    assert(profile.objectCount == 3);
    assert(profile.background.has_value());
    assert(profile.background->r == 5);
    assert(profile.ground.has_value());
    assert(profile.ground->r == 7);
    assert(profile.primary.has_value());
    assert(profile.primary->r == 10);
    assert(profile.secondary.has_value());
    assert(profile.secondary->r == 20);
    assert(profile.mostUsedChannels.size() >= 1);
    assert(profile.explicitTriggerColorCount == 1);
    assert(profile.glowDisabledObjectCount == 1);
    assert(profile.glowSignal.find("sprite metadata") != std::string::npos);

    auto mixed = mixThemes(profile, profile, 0.5);
    assert(mixed.primary.has_value());
    assert(mixed.primary->r == profile.primary->r);

    auto malformed = parseInnerLevelString("kA1,1;1,1,2,0,3;");
    assert(!malformed.ok);
    assert(malformed.segment == 1);

    auto missingPosition = parseInnerLevelString("kA1,1;1,1,2,0;");
    assert(!missingPosition.ok);

    auto empty = parseInnerLevelString("");
    assert(!empty.ok);

    std::string oversized(32u * 1024u * 1024u + 1u, 'x');
    auto tooLarge = parseInnerLevelString(oversized);
    assert(!tooLarge.ok);
    assert(tooLarge.error.find("32 MiB") != std::string::npos);

    std::cout << "GeoDecor Studio core tests passed\n";
    return 0;
}
