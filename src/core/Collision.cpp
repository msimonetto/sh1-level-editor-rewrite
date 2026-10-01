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

bool UnpackCollisionData(CollisionData& collisionData, BinaryFile& file) {
    auto& hdr = collisionData.header;

    // File offset persistently at 84 bytes (sizeof(IPD_HEADER))
    // Unpack (wall) vertices, surfaces, subcells, cylinders
    UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_IPD_COLL_WALL_VERTEX,    hdr.count_IPD_COLL_WALL_VERTEX, collisionData.wallVertices, file);
    UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_IPD_COLL_SURFACE,        hdr.count_IPD_COLL_SURFACE,     collisionData.surfaces,     file);
    UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_IPD_COLL_WALL_LINE,      hdr.count_IPD_COLL_WALL_LINE,   collisionData.wallLines,    file);
    UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_IPD_COLL_CYLINDER,       hdr.count_IPD_COLL_CYLINDER,    collisionData.cylinders,    file);

    // Unpack subcell lookup table
    int subcellCount = hdr.count_subcellX * hdr.count_subcellZ;     // May be constant across all chunks? (400), define constant if yes
    collisionData.subcellLookup.resize(subcellCount + 1);
    UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_IPD_COLL_SUBCELL_LOOKUP,  subcellCount + 1,       collisionData.subcellLookup, file);

    // Unpack indices of wall vertices
    collisionData.wallIndices.resize(hdr.count_wallIndex);
    UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_wallIndex,           hdr.count_wallIndex,            collisionData.wallIndices,  file);

    std::cout << (int)(hdr.count_IPD_COLL_WALL_LINE + hdr.count_IPD_COLL_CYLINDER)-1 << std::endl;
    std::cout << (int)std::ranges::max(collisionData.wallIndices) << std::endl;

    // Unpack indices of floor surfaces
    collisionData.floorIndices.resize(hdr.count_floorIndex);
    UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_floorIndex,           hdr.count_floorIndex,          collisionData.floorIndices,  file);

    std::cout << (int)hdr.count_IPD_COLL_SURFACE-1 << std::endl;
    std::cout << (int)std::ranges::max(collisionData.floorIndices) << std::endl;

    // collisionData.floorIndices.resize(hdr.count_floorIndex);
    // UnpackToVector(sizeof(IPD_HEADER) + hdr.offset_floorIndex,          hdr.count_floorIndex,          collisionData.floorIndices, file);

    return true;
}