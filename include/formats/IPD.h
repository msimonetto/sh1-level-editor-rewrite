#pragma once
#include <cstdint>
#include <cassert>
#include "Shared.h"
#include "LM.h"

#pragma pack(push, 1)

// Adapted from binary template (sh1_model.bt) by Sparagas (https://github.com/Sparagas/Silent-Hill)

// IPD_HEADER (size: 84 bytes)
struct IPD_HEADER {
    uint8_t         id;
    uint8_t         isLoaded;
    int8_t          cellX;
    int8_t          cellY;
    uint32_t        ptr_LM_HEADER;
    uint8_t         modelCount;
    uint8_t         modelBufferCount;
    uint8_t         modelOrderCount;
    int8_t          __pad_B[9];
    uint32_t        ptr_modelInfos;
    uint32_t        ptr_modelBuffers;
    uint8_t         textureCount;                   // 127 is reasonable and generous for texture count, game engine caps this at a much lower point (PSX: 4?, PC: 30+)
    char            unk_1D[51];                     // Still indeterminate in Sparagas binary template
    uint32_t        ptr_modelOrderList;             // Missing from sh1-level-editor
    IPD_COLL_HEADER collisionData;

    IPD_HEADER() {
        assert(id == 0x14);
    }
};

// IPD_COLL_HEADER (size: 308 bytes)
struct IPD_COLL_HEADER {
    int32_t         positionX;
    int32_t         positionZ;
    uint8_t         splitVertexCount;
    uint8_t         surfaceCount;
    uint8_t         subcellCount;
    uint8_t         field_8_24;
    uint32_t        ptr_splitVertices;
    uint32_t        ptr_surfaces;
    uint32_t        ptr_subcells;
    uint32_t        ptr_18;
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

// IPD_COLL_SURFACE (size: 12 bytes)
struct IPD_COLL_SURFACE {
    int16_t         field_0;                        // relative X, q7_8
    int16_t         baseGroundHeight;
    int16_t         field_4;                        // relative Z, q7_8
    uint16_t        groundType          : 5;        // related to enum e_GroundType in engine (concrete, grass, etc)
    uint16_t        disableHeight       : 3;        // bool(s?)
    uint16_t        field_6_8           : 3;        // related to special collision triggers (s_CollisionState)
    uint16_t        field_6_11          : 4;        // surface slope
    uint16_t        field_6_15          : 1;        // missing from decomp
    int16_t         tiltAngleX;                     // q7_8
    int16_t         tiltAngleZ;                     // q7_8
};

// IPD_COLL_SUBCELL (size: 10 bytes)
struct IPD_COLL_SUBCELL {
    int16_t         field_0_0           : 14;       // internal X, q7_8
    uint16_t        field_0_14          : 2;        // 2 bits for collision group ID
    uint16_t        field_2_0           : 14;       // internal X, q7_8
    uint16_t        field_2_14          : 2;        // 2 bits for collision group ID
    int16_t         field_4;
    uint8_t         splitVertexIdx0;
    uint8_t         splitVertexIdx1;
    uint8_t         surfaceIdx0;
    uint8_t         surfaceIdx1;
};

// IPD_COLL_SUBCELL_RANGE (size: 3 bytes)
struct IPD_COLL_SUBCELL_RANGE {
    int16_t         field_0;
    int8_t          field_2;
};

// IPD_COLL_SUBCELL_RANGE (size: 10 bytes) -- unknown
struct IPD_COLL_DATA_18 {
    uint16_t        groundType          : 5;        // related to e_GroundType
    uint16_t        disableHeight       : 3;        // bool(s?)
    uint16_t        field_0_8           : 4;
    uint16_t        field_0_12          : 3;
    uint16_t        field_0_15          : 1;
    SVECTOR3        offset;                         // q7_8
    uint16_t        field_8;                        // q7_8
};

// IPD_MODEL_INSTANCE (size: 36 bytes)
struct IPD_MODEL_INSTANCE {
    uint32_t        ptr_modelHdr;                   // connects to PLMs
    MATRIX          mat;
};

// IPD_MODEL_INFO (size: 16 bytes)
struct IPD_MODEL_INFO {
    uint8_t         isGlobalPlm;                    // (0) inside IPD, (1) from `_GLB.PLM`
    int8_t          __pad_1[3];
    u_Filename      name;
    uint32_t        ptr_modelHdr;
};

// IPD_MODEL_BUFFER (size: 24 bytes)
struct IPD_MODEL_BUFFER {
    uint8_t         modelInstanceCount;             /** `modelInstances` size. */
    uint8_t         field_1;
    uint8_t         subcellCount;
    int8_t          __pad_3;
    int16_t         minX;
    int16_t         maxX;
    int16_t         minZ;
    int16_t         maxZ;
    uint32_t        ptr_modelInstances;
    uint32_t        ptr_field_10;                   // Q7.8 | Pointer to unknown collision data. TODO: Wrong struct? See `Ipd_ChunkDraw`.
    uint32_t        ptr_subcellPositions;           /** Q7.8 | XZ positions. TODO: Wrong struct? See `Gfx_ChunkSubcellVisibleCheck`. */
};