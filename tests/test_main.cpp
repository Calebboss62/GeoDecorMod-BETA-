
#include "DecorationPlanner.hpp"
#include "DecorationEngine.hpp"
#include "LevelString.hpp"
#include "ThemeAnalyzer.hpp"
#include "BlockDNA.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace geodecor;

int main() {
    const std::string sample =
        "kA1,1,kA6,3,kA7,2,kS38,1_10_2_20_3_30_6_1|1_20_2_30_3_40_6_2|1_5_2_10_3_15_6_1000|1_7_2_8_3_9_6_1001;"
        "1,1,2,0,3,0,21,1,22,2,24,7;"
        "1,899,2,100,3,50,7,255,8,0,9,128,96,1;"
        "1,1,2,200,3,30,21,1;";

    const auto original = sample;
    auto parsed = parseInnerLevelString(sample);

    assert(parsed.ok);
    assert(parsed.document.objects.size() == 3);
    assert(parsed.document.settings.backgroundTexture == 3);
    assert(parsed.document.settings.groundTexture == 2);
    assert(parsed.document.settings.colors.size() == 4);
    assert(sample == original);

    // Theme analysis tests.
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

    // Block DNA: inventory and object ID matching.
    auto dna = analyzeBlockDNA(parsed.document);
    assert(dna.totalObjects == 3);
    assert(dna.entries.size() == 2);
    assert(dna.entries[0].objectId == 1);
    assert(dna.entries[0].count == 2);
    assert(dna.entries[0].hasBounds);
    assert(dna.entries[0].minX == 0.0);
    assert(dna.entries[0].maxX == 200.0);
    assert(dna.entries[0].minY == 0.0);
    assert(dna.entries[0].maxY == 30.0);
    assert(dna.entries[1].objectId == 899);
    assert(dna.entries[1].count == 1);

    auto blockMatches = findObjectsById(parsed.document, 1);
    assert(blockMatches.size() == 2);
    assert(blockMatches[0] == 0);
    assert(blockMatches[1] == 2);

    // Match reference metadata using exact Geometry Dash object IDs.
    std::vector<BlockReference> references = {
        {899, "Reference Block", "solid", "blocks/899.png", {"solid", "basic"}},
        {1, "Basic Block", "solid", "blocks/1.png", {"basic"}},
        {0, "Invalid Reference", "unknown", "", {}},
        {899, "Duplicate Reference", "solid", "", {}}
    };

    auto referenceMatches = matchBlockReferences(parsed.document, references);
    assert(referenceMatches.size() == 2);
    assert(referenceMatches[0].reference.objectId == 899);
    assert(referenceMatches[0].targetObjectCount == 1);
    assert(referenceMatches[1].reference.objectId == 1);
    assert(referenceMatches[1].targetObjectCount == 2);

    // Decoration planner: exact matches only, no level mutation.
    DecorationSettings settings;
    settings.blockSet = BlockSet::Solid;
    settings.style = DecorationStyle::Modern;
    settings.intensity = DecorationIntensity::Medium;
    settings.glowEnabled = true;

    auto plan = buildDecorationPlan(parsed.document, references, settings);

    assert(plan.scannedObjectCount == 3);
    assert(plan.candidates.size() == 3);

    assert(plan.candidates[0].objectId == 899);
    assert(plan.candidates[0].objectIndex == 1);

    assert(plan.candidates[1].objectId == 1);
    assert(plan.candidates[1].objectIndex == 0);

    assert(plan.candidates[2].objectId == 1);
    assert(plan.candidates[2].objectIndex == 2);

    assert(!plan.modifiesLevel);
    assert(plan.settings.blockSet == BlockSet::Solid);

    settings.blockSet = BlockSet::Glow;

    auto glowPlan = buildDecorationPlan(parsed.document, references, settings);
    assert(glowPlan.candidates.empty());

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

    assert(generation.examinedSources == 3);
    assert(generation.proposedDecorations == 3);
    assert(generation.blockedByClearance == 0);
    assert(generation.skippedInvalidSources == 0);
    assert(!generation.modifiesLevel);

   for (const auto& decoration : generation.decorations) {
    (void)decoration;
    assert(decoration.decorationObjectId == 700);
    assert(decoration.sourceObjectId > 0);
}

    // Invalid clearance must safely produce no decorations.
    auto invalidGeneration = generateDecorationProposals(
        parsed.document,
        enginePlan,
        catalog,
        -1.0
    );

    assert(invalidGeneration.proposedDecorations == 0);

    // Parser error handling tests.
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
