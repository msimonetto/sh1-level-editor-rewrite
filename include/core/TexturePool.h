#pragma once

#include <iostream>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <cstdlib>

#include "structs/LM.h"
#include "structs/TIM.h"
#include "structs/Shared.h"

struct TextureData {
    u_Filename              name;
    std::filesystem::path   absPath;
    uint8_t                 palettes = 0;
    unsigned int            refCount = 0;
};

class TexturePool {
    private:
        std::filesystem::path       sourceDir_;
        std::string                 prefix_;

        std::unordered_map<std::string, TextureData>    pool_;

    public:
        TexturePool(const std::filesystem::path& sourceDir);
        ~TexturePool();    
    
        std::filesystem::path GetAbsolutePath(const std::string& materialName);

        bool IsValidName(const std::string& materialName);
        bool AcquireTexture(const std::string& materialName);
        bool ReduceTexture(const std::string& materialName);
        bool PurgeTexture(const std::string& materialName);
};