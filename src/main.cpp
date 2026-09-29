#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>

#include "core/FileManager.h"
#include "tools/IPDTools.h"

int main() {
    // std::string path = "/mnt/data/Projects/sh1-level-editor-rewrite/data/Game Extracts/BG/THR0000.IPD";
    std::string path = "D:/Projects/sh1-level-editor-rewrite/data/Game Extracts/BG/THR0000.IPD";
	auto chunk1 = std::make_unique<IPDTools::IPDChunk>(path);

    // Load into memory
	chunk1->UnpackIPDToMem();

    // Later: Perform some fixed position manipulations
    // Later: Perform some dynamic manipulations

    return 0;
}