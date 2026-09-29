#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <format>
#include <bit>
#include <memory>

#include "tools/IPDTools.h"
#include "tools/FileMGMT.h"

// Diagnostic -- equivalent to Hex editor output
// Move to a separate IO-related file
template <std::integral T>
std::string AsHex(T x) {
    // Cast to unsigned and words swapped
    auto unsignedX = std::byteswap(static_cast<std::make_unsigned_t<T>>(x));
    return std::format("{:0{}X}", unsignedX, 2 * sizeof(T));
}

int main() {
    // std::string path = "/mnt/data/Projects/sh1-level-editor-rewrite/data/Game Extracts/BG/THR0000.IPD";
    std::string path = "D:/Projects/sh1-level-editor-rewrite/data/Game Extracts/BG/THR0000.IPD";
	auto chunk1 = std::make_unique<IPDTools::IPDChunk>(path);

	chunk1->UnpackIPDToMem();

    // Later: Perform some fixed position manipulations
    // Later: Perform some dynamic manipulations
    // Close
    return 0;
}
