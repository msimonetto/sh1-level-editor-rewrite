#pragma once

#include <iostream>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <cstdlib>

#include "structs/LM.h"
#include "structs/TIM.h"

class TexturePool {
    private:
        std::unordered_map<uint32_t, LM_TEXTURE>	texturePool;		// Mapped by file offset (material.offset_texture)
        std::filesystem::path       sourceDir_;

        uint8_t                     chunkUsage;

    public:
        TexturePool(const std::filesystem::path& sourceDir);
        ~TexturePool();    
    
    void AddTexture();
        
};