#pragma once

#include <vector>
#include <fstream>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <type_traits>

#include "core/DataStructs.h"
#include "core/DataOperations.h"
#include "core/TexturePool.h"

// ~~~~~~~~~~~ ALIGNMENT ~~~~~~~~~~~
uint32_t AlignToDWORD(const uint32_t& offset);
uint32_t AlignToQWORD(const uint32_t& offset);
bool AlignFileToSector(BinaryFile& outFile);

// ~~~~~~~~~~~ READ OPERATIONS ~~~~~~~~~~~
// ~~~ More primitive steps ~~~
bool UnpackBinary(uint32_t offset, size_t length, void* output, BinaryFile& inFile);

// Read binary data recursively and write directly to each struct in structVector
template <typename T>
bool UnpackToVector(uint32_t offset, size_t count, std::vector<T>& output, BinaryFile& inFile) {
	if (count == 0 || !offset) {
		output.clear();
		return true;
	}
	output.resize(count);

    return UnpackBinary(offset, count * sizeof(T), output.data(), inFile);
}

// ~~~ More in-depth routines (likely shared) ~~~
bool ParseLMMaterials(uint32_t lmBaseOffset, LMData& lmData, TexturePool& texturePool, BinaryFile& inFile);
bool ParseLMModels(uint32_t lmBaseOffset, LMData& lmData, TexturePool& texturePool, BinaryFile& inFile);
bool ParseLMModelOrder(uint32_t lmBaseOffset, LMData& lmData, BinaryFile& inFile);
bool ParseLMData(uint32_t lmBaseOffset, LMData& lmData, TexturePool& texturePool, BinaryFile& inFile);

// ~~~~~~~~~~~ WRITE OPERATIONS ~~~~~~~~~~~
bool PackBinary(uint32_t offset, size_t length, const void* input, BinaryFile& outFile);

// Write binary data recursively from in-memory vector of structs/types
template <typename T>
bool PackFromVector(uint32_t offset, const std::vector<T>& input, BinaryFile& outFile) {
	if (input.size() == 0 || !offset) {
		return true;
	}

    return PackBinary(offset, input.size() * sizeof(T), input.data(), outFile);
}

bool WriteLMMaterials(uint32_t lmBaseOffset, const LMData& lmData, TexturePool& texturePool, BinaryFile& outFile);
bool WriteLMModels(uint32_t lmBaseOffset, const LMData& lmData, TexturePool& texturePool, BinaryFile& outFile);
bool WriteLMModelOrder(uint32_t lmBaseOffset, const LMData& lmData, BinaryFile& outFile);
bool WriteLMData(uint32_t lmBaseOffset, const LMData& lmData, TexturePool& texturePool, BinaryFile& outFile);

// ~~~~~~~~~~~ RECONCILIATION ~~~~~~~~~~~
template <typename DiskType = void>
inline void ReconcilePush(uint32_t& currentOffset, auto& assignedOffset, const auto& container, uint32_t anchor = 0) {
	if (container.empty()) {
		return;
	}

	constexpr size_t elementSize = []() {
   		if constexpr (std::is_void_v<DiskType>) {
	        return sizeof(std::ranges::range_value_t<decltype(container)>);
	    } else {
	        return sizeof(DiskType);
	    }
	}();

	currentOffset 	= AlignToDWORD(currentOffset);
	assignedOffset 	= currentOffset - anchor;
	currentOffset  += container.size() * elementSize;
};

uint32_t ReconcileLMData(uint32_t currentOffset, LMData& lmData);
