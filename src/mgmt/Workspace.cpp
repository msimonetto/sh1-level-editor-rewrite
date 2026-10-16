#include <iostream>

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

    if (std::filesystem::exists(sourcePath)) {
        this->chunkPool.push_back(
            std::make_unique<Chunk>(
                this->sourceDir, this->targetDir,
                filename,
                *this->texturePool, *this->globalObjects
            ));
        return true;
    } else {
        std::cout << "[WARNING]: File " << filename << " does not exist!" << std::endl;
        return false;
    }

}