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

    std::vector<SVECTOR3>               splitVertices;
    std::vector<IPD_COLL_SURFACE>       surfaces;
    std::vector<IPD_COLL_SUBCELL>       subcells;
    std::vector<IPD_COLL_CYLINDER>      cylinders;

    std::vector<IPD_COLL_SUBCELL_RANGE> subcellRanges;

    std::vector<uint8_t>                wallIndices;
    std::vector<uint8_t>                floorIndices;
};

bool UnpackCollisionData(CollisionData& collisionData, BinaryFile& file);