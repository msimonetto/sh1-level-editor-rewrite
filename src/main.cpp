#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <memory>

#include "core/Chunk.h"
#include "core/GlobalObjects.h"
#include "core/TexturePool.h"
#include "mgmt/Workspace.h"

int main() {
    // Specify file (keep to one chunk for now)
    // std::string path = "/mnt/data/Projects/sh1-level-editor-rewrite/data/Game Extracts/BG/THR0000.IPD"; // Linux
    std::filesystem::path baseDir   = "D:/Projects/sh1-level-editor-rewrite/data/";
    std::filesystem::path sourceDir = baseDir / "Game Extracts/BG/";
    std::filesystem::path targetDir = baseDir / "Outputs/BG/";

    auto workspace = std::make_unique<Workspace>(sourceDir, targetDir, "THR");
    workspace->Workspace::AddChunk(-1, -1);

    // // Load into memory (most work is here currently)
	// chunk1->UnpackChunkToMem();
    // chunk1->PackChunkFromMem();

    // Later: Perform some fixed position manipulations
    // Later: Perform some dynamic manipulations

    return 0;
}