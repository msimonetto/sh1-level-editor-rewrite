#pragma once

#include <vector>
#include <fstream>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <cstdlib>

#include "core/DataStructs.h"
#include "core/DataOperations.h"
#include "core/TexturePool.h"

// ~~~~~~~~~~~ READ OPERATIONS ~~~~~~~~~~~
// ~~~ More primitive steps ~~~
bool UnpackBinary(uint32_t offset, size_t length, void* output, BinaryFile& inFile);

// Read binary data and write directly to struct (struct)
template <typename T>
bool UnpackToStruct(uint32_t offset, size_t length, T& output, BinaryFile& inFile) {
	return UnpackBinary(offset, length, &output, inFile);
}

// Read binary data recursively and write directly to each struct in structVector
template <typename T>
bool UnpackToVector(uint32_t offset, size_t count, std::vector<T>& output, BinaryFile& inFile) {
	if (count == 0 || !offset) { return false; }
	output.resize(count);

    return UnpackBinary(offset, count * sizeof(T), output.data(), inFile);
}

// ~~~ More in-depth routines (likely shared) ~~~
bool UnpackLMMaterials(uint32_t lmBaseOffset, LMData& lmData, TexturePool& texturePool, BinaryFile& inFile);
bool UnpackLMModels(uint32_t lmBaseOffset, LMData& lmData, TexturePool& texturePool, BinaryFile& inFile);
bool UnpackLMModelOrder(uint32_t lmBaseOffset, LMData& lmData, BinaryFile& inFile);

// ~~~~~~~~~~~ WRITE OPERATIONS ~~~~~~~~~~~
bool PackBinary(uint32_t offset, size_t length, const void* input, BinaryFile& outFile);

// Write binary data from in-memory struct directly
template <typename T>
bool PackFromStruct(uint32_t offset, const T& input, BinaryFile& outFile) {
	return PackBinary(offset, sizeof(T), &input, outFile);
}

// Write binary data recursively from in-memory vector of structs/types
template <typename T>
bool PackFromVector(uint32_t offset, const std::vector<T>& input, BinaryFile& outFile) {
	if (input.size() == 0 || !offset) { return false; }

    return PackBinary(offset, input.size() * sizeof(T), input.data(), outFile);
}

bool PackLMMaterials(uint32_t lmBaseOffset, const LMData& lmData, TexturePool& texturePool, BinaryFile& outFile);
bool PackLMModels(uint32_t lmBaseOffset, const LMData& lmData, TexturePool& texturePool, BinaryFile& outFile);
bool PackLMModelOrder(uint32_t lmBaseOffset, const LMData& lmData, BinaryFile& outFile);

// ~~~~~~~~~~~ MISCELLANEOUS ~~~~~~~~~~~
uint32_t AlignToDWORD(const uint32_t& offset);
