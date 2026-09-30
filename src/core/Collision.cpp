#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

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
    std::cout << collisionData.header.count_IPD_COLL_SPLIT_VERTEX << std::endl;
    // File offset persistently at 84 bytes (sizeof(IPD_HEADER))

    

    return true;
}