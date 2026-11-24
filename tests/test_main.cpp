
#include "DecorationPlanner.hpp"
#include "DecorationEngine.hpp"
#include "LevelString.hpp"
#include "ThemeAnalyzer.hpp"
#include "BlockDNA.hpp"

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace geodecor;

#define CHECK(condition) do { \
    if (!(condition)) { \
        std::cerr << __FILE__ << ':' << __LINE__ \
                  << ": CHECK failed: " << #condition << '\n'; \
        std::exit(EXIT_FAILURE); \
    } \
} while (false)

int main() {
    const std::string sample =
        "kA1,1,kA6,3,kA7,2,kS38,1_10_2_20_3_30_6_1|1_20_2_30_3_40_6_2|1_5_2_10_3_15_6_1000|1_7_2_8_3_9_6_1001;"
        "1,1,2,0,3,0,21,1,22,2,24,7;"
        "1,899,2,100,3,50,7,255,8,0,9,128,96,1;"
        "1,1,2,200,3,30,21,1;";

    const auto original = sample;
    auto parsed = parseInnerLevelString(sample);

    CHECK(parsed.ok);
    CHECK(parsed.document.objects.size() == 3);
    CHECK(parsed.document.settings.backgroundTexture == 3);
    CHECK(parsed.document.settings.groundTexture == 2);
    CHECK(parsed.document.settings.colors.size() == 4);
    CHECK(sample == original);

    // Theme analysis tests.
    auto profile = analyzeTheme(parsed.document);
    CHECK(profile.objectCount == 3);
    CHECK(profile.background.has_value());
    CHECK(profile.background->r == 5);
    CHECK(profile.ground.has_value());
    CHECK(profile.ground->r == 7);
    CHECK(profile.primary.has_value());
    CHECK(profile.primary->r == 10);
    CHECK(profile.secondary.has_value());
    CHECK(profile.secondary->r == 20);
    CHECK(profile.mostUsedChannels.size() >= 1);
    CHECK(profile.explicitTriggerColorCount == 1);
    CHECK(profile.glowDisabledObjectCount == 1);
    CHECK(profile.glowSignal.find("sprite metadata") != std::string::npos);

    auto mixed = mixThemes(profile, profile, 0.5);
    CHECK(mixed.primary.has_value());
    CHECK(mixed.primary->r == profile.primary->r);

    // Block DNA: inventory and object ID matching.
    auto dna = analyzeBlockDNA(parsed.document);
    CHECK(dna.totalObjects == 3);
    CHECK(dna.entries.size() == 2);
    CHECK(dna.entries[0].objectId == 1);
    CHECK(dna.entries[0].count == 2);
    CHECK(dna.entries[0].hasBounds);
    CHECK(dna.entries[0].minX == 0.0);
    CHECK(dna.entries[0].maxX == 200.0);
    CHECK(dna.entries[0].minY == 0.0);
    CHECK(dna.entries[0].maxY == 30.0);
    CHECK(dna.entries[1].objectId == 899);
    CHECK(dna.entries[1].count == 1);

    auto blockMatches = findObjectsById(parsed.document, 1);
    CHECK(blockMatches.size() == 2);
    CHECK(blockMatches[0] == 0);
    CHECK(blockMatches[1] == 2);

    // Match reference metadata using exact Geometry Dash object IDs.
    std::vector<BlockReference> references = {
        {899, "Reference Block", "solid", "blocks/899.png", {"solid", "basic"}},
        {1, "Basic Block", "solid", "blocks/1.png", {"basic"}},
        {0, "Invalid Reference", "unknown", "", {}},
        {899, "Duplicate Reference", "solid", "", {}}
    };

    auto referenceMatches = matchBlockReferences(parsed.document, references);
    CHECK(referenceMatches.size() == 2);
    CHECK(referenceMatches[0].reference.objectId == 899);
    CHECK(referenceMatches[0].targetObjectCount == 1);
    CHECK(referenceMatches[1].reference.objectId == 1);
    CHECK(referenceMatches[1].targetObjectCount == 2);

    // Decoration planner: exact matches only, no level mutation.
    DecorationSettings settings;
    settings.blockSet = BlockSet::Solid;
    settings.style = DecorationStyle::Modern;
    settings.intensity = DecorationIntensity::Medium;
    settings.glowEnabled = true;

    auto plan = buildDecorationPlan(parsed.document, references, settings);

    CHECK(plan.scannedObjectCount == 3);
    CHECK(plan.candidates.size() == 3);

    CHECK(plan.candidates[0].objectId == 899);
    CHECK(plan.candidates[0].objectIndex == 1);

    CHECK(plan.candidates[1].objectId == 1);
    CHECK(plan.candidates[1].objectIndex == 0);

    CHECK(plan.candidates[2].objectId == 1);
    CHECK(plan.candidates[2].objectIndex == 2);

    CHECK(!plan.modifiesLevel);
    CHECK(plan.settings.blockSet == BlockSet::Solid);

    settings.blockSet = BlockSet::Glow;

    auto glowPlan = buildDecorationPlan(parsed.document, references, settings);
    CHECK(glowPlan.candidates.empty());

    // Decoration engine: generate read-only decoration proposals.
    DecorationSettings engineSettings;
    engineSettings.blockSet = BlockSet::Solid;
    engineSettings.style = DecorationStyle::Modern;
    engineSettings.intensity = DecorationIntensity::Medium;
    engineSettings.glowEnabled = true;

    auto enginePlan = buildDecorationPlan(
        parsed.document,
        references,
        engineSettings
    );

    std::vector<DecorationAsset> catalog = {
        {700, DecorationStyle::Modern, "detail", 0.0, 40.0, 12.0, false}
    };

    auto generation = generateDecorationProposals(
        parsed.document,
        enginePlan,
        catalog,
        20.0
    );

    CHECK(generation.examinedSources == 3);
    CHECK(generation.proposedDecorations == 3);
    CHECK(generation.blockedByClearance == 0);
    CHECK(generation.skippedInvalidSources == 0);
    CHECK(!generation.modifiesLevel);

   for (const auto& decoration : generation.decorations) {
    (void)decoration;
    CHECK(decoration.decorationObjectId == 700);
    CHECK(decoration.sourceObjectId > 0);
}

    // Invalid clearance must safely produce no decorations.
    auto invalidGeneration = generateDecorationProposals(
        parsed.document,
        enginePlan,
        catalog,
        -1.0
    );

    CHECK(invalidGeneration.proposedDecorations == 0);

    // Parser error handling tests.
    auto malformed = parseInnerLevelString("kA1,1;1,1,2,0,3;");
    CHECK(!malformed.ok);
    CHECK(malformed.segment == 1);

    auto missingPosition = parseInnerLevelString("kA1,1;1,1,2,0;");
    CHECK(!missingPosition.ok);

    auto empty = parseInnerLevelString("");
    CHECK(!empty.ok);

    std::string oversized(32u * 1024u * 1024u + 1u, 'x');
    auto tooLarge = parseInnerLevelString(oversized);
    CHECK(!tooLarge.ok);
    CHECK(tooLarge.error.find("32 MiB") != std::string::npos);

    std::cout << "GeoDecor Studio core tests passed\n";
    return 0;
}
