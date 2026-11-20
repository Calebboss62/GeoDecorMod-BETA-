#pragma once

#include "BlockDNA.hpp"

#include <cstddef>
#include <vector>

namespace geodecor {

enum class DecorationStyle {
    Modern,
    Glow,
    Tech,
    Simple,
    EffectHeavy
};

enum class DecorationIntensity {
    Light,
    Medium,
    Heavy
};

enum class BlockSet {
    All,
    Solid,
    Glow,
    Slopes,
    Details
};

struct DecorationSettings {
    BlockSet blockSet = BlockSet::All;
    DecorationStyle style = DecorationStyle::Modern;
    DecorationIntensity intensity = DecorationIntensity::Medium;
    bool glowEnabled = true;
};

struct DecorationCandidate {
    std::size_t objectIndex = 0;
    int objectId = -1;
};

struct DecorationPlan {
    std::size_t scannedObjectCount = 0;
    std::vector<DecorationCandidate> candidates;
    std::vector<int> unmatchedObjectIds;
    DecorationSettings settings;
    bool modifiesLevel = false;
};

DecorationPlan buildDecorationPlan(
    const LevelDocument& target,
    const std::vector<BlockReference>& references,
    const DecorationSettings& settings);

} // namespace geodecor
