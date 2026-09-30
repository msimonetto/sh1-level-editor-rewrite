#include <iostream>
#include <vector>
#include <fstream>

#include "core/MemoryOperations.h"

// Shared operations between Chunk and GlobalObjects (will be expanded further)

// ~~~~~~~~~~~ READ OPERATIONS ~~~~~~~~~~~
// Read binary data and write directly to struct (struct)
template <typename T, typename ptr_T>
void UnpackToStruct(ptr_T basePointer, size_t length, T& outStruct,
					std::fstream& streamFile, std::streampos& streamPos) {

	streamPos = basePointer;
	streamFile.seekg(streamPos, std::ios::beg);
	streamFile.read(reinterpret_cast<char*>(&outStruct), length);

	if (!(streamFile)) {
		std::cerr << "[ERROR]: Failed to unpack from offset " << basePointer << std::endl;
	}
}

// Read binary data recursively and write directly to each struct in structVector
template <typename T, typename ptr_T>
void UnpackToVector(ptr_T basePointer, size_t count, std::vector<T>& structVector,
					std::fstream& streamFile, std::streampos& streamPos) {

	structVector.resize(count);
	if (count == 0 || !basePointer) { return; }

	for (size_t i = 0; i < count; ++i) {
		UnpackToStruct(basePointer + i * sizeof(T), sizeof(T), structVector[i], streamFile, streamPos);
	}
}

// ~~~~~~~~~~~ WRITE OPERATIONS ~~~~~~~~~~~
// None.