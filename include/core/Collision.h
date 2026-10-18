#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <cstdlib>

#include "core/DataOperations.h"
#include "core/DataStructs.h"
#include "structs/IPD.h"

// To allow for differencing/aggregating
struct SubcellLookupEntry : IPD_COLL_SUBCELL_LOOKUP {
    SubcellLookupEntry operator-(const SubcellLookupEntry& other) const {
        return {{static_cast<uint16_t>(verticalIdx0 - other.verticalIdx0), 
                 static_cast<uint16_t>(floorIdx0 - other.floorIdx0)}};
    }

    SubcellLookupEntry operator+(const SubcellLookupEntry& other) const {
        return {{static_cast<uint16_t>(verticalIdx0 + other.verticalIdx0), 
                 static_cast<uint16_t>(floorIdx0 + other.floorIdx0)}};
    }
};

struct CollisionData {
    IPD_COLL_HEADER                     header;
    int32_t                             positionX;
    int32_t                             positionZ;

    // Uniques
    BoundedVector_uint8<SVECTOR>                vertices;               // vertical
    BoundedVector_uint8<IPD_COLL_SURFACE>       surfaces;               // floor
    BoundedVector_uint8<IPD_COLL_LINE>          lines;                  // vertical
    BoundedVector_uint8<IPD_COLL_CYLINDER>      cylinders;              // vertical

    // Ordered table of uniques (not attached to subcell yet)
    BoundedVector_uint16<uint8_t>                verticalIdxPool;
    BoundedVector_uint16<uint8_t>                floorIdxPool;

    // Members of ordered table per subcell (both verticals and floors)
    // Maps each subcell in 20x20 grid (+1 member for terminus) to pool value (connected for verticals and floors, not one-to-one)
    BoundedVector_uint16<SubcellLookupEntry>    subcellLookupAggregated;    // From original
    BoundedVector_uint16<SubcellLookupEntry>    subcellLookupDifferenced;   // For editing
};

bool ParseCollision(CollisionData& collisionData, BinaryFile& inFile);
bool WriteCollision(const CollisionData& collisionData, BinaryFile& outFile);
uint32_t ReconcileCollision(uint32_t& currentOffset, CollisionData& collisionData);

bool DifferenceSubcellCounts(CollisionData& collisionData);
bool AggregateSubcellCounts(CollisionData& collisionData);
