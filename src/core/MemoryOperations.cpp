#include <iostream>
#include <vector>
#include <fstream>

#include "core/MemoryOperations.h"

// Shared operations between Chunk and GlobalObjects (will be expanded further)

// ~~~~~~~~~~~ READ OPERATIONS ~~~~~~~~~~~

// Non-template version
bool UnpackBinary(uint32_t offset, size_t length, void* output, BinaryFile& file) {
	file.position = offset;
	file.stream.seekg(file.position, std::ios::beg);
	file.stream.read(reinterpret_cast<char*>(output), length);

	if (!(file.stream)) {
		std::cerr << "[ERROR]: Failed to unpack from offset " << offset << std::endl;
		return false;
	}

	return true;
}

// ~~~~~~~~~~~ WRITE OPERATIONS ~~~~~~~~~~~
// None.