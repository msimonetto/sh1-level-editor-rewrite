#pragma once
#include <cstdint>
#include <cassert>
#include "Shared.h"

#pragma pack(push, 1)

// Adapted from binary template (sh1_model.bt) by Sparagas (https://github.com/Sparagas/Silent-Hill)
// Relevant to both .PLM and .ILM extensions

// LM_FILE_HEADER (size: 20 bytes)
struct LM_HEADER {
    uint16_t        id;
    uint8_t         version;
    uint8_t         isLoaded;                       // bool
    uint8_t         materialCount;
    uint32_t        ptr_materials;
    uint8_t         modelCount;
    uint8_t         __pad_9[3];
    uint32_t        ptr_modelHdrs;
    uint32_t        ptr_modelOrder;

    LM_HEADER() {
        assert(id == 0x30);
        assert(version == 6);
    }
};

struct s_FsImageDesc {
    uint8_t         tPage[2];
    uint8_t         u, v;
    int16_t         clutX, clutY;
};

struct LM_NORMAL {
    int8_t          nx;
    int8_t          ny;
    int8_t          nz;
    uint8_t         count;
};

struct LM_PRIMITIVE {
    uint8_t         u1, v1;
    uint16_t        clutX : 6;                      // Upper six bits of 10 bits of X coordinate value for CLUT on the VRAM
    uint16_t        clutY : 9;                      // Nine bits of Y coordinate value for CLUT on the VRAM
    uint16_t        __pad : 1;
    uint8_t         u2, v2;
    uint8_t         field_6;                        // Set to `s_Material::field_E`
    int8_t          materialIdx     : 7;
    uint8_t         isTransparent   : 1;            // bool
    uint8_t         u3, v3;
    uint8_t         u4, v4;
    uint8_t         faceIdxs[4];
    uint8_t         normalIdx[4];
};

struct LM_MESH_HEADER {
    uint8_t         primitiveCount;
    uint8_t         vertexCount;
    uint8_t         normalCount;
    uint8_t         unkCount_3;
    uint32_t        ptr_primitives;
    uint32_t        ptr_verticesXy;
    uint32_t        ptr_verticesZ;
    uint32_t        ptr_normals;
    uint32_t        ptr_unkPtr_14;
};

struct LM_MODEL_HEADER {
    u_Filename      name;
    uint8_t         meshCount;
    uint8_t         vertexOffset;
    uint8_t         normalOffset;
    uint8_t         field_B_0       : 1;
    uint8_t         field_B_1       : 3;            // Value used in `func_800571D0` switch.
    uint8_t         field_B_4       : 2;
    uint8_t         unk_B_6         : 2;
    uint32_t        ptr_meshHdrs;
};

struct LM_TEXTURE {
    s_FsImageDesc   imageDesc;
    u_Filename      name;
    uint32_t        queueIdx;
    int8_t          refCount;
};

// s_Material (size: 24 bytes)
struct LM_MATERIAL {
    u_Filename      name;
    uint32_t        ptr_texture; 
    uint8_t         field_C;
    uint8_t         unk_D[1];
    uint8_t         tPage;
    uint8_t         field_F;
    uint16_t        base_clutY;
    uint16_t        field_12;

    union {
        uint8_t     u8[2];
        uint16_t    u16;
    } field_14;

    union {
        uint8_t     u8[2];
        uint16_t    u16;
    } field_16;
};