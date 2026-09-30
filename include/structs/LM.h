#pragma once
#include <cstdint>
#include "Shared.h"

#pragma pack(push, 1)

// Adapted from binary template (sh1_model.bt) by Sparagas (https://github.com/Sparagas/Silent-Hill)
// Relevant to both .PLM and .ILM extensions

// LM_NORMAL (size: 4 bytes)
struct LM_NORMAL {
    int8_t          nx;
    int8_t          ny;
    int8_t          nz;
    uint8_t         count;
};

// LM_PRIMITIVE (size: 20 bytes)
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

// LM_MESH_HEADER (size: 24 bytes)
struct LM_MESH_HEADER {
    uint8_t         count_LM_PRIMITIVE;
    uint8_t         count_Vertex;
    uint8_t         count_LM_NORMAL;
    uint8_t         count_Shading;                  // Unknown in Sparagas, related to offset_unkoffset_14. 
                                                    // Ambient occlusion. For unlit models, light intensity bytes are passed to GTE into shading buffer.
    uint32_t        offset_LM_PRIMITIVE;
    uint32_t        offset_VertexXY;
    uint32_t        offset_VertexZ;
    uint32_t        offset_LM_NORMAL;
    uint32_t        offset_Shading;                 // Collected from unkCount_3
};

// LM_MODEL_HEADER (size: 16 bytes)
struct LM_MODEL_HEADER {
    u_Filename      name;
    uint8_t         meshCount;
    uint8_t         u_offset_vertex;                // Why would these be standalone and uint8_t?
    uint8_t         u_offset_normal;                // Unsure what these do
    uint8_t         field_B_0       : 1;
    uint8_t         field_B_1       : 3;            // Unknown in Sparagas, related to func_800571D0
                                                    // PS1 ordering table 'depth bin', similar to Z-buffer in other engines
    uint8_t         field_B_4       : 2;            // Lighting mode (0 = unlit/flat, 1 = directional/GTE, 2 = ambient/point -- e.g., flashlight)
    uint8_t         unk_B_6         : 2;
    uint32_t        offset_LM_MESH_HEADER;
};

// s_FsImageDesc (size: 8 bytes)
struct s_FsImageDesc {
    uint8_t         tPage[2];
    uint8_t         u, v;
    int16_t         clutX, clutY;
};

// LM_TEXTURE (size: 24 bytes) -- mainly used at runtime, shouldn't be expanded
struct LM_TEXTURE {
    s_FsImageDesc   imageDesc;
    u_Filename      name;
    uint32_t        queueIdx;
    int8_t          refCount;                       // VRAM cache lifetime across shared models/chunks, runtime only (verify)
                                                    // Incremented per chunk/model, decremented until reaching 0 (offloaded)
    uint8_t         __pad[3];
};

// LM_MATERIAL (size: 24 bytes)
struct LM_MATERIAL {
    u_Filename      name;
    uint32_t        offset_texture; 
    uint8_t         field_C;
    uint8_t         unk_D[1];
    uint8_t         tPage;                          // Unknown in Sparagas, field_E
                                                    // TPage attribute byte (X/Y bits for texture, color depth 4 or 8-bit, semi-transparency)
    uint8_t         field_F;
    uint16_t        base_clutY;                     // Active CLUT attribute word
    uint16_t        field_12;                       // Original CLUT in file

    union {
        uint8_t     u8[2];
        uint16_t    u16;
    } field_14;                                     // Main/base UV offsets (as 2 bytes)

    union {
        uint8_t     u8[2];
        uint16_t    u16;
    } field_16;
};

// LM_FILE_HEADER (size: 20 bytes)
struct LM_HEADER {
    uint8_t         id;
    uint8_t         version;
    uint8_t         isLoaded;                       // bool
    uint8_t         count_LM_MATERIAL;
    uint32_t        offset_LM_MATERIAL;
    uint8_t         count_LM_MODEL_HEADER;
    uint8_t         __pad[3];
    uint32_t        offset_LM_MODEL_HEADER;
    uint32_t        offset_LM_MODEL_ORDER;

    bool isValid() const {
        return ((id == 0x30) && (version == 6));
    }
};

#pragma pack(pop)