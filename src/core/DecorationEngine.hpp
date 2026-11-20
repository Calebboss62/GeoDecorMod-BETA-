#pragma once

#include "DecorationPlanner.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace geodecor {

struct DecorationAsset {
    int objectId = -1;
    DecorationStyle style = DecorationStyle::Modern;
    std::string category;
    double offsetX = 0.0;
    double offsetY = 30.0;
    double clearanceRadius = 12.0;
    bool requiresGlow = false;
};

struct GeneratedDecoration {
    std::size_t sourceObjectIndex = 0;
    int sourceObjectId = -1;
    int decorationObjectId = -1;
    double x = 0.0;
    double y = 0.0;
};

struct DecorationGenerationReport {
    std::size_t examinedSources = 0;
    std::size_t proposedDecorations = 0;
    std::size_t blockedByClearance = 0;
    std::size_t skippedInvalidSources = 0;

    std::vector<GeneratedDecoration> decorations;

    bool modifiesLevel = false;
};

DecorationGenerationReport generateDecorationProposals(
    const LevelDocument& level,
    const DecorationPlan& plan,
    const std::vector<DecorationAsset>& catalog,
    double minimumClearance = 20.0
);

} // namespace geodecor
