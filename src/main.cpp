#include <filesystem>
#include <string>
#include <memory>

#include "mgmt/Workspace.h"

int main() {
    std::filesystem::path baseDir   = "D:/Projects/sh1-level-editor-rewrite/data/";
    std::filesystem::path sourceDir = baseDir / "Game Extracts/BG/";
    std::filesystem::path targetDir = baseDir / "Outputs/BG/";

    auto workspace = std::make_unique<Workspace>(sourceDir, targetDir, "THR");
    workspace->AddChunk(0, 0);
    workspace->PackChunk(0, 0);

    return 0;
}