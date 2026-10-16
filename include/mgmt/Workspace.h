#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>
#include <memory>
#include <cstdint>

#include "core/Chunk.h"
#include "core/GlobalObjects.h"
#include "core/TexturePool.h"

// (-1, -1) returns FFFF as expected, this is congruent with filename
inline uint16_t ChunkHash(int8_t x, int8_t z) {
    return static_cast<uint16_t>((static_cast<uint8_t>(x) << 8) | static_cast<uint8_t>(z));
}

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
        std::unordered_map<uint16_t, std::unique_ptr<Chunk>> chunkPool;

    public:
        Workspace(const std::filesystem::path& sourceDir, const std::filesystem::path& targetDir, const std::string& prefix);
        ~Workspace();

        bool AddChunk(int8_t x, int8_t z);
        bool RemoveChunk(int8_t x, int8_t z);
        bool PackChunk(int8_t x, int8_t z);
};