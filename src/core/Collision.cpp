#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <cstdint>
#include <array>
#include <numeric>

#include "core/Chunk.h"
#include "core/Collision.h"
#include "core/DataStructs.h"
#include "core/GlobalObjects.h"
#include "core/DataOperations.h"
#include "core/TexturePool.h"
#include "mgmt/FileOperations.h"
#include "structs/IPD.h"
#include "structs/LM.h"

bool ParseCollision(CollisionData& collisionData, BinaryFile& inFile) {
    auto& hdr = collisionData.header;

    collisionData.positionX = hdr.positionX;
    collisionData.positionZ = hdr.positionZ;

    // Base offset for collision data persistently at 84 bytes (sizeof(IPD_HEADER))
    // Unpack subcell lookup table
    const uint16_t subcellCount = hdr.count_subcellX * hdr.count_subcellZ;  // always expect 400, as observed in data
    if (!UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_IPD_COLL_SUBCELL_LOOKUP,     subcellCount + 1,       collisionData.subcellLookupAggregated,    inFile)) {
        return false;
    }

    // Unpack vertical/floor index pools
    if (!UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_floorIdxPool,        hdr.count_floorIdxPool,         collisionData.floorIdxPool,     inFile) ||
        !UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_verticalIdxPool,     hdr.count_verticalIdxPool,      collisionData.verticalIdxPool,  inFile)) {
        return false;
    }

    // Unpack all other constituent parts
    if (!UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_IPD_COLL_SURFACE,    hdr.count_IPD_COLL_SURFACE,     collisionData.surfaces,         inFile) ||
        !UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_IPD_COLL_VERTEX,     hdr.count_IPD_COLL_VERTEX,      collisionData.vertices,         inFile) ||
        !UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_IPD_COLL_LINE,       hdr.count_IPD_COLL_LINE,        collisionData.lines,            inFile) ||
        !UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_IPD_COLL_CYLINDER,   hdr.count_IPD_COLL_CYLINDER,    collisionData.cylinders,        inFile)) {
        return false;
    }

    return DifferenceSubcellCounts(collisionData);
}

bool WriteCollision(const CollisionData& collisionData, BinaryFile& outFile) {
    auto& hdr = collisionData.header;

    // Assumes that offsets have been corrected (upon modification upstream)
    // Pack vectors of unique values (Collision.h)
    if (!PackFromVector(sizeof(IPD_HEADER) + hdr.offset_IPD_COLL_VERTEX,     collisionData.vertices,    outFile) ||
        !PackFromVector(sizeof(IPD_HEADER) + hdr.offset_IPD_COLL_SURFACE,    collisionData.surfaces,    outFile) ||
        !PackFromVector(sizeof(IPD_HEADER) + hdr.offset_IPD_COLL_LINE,       collisionData.lines,       outFile) ||
        !PackFromVector(sizeof(IPD_HEADER) + hdr.offset_IPD_COLL_CYLINDER,   collisionData.cylinders,   outFile)) {
        return false;
    }

    // Pack subcell lookup table
    if (!PackFromVector(sizeof(IPD_HEADER) + hdr.offset_IPD_COLL_SUBCELL_LOOKUP, collisionData.subcellLookupAggregated,  outFile)) {
        return false;
    }

    // Pack vertical/floor index pools (last)
    if (!PackFromVector(sizeof(IPD_HEADER) + hdr.offset_verticalIdxPool,     collisionData.verticalIdxPool,  outFile) ||
        !PackFromVector(sizeof(IPD_HEADER) + hdr.offset_floorIdxPool,        collisionData.floorIdxPool,     outFile)) {
        return false;
    }

    return true;
}

uint32_t ReconcileCollision(uint32_t& currentOffset, CollisionData& collisionData) {
    if (!collisionData.subcellLookupDifferenced.empty()) { AggregateSubcellCounts(collisionData); }

    collisionData.header.count_IPD_COLL_VERTEX     = collisionData.vertices.size();
    collisionData.header.count_IPD_COLL_SURFACE    = collisionData.surfaces.size();
    collisionData.header.count_IPD_COLL_LINE       = collisionData.lines.size();
    collisionData.header.count_IPD_COLL_CYLINDER   = collisionData.cylinders.size();
    collisionData.header.count_verticalIdxPool     = collisionData.verticalIdxPool.size();
    collisionData.header.count_floorIdxPool        = collisionData.floorIdxPool.size();

    ReconcilePush(currentOffset, collisionData.header.offset_IPD_COLL_VERTEX,         collisionData.vertices,         sizeof(IPD_HEADER));
    ReconcilePush(currentOffset, collisionData.header.offset_IPD_COLL_SURFACE,        collisionData.surfaces,         sizeof(IPD_HEADER));
    ReconcilePush(currentOffset, collisionData.header.offset_IPD_COLL_LINE,           collisionData.lines,            sizeof(IPD_HEADER));
    ReconcilePush(currentOffset, collisionData.header.offset_IPD_COLL_CYLINDER,       collisionData.cylinders,        sizeof(IPD_HEADER));
    ReconcilePush(currentOffset, collisionData.header.offset_IPD_COLL_SUBCELL_LOOKUP, collisionData.subcellLookupAggregated,    sizeof(IPD_HEADER));
    ReconcilePush(currentOffset, collisionData.header.offset_verticalIdxPool,         collisionData.verticalIdxPool,  sizeof(IPD_HEADER));
    ReconcilePush(currentOffset, collisionData.header.offset_floorIdxPool,            collisionData.floorIdxPool,     sizeof(IPD_HEADER));

    return currentOffset;
}

// Converting subcell lookup tables to more manageable form
bool DifferenceSubcellCounts(CollisionData& collisionData) {
    // 20x20 collision surface subcells per chunk + 1 for terminus
    if (collisionData.subcellLookupAggregated.size() != 401) { return false; }
    collisionData.subcellLookupDifferenced.resize(400);

    for (size_t i = 0; i < 400; ++i) {
        collisionData.subcellLookupDifferenced[i] = collisionData.subcellLookupAggregated[i+1] - collisionData.subcellLookupAggregated[i];
    }

    return true;
}

bool AggregateSubcellCounts(CollisionData& collisionData) {
    // 20x20 collision surface subcells per chunk
    if (collisionData.subcellLookupAggregated.size() != 400) { return false; }
    collisionData.subcellLookupAggregated.resize(401);

    std::partial_sum(
        collisionData.subcellLookupDifferenced.begin(),
        collisionData.subcellLookupDifferenced.end(),
        collisionData.subcellLookupAggregated.begin()
    );
    collisionData.subcellLookupAggregated[400] = collisionData.subcellLookupAggregated[399];

    return true;
}