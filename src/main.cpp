#include <filesystem>
#include <string>
#include <memory>

#include "mgmt/Workspace.h"

int main() {
    std::filesystem::path baseDir   = "D:/Projects/sh1-level-editor-rewrite/data/";
    std::filesystem::path sourceDir = baseDir / "Game Extracts/BG/";
    std::filesystem::path targetDir = baseDir / "Outputs/BG/";

    auto workspace = std::make_unique<Workspace>(sourceDir, targetDir, "THR");
    workspace->Workspace::AddChunk(-1, -1);
    workspace->Workspace::AddChunk(0, 0);
    workspace->Workspace::AddChunk(-1, -1);

    // Later: Perform some fixed position manipulations
    // Later: Perform some dynamic manipulations

    return 0;
}