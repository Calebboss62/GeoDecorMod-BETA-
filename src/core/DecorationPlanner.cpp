#include "DecorationPlanner.hpp"

#include <set>
#include <string>

namespace geodecor {
namespace {

bool categoryMatches(BlockSet blockSet, const std::string& category) {
    if (blockSet == BlockSet::All) return true;
    if (blockSet == BlockSet::Solid) return category == "solid";
    if (blockSet == BlockSet::Glow) return category == "glow";
    if (blockSet == BlockSet::Slopes)
        return category == "slope" || category == "slopes";
    if (blockSet == BlockSet::Details)
        return category == "detail" || category == "details";
    return false;
}

} // namespace

DecorationPlan buildDecorationPlan(
    const LevelDocument& target,
    const std::vector<BlockReference>& references,
    const DecorationSettings& settings) {

    DecorationPlan plan;
    plan.scannedObjectCount = target.objects.size();
    plan.settings = settings;

    std::set<int> seenIds;

    for (const auto& reference : references) {
        if (reference.objectId <= 0)
            continue;

        if (!seenIds.insert(reference.objectId).second)
            continue;

        if (!categoryMatches(settings.blockSet, reference.category))
            continue;

        const auto indices = findObjectsById(target, reference.objectId);

        if (indices.empty()) {
            plan.unmatchedObjectIds.push_back(reference.objectId);
            continue;
        }

        for (const auto index : indices) {
            plan.candidates.push_back({index, reference.objectId});
        }
    }

    return plan;
}

} // namespace geodecor
