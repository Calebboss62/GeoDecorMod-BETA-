#include "DecorationEngine.hpp"

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

namespace geodecor {
namespace {

struct OccupiedPosition {
    std::size_t sourceIndex;
    double x;
    double y;
};

bool validAsset(const DecorationAsset& asset) {
    return asset.objectId > 0 &&
        std::isfinite(asset.offsetX) &&
        std::isfinite(asset.offsetY) &&
        std::isfinite(asset.clearanceRadius) &&
        asset.clearanceRadius >= 0.0;
}

bool tooClose(
    double x,
    double y,
    double otherX,
    double otherY,
    double radius
) {
    const double dx = x - otherX;
    const double dy = y - otherY;

    return dx * dx + dy * dy < radius * radius;
}

} // namespace

DecorationGenerationReport generateDecorationProposals(
    const LevelDocument& level,
    const DecorationPlan& plan,
    const std::vector<DecorationAsset>& catalog,
    double minimumClearance
) {
    DecorationGenerationReport report;

    if (!std::isfinite(minimumClearance) ||
        minimumClearance < 0.0) {
        return report;
    }

    std::vector<const DecorationAsset*> usableAssets;

    for (const auto& asset : catalog) {
        if (!validAsset(asset))
            continue;

        if (asset.style != plan.settings.style)
            continue;

        if (asset.requiresGlow && !plan.settings.glowEnabled)
            continue;

        usableAssets.push_back(&asset);
    }

    std::vector<OccupiedPosition> occupied;

    occupied.reserve(
        level.objects.size() + plan.candidates.size()
    );

    for (std::size_t i = 0; i < level.objects.size(); i++) {
        const auto x = level.objects[i].number(2);
        const auto y = level.objects[i].number(3);

        if (x && y) {
            occupied.push_back({i, *x, *y});
        }
    }

    std::size_t maxPerSource = 1;

    if (plan.settings.intensity == DecorationIntensity::Medium)
        maxPerSource = 2;

    if (plan.settings.intensity == DecorationIntensity::Heavy)
        maxPerSource = 3;

    for (const auto& candidate : plan.candidates) {
        report.examinedSources++;

        if (candidate.objectIndex >= level.objects.size()) {
            report.skippedInvalidSources++;
            continue;
        }

        const auto& source = level.objects[candidate.objectIndex];

        const auto sourceId = source.integer(1);
        const auto sourceX = source.number(2);
        const auto sourceY = source.number(3);

        if (!sourceId || !sourceX || !sourceY ||
            *sourceId <= 0 ||
            *sourceId != candidate.objectId) {
            report.skippedInvalidSources++;
            continue;
        }

        std::size_t placedForSource = 0;

        for (const auto* asset : usableAssets) {
            if (placedForSource >= maxPerSource)
                break;

            const double x = *sourceX + asset->offsetX;
            const double y = *sourceY + asset->offsetY;

            const double radius = std::max(
                minimumClearance,
                asset->clearanceRadius
            );

            bool blocked = false;

            for (const auto& position : occupied) {
                if (position.sourceIndex == candidate.objectIndex)
                    continue;

                if (tooClose(
                    x,
                    y,
                    position.x,
                    position.y,
                    radius
                )) {
                    blocked = true;
                    break;
                }
            }

            if (blocked) {
                report.blockedByClearance++;
                continue;
            }

            report.decorations.push_back({
                candidate.objectIndex,
                candidate.objectId,
                asset->objectId,
                x,
                y
            });

            // Newly proposed decorations also occupy space.
            occupied.push_back({
                level.objects.size(),
                x,
                y
            });

            placedForSource++;
        }
    }

    report.proposedDecorations =
        report.decorations.size();

    return report;
}

} // namespace geodecor
