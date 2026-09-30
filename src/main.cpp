#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <memory>

#include "core/Chunk.h"
#include "core/GlobalObjects.h"
#include "core/TexturePool.h"
#include "mgmt/FileManager.h"

int main() {
    // Specify file (keep to one chunk for now)
    // std::string path = "/mnt/data/Projects/sh1-level-editor-rewrite/data/Game Extracts/BG/THR0000.IPD"; // Linux
    std::filesystem::path sourceDir = "D:/Projects/sh1-level-editor-rewrite/data/Game Extracts/BG/"; // Windows
    std::string filename = "THR0000.IPD";

    // Create shared texture pool
    auto texturePool = std::make_unique<TexturePool>();

    // Create prefix-specific global object bank
    auto globalObjects = std::make_unique<GlobalObjects>(&texturePool, sourceDir);
    
    // Create chunk (THR0000.IPD)
    // Note: Overloading for major coords (as opposed to filename) will be introduced eventually
	auto chunk1 = std::make_unique<Chunk>(&texturePool, &globalObjects, sourceDir, filename);

    // Load into memory (most work is here currently)
	chunk1->UnpackIPDToMem();

    // Later: Perform some fixed position manipulations
    // Later: Perform some dynamic manipulations

    return 0;
}