#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <cstdlib>

#include "core/DataOperations.h"
#include "core/DataStructs.h"
#include "structs/IPD.h"

struct CollisionData {
    IPD_COLL_HEADER                     header;
    int32_t                             positionX;
    int32_t                             positionZ;

    // Uniques
    std::vector<SVECTOR>                vertices;               // vertical
    std::vector<IPD_COLL_SURFACE>       surfaces;               // floor
    std::vector<IPD_COLL_LINE>          lines;                  // vertical
    std::vector<IPD_COLL_CYLINDER>      cylinders;              // vertical

    // Ordered table of uniques (not attached to subcell yet)
    std::vector<uint8_t>                verticalIdxPool;
    std::vector<uint8_t>                floorIdxPool;

    // Members of ordered table per subcell (both verticals and floors)
    std::vector<IPD_COLL_SUBCELL_LOOKUP>    subcellLookup;      // Maps each subcell in 20x20 grid (+1 member for terminus) to pool value (connected for verticals and floors, not one-to-one)
};

bool ParseCollision(CollisionData& collisionData, BinaryFile& inFile);
bool WriteCollision(const CollisionData& collisionData, BinaryFile& outFile);
uint32_t ReconcileCollision(uint32_t& currentOffset, CollisionData& collisionData);