#pragma once
#include <cstdint>
#include "Shared.h"

#pragma pack(push, 1)

// Adapted from binary template (sh1_model.bt) by Sparagas (https://github.com/Sparagas/Silent-Hill)
//      Collision header and scene layout is extended further in its mapping
//      Ranges are inclusive at end (w/rt structs at fixed positions: IPD_HEADER, IPD_COLL_HEADER)

// ~~~~~~~~~~~~ COLLISION ~~~~~~~~~~~~
// IPD_COLL_SURFACE (size: 12 bytes)
struct IPD_COLL_SURFACE {
    int16_t         anchorX;                    // Relative X (Q7.8), relative to absolute top-left
    int16_t         baseHeight;                 // <= 0 (negative Y is up)
    int16_t         anchorZ;                    // Relative Z (Q7.8), relative to absolute top-left
    uint16_t        groundType          : 5;    // e_GroundType (concrete, grass, etc), footstep audio
    uint16_t        isWalkable          : 3;    // bool(s?)
    uint16_t        field_6_8           : 3;    // related to special collision triggers (s_CollisionState), stair/basement entry
    uint16_t        field_6_11          : 4;    // surface slope?
    uint16_t        __pad               : 1;
    int16_t         slopeX;                     // Slope gradient (dY/dX, Q7.8)
    int16_t         slopeZ;                     // Slope gradient (dY/dZ, Q7.8)
};

// IPD_COLL_LINE (size: 10 bytes)
//      2D line segments that are floor-material boundaries OR solid physical walls
//      Needs improvement
struct IPD_COLL_LINE {
    int16_t         collLineX           : 14;       // 2D line transform, loaded into GTE R11
    uint16_t        collTriggerUpper    : 2;        // Collision trigger upper section
    int16_t         collLineZ           : 14;       // 2D line transform, loaded into GTE R12
    uint16_t        collTriggerLower    : 2;        // Collision trigger lower section
    int16_t         length;
    uint8_t         vertexIdx0;
    uint8_t         vertexIdx1;
    uint8_t         surfaceIdx0;                    // front
    uint8_t         surfaceIdx1;                    // back, 0xFF = none
};

// IPD_COLL_SUBCELL_LOOKUP (size: 4 bytes), retrieves from pool
struct IPD_COLL_SUBCELL_LOOKUP {
    uint16_t        verticalIdx0;                   // Differenced with next value, not one-to-one
    uint16_t        floorIdx0;
};

// IPD_COLL_CYLINDER (size: 10 bytes)
struct IPD_COLL_CYLINDER {
    uint16_t        groundType          : 5;        // `e_GroundType` of the obstacle
    uint16_t        isFullHeight        : 3;        // bool -- whether the top surface can be stood on
                                                    //      false = standard obstacle (crate, curb)
                                                    //      true  = infinitely tall obstacle (tree)
    uint16_t        eventTrigger        : 4;        // dynamic enabling/disabling of obstacle
    uint16_t        __pad               : 4;        // Set as '0' across all IPD files (previously mapped as 3: unknown, 1: __pad)
    SVECTOR         center;                         // (vx, vy, vz) -- position on map plane (relative to its center)
    uint16_t        radius;                         // r        -- horizontal radius of cylinder
};

// IPD_COLL_HEADER (size: 308 bytes, range: 0x0054 - 0x0187)
// 20x20 spatial grid of subcells
struct IPD_COLL_HEADER {
    int32_t         positionX;                      // Chunk world X (Q23.8)?
    int32_t         positionZ;                      // Chunk world Z (Q23.8)?
    uint8_t         count_IPD_COLL_VERTEX;          // Wall segments corner vertices (unsure if this also relates to cylinders)
    uint8_t         count_IPD_COLL_SURFACE;         // Ground height field (contains baseGroundHeight)
    uint8_t         count_IPD_COLL_LINE;            // Connecting wall segment corner vertices
    uint8_t         count_IPD_COLL_CYLINDER;        // Radial obstacles (trees, barrels)
    uint32_t        offset_IPD_COLL_VERTEX;
    uint32_t        offset_IPD_COLL_SURFACE;
    uint32_t        offset_IPD_COLL_LINE;           // Adjusted from Sparagas, offset_14 -> offset_subcells
                                                    // Wall/obstacle elements, IpdColl_TestWallElement.c (verify)
    uint32_t        offset_IPD_COLL_CYLINDER;       // Adjusted from Sparagas, offset_18 -> offset_cylinderColliders
                                                    // Cylindrical colliders, IpdColl_TestFloorElement.c (street poles, trees, verify)
    uint16_t        subcellSize;                    // Fixed at 5120
    uint8_t         count_subcellX;                 // } Spatial grid size (could always be 20x20?)
    uint8_t         count_subcellZ;                 // }
    uint32_t        offset_IPD_COLL_SUBCELL_LOOKUP; // Lookup table for all 20x20 subcells, persistently length of 1604 bytes
    uint16_t        count_verticalIdxPool;
    uint16_t        count_floorIdxPool;
    uint32_t        offset_verticalIdxPool;         // Each wallIndex ranges from 0 to count_IPD_COLL_WALL_LINE + count_IPD_COLL_CYLINDER - 1
    uint32_t        offset_floorIdxPool;            // Each floorIndex ranges from 0 to count_IPD_COLL_SURFACE - 1
    uint8_t         runtimeCounter1;                // } Always stored as 0
    uint8_t         __pad[3];                       // }
    uint8_t         runtimeCounter2[256];           // }
};

