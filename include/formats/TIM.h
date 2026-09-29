#pragma once
#include <cstdint>
#include "Shared.h"

#pragma pack(push, 1)

// Adapted mainly from sh1-level-editor

// TIM_FILE_HEADER (size: 8 bytes)
struct TIM_FILE_HEADER {
    uint8_t         id;
    uint8_t         version;
    uint8_t         __pad_2[2];
    uint8_t         flags;
    uint8_t         __pad_5[3];
    
    bool isValid() const {
        return (id == 0x10);
    }
};

// TIM_CLUT_HEADER (size: 12 bytes)
struct TIM_CLUT_HEADER {
    int32_t         clut_length;
    uint16_t        x, y;
    uint16_t        height;
    uint16_t        width;
};

// TIM_IMG_HEADER (size: 12 bytes)
struct TIM_IMG_HEADER {
    int32_t         img_length;
    uint16_t        x, y;
    uint16_t        height;
    uint16_t        width;
};

#pragma pack(pop)