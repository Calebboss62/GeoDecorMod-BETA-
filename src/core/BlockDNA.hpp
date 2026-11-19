#pragma once

#include "LevelString.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace geodecor {

// A compact, read-only summary of one Geometry Dash object ID in a level.
struct BlockDNAEntry {
    int objectId = -1;
    std::size_t count = 0;
    double minX = 0.0;
    double maxX = 0.0;
    double minY = 0.0;
    double maxY = 0.0;
    bool hasBounds = false;
};

// Metadata for a future user-supplied visual reference.
// objectId is the authoritative match key; imageFile is only a reference asset.
struct BlockReference {
    int objectId = -1;
    std::string label;
    std::string category;
    std::string imageFile;
    std::vector<std::string> tags;
};

struct BlockDNAMatch {
    BlockReference reference;
    std::size_t targetObjectCount = 0;
    std::vector<std::size_t> targetObjectIndices;
};

struct BlockDNAReport {
    std::size_t totalObjects = 0;
    std::vector<BlockDNAEntry> entries;
    std::vector<int> unknownObjectIds;
};

// Builds a sorted inventory from object IDs (key 1) and positions (keys 2, 3).
// Invalid/missing IDs are skipped; valid objects without coordinates are counted,
// but do not contribute to spatial bounds.
BlockDNAReport analyzeBlockDNA(const LevelDocument& level);

// Returns zero-based indices of objects with an exact object-ID match.
std::vector<std::size_t> findObjectsById(const LevelDocument& level, int objectId);

// Matches supplied reference metadata against a target level by exact object ID.
// References with invalid IDs are ignored. Duplicate reference IDs use the first entry.
std::vector<BlockDNAMatch> matchBlockReferences(
    const LevelDocument& target,
    const std::vector<BlockReference>& references);

} // namespace geodecor
