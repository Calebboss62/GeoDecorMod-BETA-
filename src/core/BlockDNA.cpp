#include "BlockDNA.hpp"

#include <algorithm>
#include <map>
#include <set>

namespace geodecor {

BlockDNAReport analyzeBlockDNA(const LevelDocument& level) {
    BlockDNAReport report;
    report.totalObjects = level.objects.size();

    std::map<int, BlockDNAEntry> inventory;
    std::set<int> invalidOrMissingIds;

    for (const auto& object : level.objects) {
        const auto id = object.integer(1);
        if (!id || *id <= 0) {
            if (id) invalidOrMissingIds.insert(*id);
            continue;
        }

        auto [it, inserted] = inventory.try_emplace(*id);
        auto& entry = it->second;
        if (inserted) entry.objectId = *id;
        ++entry.count;

        const auto x = object.number(2);
        const auto y = object.number(3);
        if (x && y) {
            if (!entry.hasBounds) {
                entry.minX = entry.maxX = *x;
                entry.minY = entry.maxY = *y;
                entry.hasBounds = true;
            } else {
                entry.minX = std::min(entry.minX, *x);
                entry.maxX = std::max(entry.maxX, *x);
                entry.minY = std::min(entry.minY, *y);
                entry.maxY = std::max(entry.maxY, *y);
            }
        }
    }

    report.entries.reserve(inventory.size());
    for (const auto& [id, entry] : inventory) {
        (void)id;
        report.entries.push_back(entry);
    }
    report.unknownObjectIds.assign(invalidOrMissingIds.begin(), invalidOrMissingIds.end());
    return report;
}

std::vector<std::size_t> findObjectsById(const LevelDocument& level, int objectId) {
    std::vector<std::size_t> matches;
    if (objectId <= 0) return matches;

    for (std::size_t i = 0; i < level.objects.size(); ++i) {
        if (level.objects[i].integer(1).value_or(-1) == objectId) {
            matches.push_back(i);
        }
    }
    return matches;
}

std::vector<BlockDNAMatch> matchBlockReferences(
    const LevelDocument& target,
    const std::vector<BlockReference>& references) {
    std::vector<BlockDNAMatch> matches;
    std::set<int> seen;

    for (const auto& reference : references) {
        if (reference.objectId <= 0 || !seen.insert(reference.objectId).second) continue;

        auto indices = findObjectsById(target, reference.objectId);
        if (indices.empty()) continue;

        BlockDNAMatch match;
        match.reference = reference;
        match.targetObjectCount = indices.size();
        match.targetObjectIndices = std::move(indices);
        matches.push_back(std::move(match));
    }
    return matches;
}

} // namespace geodecor
