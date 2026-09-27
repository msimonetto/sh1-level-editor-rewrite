#pragma once
#include <cstdint>

#pragma pack(push, 1)

// Adapted from binary template (sh1_model.bt) by Sparagas (https://github.com/Sparagas/Silent-Hill)

// Embedded structs
// MATRIX (size: 32 bytes)
struct MATRIX {
    int16_t         m[3][3];                        // 3x3 Euler rotation matrix
    int16_t         pad;
    int32_t         t[3];                           // 3D position
};

// SVECTOR3 (size: 6 bytes)
struct SVECTOR3 {
    int16_t         vx;
    int16_t         vy;
    int16_t         vz;
};

// DVECTOR (size: 4 bytes)
struct DVECTOR {
    int16_t         vx;
    int16_t         vy;
};

struct u_Filename {
    char            str[8];
    uint32_t        u32[2];
};