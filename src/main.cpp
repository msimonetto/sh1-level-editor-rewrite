#include <filesystem>
#include <string>
#include <memory>
#include <cstdint>

#include "mgmt/Workspace.h"

int main(int argc, char* argv[]) {
	std::string prefix = argv[1];
	uint8_t x = std::stoi(argv[2]),
			z = std::stoi(argv[3]);

    std::filesystem::path dir = "D:/Projects/sh1-level-editor-rewrite/data/";
    auto workspace = std::make_unique<Workspace>(dir, prefix);
    workspace->AddChunk(x, z);
    workspace->PackChunk(x, z);

    return 0;
}