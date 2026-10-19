#include <filesystem>
#include <string>
#include <memory>
#include <cstdint>

#include "mgmt/Workspace.h"
#include "gui/Window.h"

int main(int argc, char** argv) {
    std::filesystem::path dir = "D:/Projects/sh1-level-editor-rewrite/data/";
    std::unique_ptr<Workspace> workspace = std::make_unique<Workspace>(dir, "THR");
    workspace->AddChunk(0, 0);
    workspace->ExportChunk(0, 0);
    workspace->AddGlobals();
    workspace->ExportGlobals();
    
    // Window window(workspace.get(), {
    //     .width = 1024, .height = 768, .title = "untitled - SHEditor",
    // });
    // window.Open();

    return 0;
}
