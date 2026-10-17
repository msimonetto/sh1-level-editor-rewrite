#include <iostream>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <format>
#include <filesystem>

#include "core/Chunk.h"
#include "mgmt/Workspace.h"
#include "mgmt/HexOperations.h"

Workspace::Workspace(
    const std::filesystem::path& sourceDir,
    const std::filesystem::path& targetDir,
    const std::string& prefix)
    :
    sourceDir(sourceDir), targetDir(targetDir), prefix(prefix)
{
    if (!std::filesystem::exists(sourceDir) || !std::filesystem::is_directory(sourceDir)) {
        throw std::invalid_argument("[WARNING]: Source directory did not exist.");
    }

    if (!std::filesystem::exists(targetDir) || !std::filesystem::is_directory(targetDir)) {
        throw std::invalid_argument("[WARNING]: Target directory did not exist.");
    }

    texturePool = std::make_unique<TexturePool>(sourceDir);
    globalObjects = std::make_unique<GlobalObjects>(sourceDir, *texturePool);
}

Workspace::~Workspace() {}

bool Workspace::AddChunk(int8_t x, int8_t z) {
    uint16_t hash = ChunkHash(x, z);

    // Ensures unique XZ coordinates (for fixed prefix)
    if (chunkPool.contains(hash)) {
        std::cerr << "[WARNING]: Chunk already exists with hash " << (int)hash << std::endl;
        return false;
    }

    std::string filename = std::format("{}{:04X}.IPD", this->prefix, hash);
    std::filesystem::path sourcePath = this->sourceDir / filename;

    if (std::filesystem::exists(sourcePath)) {
        auto chunk = std::make_unique<Chunk>(
            sourcePath,                         // inFile
            targetDir / filename,               // outFile
            *texturePool,
            *globalObjects
        );

        chunk->UnpackChunkToMem();
        chunkPool.emplace(hash, std::move(chunk));

        return true;
    } else {
        std::cerr << "[WARNING]: Chunk does not exist with filename " << filename << std::endl;
        return false;
    }
}

bool Workspace::RemoveChunk(int8_t x, int8_t z) {
    return (chunkPool.erase(ChunkHash(x, z)));
}

bool Workspace::PackChunk(int8_t x, int8_t z) {
    uint16_t hash = ChunkHash(x, z);

    if (!chunkPool.contains(hash)) {
        std::cerr << "[WARNING]: Tried to pack chunk with hash " << hash << " but it didn't exist." << std::endl;
        return false;
    }

    return chunkPool.at(hash)->PackChunkFromMem();
}