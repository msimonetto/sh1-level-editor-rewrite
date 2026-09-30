#pragma once

#include <vector>
#include <fstream>
#include <cstdint>
#include <filesystem>
#include <iostream>

#include "core/DataStructs.h"
#include "core/DataOperations.h"
#include "core/TexturePool.h"

// ~~~~~~~~~~~ READ OPERATIONS ~~~~~~~~~~~

// ~~~ More primitive steps ~~~
bool UnpackBinary(uint32_t offset, size_t length, void* output, BinaryFile& file);

// Read binary data and write directly to struct (struct)
template <typename T>
bool UnpackToStruct(uint32_t offset, size_t length, T& output, BinaryFile& file) {
	return UnpackBinary(offset, length, &output, file);
}

// Read binary data recursively and write directly to each struct in structVector
template <typename T>
bool UnpackToVector(uint32_t offset, size_t count, std::vector<T>& output, BinaryFile& file) {
    bool state = true;
	if (count == 0 || !offset) { return false; }
	output.resize(count);

    return UnpackBinary(offset, count * sizeof(T), output.data(), file);
}

// ~~~ More in-depth routines (likely shared) ~~~
bool UnpackLMData(LMData& lmData, size_t lmBaseOffset, TexturePool& texturePool, BinaryFile& file);

// ~~~~~~~~~~~ WRITE OPERATIONS ~~~~~~~~~~~