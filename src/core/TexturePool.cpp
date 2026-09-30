#include <iostream>
#include <string>
#include <cstring>
#include <filesystem>
#include <unordered_map>

#include "core/TexturePool.h"
#include "structs/LM.h"
#include "structs/TIM.h"
#include "structs/Shared.h"

TexturePool::TexturePool(const std::filesystem::path& sourceDir) : sourceDir_(sourceDir) {
    std::cout << "[INFO]: Constructed texture pool from path!" << std::endl;
}

TexturePool::~TexturePool() {
    for (const auto& [materialName, textureData] : pool_) {
        if (textureData.refCount > 0) {
            std::cout << "[WARN]: Purging texture " << materialName << " with active count of "
                      << textureData.refCount << "! Textures were not released earlier." << std::endl;
        }
    }
    pool_.clear();

    std::cout << "[INFO]: Destroyed texture pool!" << std::endl;
}

std::filesystem::path TexturePool::GetAbsolutePath(const std::string& materialName) {
    return ((sourceDir_ / materialName) += ".TIM");
}

bool TexturePool::IsValidName(const std::string& materialName) {
    if (!(materialName.size() == 8)) {
        std::cerr << "[ERROR]: Material name is not 8 characters long!" << std::endl;
        return false;
    }

    if (!(materialName.back() == 'F' || materialName.back() == 'H')) {
        std::cerr << "[ERROR]: Material name needs to end in 'F' (256x256) or 'H' (128x256)!" << std::endl;
        return false;
    }

    if (!(materialName.contains(prefix_))) {
        std::cerr << "[ERROR]: Material does not contain correct prefix name! It should be " << prefix_ << std::endl;
        return false;
    }

    return true;
}

bool TexturePool::AcquireTexture(const std::string& materialName) {
    // Check that filename is 8 characters long and is valid
    if (!IsValidName(materialName)) { return false; }

    // Find whether texture is unique, add to pool if yes
    if (!pool_.contains(materialName)) {
        TextureData uniqueTexture{
            .name       = {},
            .absPath    = GetAbsolutePath(materialName),
            .palettes   = 0,
            .refCount   = 1
        };
        std::memcpy(uniqueTexture.name.str, materialName.data(), 8);

        pool_.insert({materialName, uniqueTexture});
    }

    // If not unique, increment its refCount
    else {
        pool_.at(materialName).refCount++;
    }

    return true;
}

bool TexturePool::ReduceTexture(const std::string& materialName) {
    // Determine whether texture is contained in pool, then decrement
    if (!pool_.contains(materialName)) {
        std::cerr << "[ERROR]: Material " << materialName << " could not be found. Could not reduce." << std::endl;
        return false;
    } else {
        pool_.at(materialName).refCount--;
    }

    // Purge if the last instance of its texture
    if (pool_.at(materialName).refCount == 0) {
        pool_.erase(materialName);
    }

    return true;
}

bool TexturePool::PurgeTexture(const std::string& materialName) {
    unsigned int refCount = pool_.at(materialName).refCount;

    if (refCount) {
        std::cerr << "[WARN]: Purging texture " << materialName << " from texture pool with active count of "
                << pool_.at(materialName).refCount << std::endl;
    }

    pool_.erase(materialName);
    return true;
}
