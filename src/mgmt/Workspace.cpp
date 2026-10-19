#include <iostream>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <format>
#include <filesystem>

#include "core/Chunk.h"
#include "mgmt/FileTable.h"
#include "mgmt/Workspace.h"
#include "mgmt/HexOperations.h"
#include "mgmt/FileOperations.h"

Workspace::Workspace(
    const std::filesystem::path& dir,
    const std::string& prefix)
    :
    sourceDir(dir / "Game Extracts"), targetDir(dir / "Outputs"), prefix(prefix)
{
    if (!std::filesystem::exists(sourceDir) || !std::filesystem::is_directory(sourceDir)) {
        throw std::invalid_argument("[WARNING]: Source directory did not exist.");
    }

    if (!std::filesystem::exists(targetDir) || !std::filesystem::is_directory(targetDir)) {
        throw std::invalid_argument("[WARNING]: Target directory did not exist.");
    }

    texturePool     = std::make_unique<TexturePool>(sourceDir / "BG");

    std::filesystem::path sourcePLM = sourceDir / "BG" / (prefix + "_GLB.PLM");
    std::filesystem::path targetPLM = targetDir / "BG" / (prefix + "_GLB.PLM");
    if (std::filesystem::exists(sourcePLM)) {
        globalObjects   = std::make_unique<GlobalObjects>(sourcePLM, targetPLM, *texturePool);
    } else {
        globalObjects   = std::make_unique<GlobalObjects>(*texturePool);    // Inactive
    }
    
    fileTable       = std::make_unique<FileTable>(sourceDir / "fileTable.json");
}

Workspace::~Workspace() {}

// ~~~~~~~~~ CHUNK MANAGEMENT ~~~~~~~~~
bool Workspace::AddChunk(int8_t x, int8_t z) {
    uint16_t hash = ChunkHash(x, z);

    // Ensures unique XZ coordinates (for fixed prefix)
    if (chunkPool.contains(hash)) {
        std::cerr << "[WARNING]: Chunk already exists with hash " << (int)hash << std::endl;
        return false;
    }

    std::string filename = std::format("{}{:04X}.IPD", this->prefix, hash);
    std::filesystem::path sourcePath = (this->sourceDir / "BG" / filename).lexically_normal();
    std::filesystem::path targetPath = (this->targetDir / "BG" / filename).lexically_normal();

    if (std::filesystem::exists(sourcePath)) {
        auto chunk = std::make_unique<Chunk>(
            sourcePath,         // inFile
            targetPath,         // outFile
            *texturePool,
            *globalObjects
        );

        chunk->Parse();
        chunkPool.emplace(hash, std::move(chunk));

        chunk->GetDependencies();

        return true;
    } else {
        std::cerr << "[WARNING]: Chunk does not exist with path " << NormalisePath((targetDir / "BG" / filename).string()) << std::endl;
        return false;
    }
}

bool Workspace::RemoveChunk(int8_t x, int8_t z) {
    return (chunkPool.erase(ChunkHash(x, z)));
}

bool Workspace::ExportChunk(int8_t x, int8_t z) {
    uint16_t hash = ChunkHash(x, z);

    if (!chunkPool.contains(hash)) {
        std::cerr << "[WARNING]: Tried to pack chunk with hash " << hash << " but it didn't exist." << std::endl;
        return false;
    }

    return chunkPool.at(hash)->Write();
}

bool Workspace::ReconcileChunk(int8_t x, int8_t z) {
    uint16_t hash = ChunkHash(x, z);
    return chunkPool.at(hash)->ReconcileHeaders();
}

// ~~~~~~~~~ GLOBAL OBJECT MANAGEMENT ~~~~~~~~~
bool Workspace::AddGlobals() {
    return globalObjects->Parse();
}

bool Workspace::ExportGlobals() {
    return globalObjects->Write();
}
