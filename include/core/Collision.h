#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include "core/DataOperations.h"
#include "core/DataStructs.h"
#include "structs/IPD.h"

struct CollisionData {
    IPD_COLL_HEADER                     header;
    std::vector<IPD_COLL_SPLIT_VERTEX>  splitVertices;
    std::vector<IPD_COLL_SURFACE>       surfaces;
    std::vector<IPD_COLL_SUBCELL>       subcells;
    std::vector<IPD_COLL_CYLINDER>      cylinders;
};

bool UnpackCollisionData(CollisionData& collisionData, BinaryFile& file);