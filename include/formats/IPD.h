#pragma once
#include <cstdint>
#include "Shared.h"
#include "LM.h"

#pragma pack(push, 1)

// Adapted from binary template (sh1_model.bt) by Sparagas (https://github.com/Sparagas/Silent-Hill)

// IPD_COLL_SURFACE (size: 12 bytes)
struct IPD_COLL_SURFACE {
    int16_t         relativeX;                      // Relative X (Q7.8)
    int16_t         baseGroundHeight;
    int16_t         relativeZ;                      // Relative Z (Q7.8)
    uint16_t        groundType          : 5;        // related to enum e_GroundType in engine (concrete, grass, etc)
    uint16_t        disableHeight       : 3;        // bool(s?)
    uint16_t        field_6_8           : 3;        // related to special collision triggers (s_CollisionState)
    uint16_t        field_6_11          : 4;        // surface slope?
    uint16_t        field_6_15          : 1;        // missing from decomp
    int16_t         tiltAngleX;                     // q7_8
    int16_t         tiltAngleZ;                     // q7_8
};

// IPD_COLL_SUBCELL (size: 10 bytes)
struct IPD_COLL_SUBCELL {
    int16_t         field_0_0           : 14;       // internal X, Q7.8
    uint16_t        field_0_14          : 2;        // 2 bits for collision group ID
    int16_t         field_2_0           : 14;       // internal Y, Q7.8
    uint16_t        field_2_14          : 2;        // 2 bits for collision group ID
    int16_t         field_4;                        // internal Z
    uint8_t         splitVertexIdx0;
    uint8_t         splitVertexIdx1;
    uint8_t         surfaceIdx0;
    uint8_t         surfaceIdx1;
};

// IPD_COLL_SUBCELL_RANGE (size: 4 bytes)
struct IPD_COLL_SUBCELL_RANGE {
    int16_t         field_0;
    int16_t         field_2;
};

// IPD_COLL_DATA_18 (size: 10 bytes) -- unknown
struct IPD_COLL_DATA_18 {
    uint16_t        groundType          : 5;        // related to e_GroundType
    uint16_t        disableHeight       : 3;        // bool(s?)
    uint16_t        field_0_8           : 4;
    uint16_t        field_0_12          : 3;
    uint16_t        field_0_15          : 1;
    SVECTOR3        offset;                         // q7_8
    uint16_t        field_8;                        // q7_8
};

// IPD_COLL_HEADER (size: 308 bytes)
struct IPD_COLL_HEADER {
    int32_t         positionX;                      // Chunk world X (Q23.8)
    int32_t         positionZ;                      // Chunk world Z (Q23.8)
    uint8_t         splitVertexCount;
    uint8_t         surfaceCount;
    uint8_t         subcellCount;
    uint8_t         cylinderColliderCount;
    uint32_t        ptr_splitVertices;
    uint32_t        ptr_surfaces;
    uint32_t        ptr_subcells;                   // Adjusted from Sparagas, ptr_14 -> ptr_subcells
                                                    // Wall/obstacle elements, IpdColl_TestWallElement.c (verify)
    uint32_t        ptr_cylinderColliders;          // Adjusted from Sparagas, ptr_18 -> ptr_cylinderColliders
                                                    // Cylindrical colliders, IpdColl_TestFloorElement.c (street poles, trees, verify)
    int16_t         subcellSize;
    int8_t          subcellCountX;
    int8_t          subcellCountZ;
    uint32_t        ptr_subcellRanges;
    uint16_t        ptr_28_count;
    uint16_t        ptr_2C_count;
    uint32_t        ptr_28;
    uint32_t        ptr_2C;
    uint8_t         subcellCheckCount;
    uint8_t         __pad[3];
    uint8_t         subcellCheckIdx[256];
};

// IPD_MODEL_INSTANCE (size: 36 bytes)
struct IPD_MODEL_INSTANCE {
    uint32_t        ptr_modelHdr;                   // connects to LM_MODEL_HEADER
    MATRIX          mat;
};

// IPD_MODEL_INFO (size: 16 bytes)
struct IPD_MODEL_INFO {
    uint8_t         isGlobalPlm;                    // (0) inside IPD, (1) from `*_GLB.PLM`
    int8_t          __pad[3];
    u_Filename      name;
    uint32_t        ptr_modelHdr;
};

// IPD_MODEL_BUFFER (size: 24 bytes)
struct IPD_MODEL_BUFFER {
    uint8_t         modelInstanceCount;             // modelInstances count
    uint8_t         billboardInstanceCount;         // Unknown in Sparagas: field_1 -> billboardCount
                                                    //      Streetlamp glows, light source flares
    uint8_t         subcellCount;
    int8_t          __pad;
    int16_t         minX;
    int16_t         maxX;
    int16_t         minZ;
    int16_t         maxZ;
    uint32_t        ptr_modelInstances;
    uint32_t        ptr_billboardInstances;         // Unknown in Sparagas: ptr_field_10 -> ptr_billboardInstances
    uint32_t        ptr_subcellPositions;
};

// IPD_SUBCELL_RANGE (size: 2 bytes)
struct IPD_SUBCELL_RANGE {
    uint8_t         startIndex;
    uint8_t         count;
};

// IPD_SUBCELL_VISIBILITY_TABLE (size: 52 bytes)
struct IPD_SUBCELL_VISIBILITY_TABLE {
    IPD_SUBCELL_RANGE   subcells[5][5];             // 25 subcells
    uint8_t         __pad[2];
};

// IPD_SUBCELL_AABB (size: 8 bytes)
struct IPD_SUBCELL_AABB {
    int16_t         minX;
    int16_t         maxX;
    int16_t         minZ;
    int16_t         maxZ;
};

// IPD_BILLBOARD_INSTANCE (size: 8 bytes)
struct IPD_BILLBOARD_INSTANCE {
    SVECTOR3        coords;
    int8_t          type;                           // 0 = lamp/glow, 1 = flare
    int8_t          __pad;
};

// IPD_HEADER (size: 84 bytes)
struct IPD_HEADER {
    uint8_t         id;
    uint8_t         isLoaded;
    int8_t          cellX;
    int8_t          cellZ;
    uint32_t        ptr_LM_HEADER;
    uint8_t         modelCount;
    uint8_t         modelBufferCount;
    uint8_t         modelOrderCount;
    int8_t          __pad[9];
    uint32_t        ptr_modelInfos;
    uint32_t        ptr_modelBuffers;
    IPD_SUBCELL_VISIBILITY_TABLE    visibilityTable;
    uint32_t        ptr_modelOrderList;             // Missing from sh1-level-editor
    IPD_COLL_HEADER collisionData;

    bool isValid() const {
        return (id == 0x14);
    }
};

#pragma pack(pop)