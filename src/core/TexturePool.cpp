#include <iostream>
#include <string>
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
    std::cout << "[INFO]: Destroyed texture pool!" << std::endl;
}