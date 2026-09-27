#pragma once
#include <cstdint>
#include "Shared.h"

#pragma pack(push, 1)

// Adapted mainly from sh1-level-editor

// TIM_FILE_HEADER (size: 8 bytes)
struct TIM_FILE_HEADER {
    uint32_t        id;
    uint32_t        flag;
    
    bool isValid() const {
        return (id == 0x10);
    }
};

#pragma pack(pop)