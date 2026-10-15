#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>

#include "core/Chunk.h"
#include "core/Collision.h"
#include "core/DataStructs.h"
#include "core/GlobalObjects.h"
#include "core/DataOperations.h"
#include "core/TexturePool.h"
#include "mgmt/FileManager.h"
#include "structs/IPD.h"
#include "structs/LM.h"

bool UnpackCollisionData(CollisionData& collisionData, BinaryFile& inFile) {
    auto& hdr = collisionData.header;

    collisionData.positionX = hdr.positionX;
    collisionData.positionZ = hdr.positionZ;

    // Base offset for collision data persistently at 84 bytes (sizeof(IPD_HEADER))
    // Unpack subcell lookup table
    const uint16_t subcellCount = hdr.count_subcellX * hdr.count_subcellZ;  // always expect 400, as observed in data
    UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_IPD_COLL_SUBCELL_LOOKUP,     subcellCount + 1,       collisionData.subcellLookup,    inFile);

    // Unpack vertical/floor index pools
    uint16_t poolSize = std::max(hdr.count_floorIdxPool, hdr.count_verticalIdxPool);
    UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_floorIdxPool,        hdr.count_floorIdxPool,         collisionData.floorIdxPool,     inFile);
    UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_verticalIdxPool,     hdr.count_verticalIdxPool,      collisionData.verticalIdxPool,  inFile);

    // Unpack all other constituent parts
    UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_IPD_COLL_SURFACE,    hdr.count_IPD_COLL_SURFACE,     collisionData.surfaces,         inFile);
    UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_IPD_COLL_VERTEX,     hdr.count_IPD_COLL_VERTEX,      collisionData.vertices,         inFile);
    UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_IPD_COLL_LINE,       hdr.count_IPD_COLL_LINE,        collisionData.lines,            inFile);
    UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_IPD_COLL_CYLINDER,   hdr.count_IPD_COLL_CYLINDER,    collisionData.cylinders,        inFile);

    return true;
}

bool PackCollisionData(const CollisionData& collisionData, BinaryFile& outFile) {
    return false;
}