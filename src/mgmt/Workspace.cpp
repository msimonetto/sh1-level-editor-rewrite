#include <iostream>
#include <set>
#include <utility>

#include "core/Chunk.h"
#include "mgmt/Workspace.h"
#include "mgmt/HexOperations.h"

Workspace::Workspace(
    const std::filesystem::path& sourceDir,
    const std::filesystem::path& targetDir,
    const std::string& prefix)
    :
    sourceDir(sourceDir), targetDir(targetDir), prefix(prefix),
    texturePool(std::make_unique<TexturePool>(sourceDir)),
    globalObjects(std::make_unique<GlobalObjects>(sourceDir, *texturePool))
{}

Workspace::~Workspace() {}

bool Workspace::AddChunk(int8_t x, int8_t z) {
    std::string filename = this->prefix + HexOperations::AsHex(x) + HexOperations::AsHex(z) + ".IPD";
    std::filesystem::path sourcePath = this->sourceDir / filename;

    // Ensures unique XZ coordinates (for fixed prefix)
    for (const auto& chunkPtr : chunkPool) {
        if (chunkPtr && chunkPtr->chunkMajorX_ == x && chunkPtr->chunkMajorZ_ == z) {
            std::cerr << "[WARNING]: Chunk already exists at (" << (int)x << ", " << (int)z << ")!" << std::endl;
            return false;
        }
    }

    if (std::filesystem::exists(sourcePath)) {
        this->chunkPool.push_back(
            std::make_unique<Chunk>(
                this->sourceDir / filename, // inFile
                this->targetDir / filename, // outFile
                *this->texturePool, *this->globalObjects
            )
        );

        // Unpack automatically
        chunkPool.back()->Chunk::UnpackChunkToMem();
        return true;
    } else {
        std::cout << "[WARNING]: Chunk does not exist with name " << filename << "!" << std::endl;
        return false;
    }

}