// ~~~~~~~~~~~~ BUFFER ~~~~~~~~~~~~
// IPD_SCENE_GROUP_INSTANCE (size: 36 bytes) -- see if this one relates to global objects exclusively
struct IPD_SCENE_GROUP_INSTANCE {
    uint32_t        offset_LM_MODEL_HEADER;         // connects to LM_MODEL_HEADER
    MATRIX          mat;
};

// IPD_SCENE_BOUNDING_BOX (size: 8 bytes)
struct IPD_SCENE_BOUNDING_BOX {
    int16_t         minX;
    int16_t         maxX;
    int16_t         minZ;
    int16_t         maxZ;
};

// IPD_SCENE_GROUP (size: 24 bytes), assume contiguous
struct IPD_SCENE_GROUP {
    uint8_t         count_IPD_SCENE_GROUP_INSTANCE;
    uint8_t         count_IPD_SCENE_BILLBOARD;      // Tree leaves, etc
    uint8_t         count_IPD_SCENE_BOUNDING_BOX;
    int8_t          __pad;
    IPD_SCENE_BOUNDING_BOX   groupBoundingBox;
    uint32_t        offset_IPD_SCENE_GROUP_INSTANCE;
    uint32_t        offset_IPD_SCENE_BILLBOARD;
    uint32_t        offset_IPD_SCENE_BOUNDING_BOX;
};

// IPD_SCENE_BILLBOARD (size: 8 bytes)
struct IPD_SCENE_BILLBOARD {
    SVECTOR         coords;
    int8_t          type;                           // 0 = lamp/glow, 1 = flare
    int8_t          __pad;
};

// ~~~~~~~~~~~~ VISIBILITY TABLE ~~~~~~~~~~~~
// IPD_VISIBILITY_RANGE (size: 2 bytes)
struct IPD_VISIBILITY_RANGE {
    uint8_t         startIndex;
    uint8_t         count;
};

// IPD_VISIBILITY_TABLE (size: 52 bytes)
struct IPD_VISIBILITY_TABLE {
    IPD_VISIBILITY_RANGE   subcells[5][5];           // 25 subcells (differentiate this better in name from the 20x20 collision subcells)
    uint8_t         __pad[2];
};

// ~~~~~~~~~~~~ MODEL LOOKUP TABLE ~~~~~~~~~~~~
// IPD_MODEL_INFO (size: 16 bytes, range: 0x0188-dynamic)
struct IPD_MODEL_INFO {
    uint8_t         isGlobalPlm;                    // (0) inside IPD, (1) from `*_GLB.PLM`, most likely bool and __pad could be extended
                                                    //      Verify with actual game data
    int8_t          __pad[3];
    u_Filename      name;							// Asset name within the PLM
													// 		based purely on prefix? (verify)
    uint32_t        offset_LM_MODEL_HEADER;
};

// ~~~~~~~~~~~~ IPD HEADER ~~~~~~~~~~~~
// IPD_HEADER (size: 84 bytes w/o collision, range: 0x0000-0x0053)
struct IPD_HEADER {
    uint8_t         id;
    uint8_t         isLoaded;
    int8_t          chunkX;
    int8_t          chunkZ;
    uint32_t        offset_LM_HEADER;               // } Internally embedded LM data in IPD
    uint8_t         count_LM_MODEL_HEADER;          // }
    uint8_t         count_IPD_SCENE_GROUP;
    uint8_t         count_LM_MODEL_ORDER;           // uint8_t
    uint8_t         __pad[9];
    uint32_t        offset_IPD_MODEL_INFO;
    uint32_t        offset_IPD_SCENE_GROUP;
    IPD_VISIBILITY_TABLE    visibilityTable;
    uint32_t        offset_LM_MODEL_ORDER;          // uint8_t
                                                    // Note: Collision header is immediately after this (fixed position)

    bool isValid() const {
        return (id == 0x14);
    }
};

#pragma pack(pop)