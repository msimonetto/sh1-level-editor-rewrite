#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include <memory>

#include "core/Chunk.h"
#include "core/GlobalObjects.h"
#include "core/TexturePool.h"

// Takes in extracts and output directory, manages chunk loading/offloading, required global object banks and texture pools
// Prefix-specific
class Workspace {
    private:
        // std::string sourceDir = "/mnt/data/Projects/sh1-level-editor-rewrite/data/Game Extracts/BG/THR0000.IPD"; // Linux
        std::filesystem::path   sourceDir;
        std::filesystem::path   targetDir;
        std::string             prefix;

        // Create shared pools
        std::unique_ptr<TexturePool>        texturePool;
        std::unique_ptr<GlobalObjects>      globalObjects;
        std::vector<std::unique_ptr<Chunk>> chunkPool;

    public:
        Workspace(const std::filesystem::path& sourceDir, const std::filesystem::path& targetDir, const std::string& prefix);
        ~Workspace();

        bool AddChunk(int8_t x, int8_t z);
};