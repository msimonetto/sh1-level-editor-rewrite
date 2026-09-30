#pragma once

#include <vector>
#include <fstream>
#include <cstdint>
#include <filesystem>

struct BinaryFile {
    std::filesystem::path   path;
    std::fstream            stream;
    std::streampos          position;
    uint8_t                 state;
};

// ~~~~~~~~~~~ READ OPERATIONS ~~~~~~~~~~~
template <typename T, typename ptr_T>
void UnpackToStruct(ptr_T baseOffset, size_t length, T& outStruct, BinaryFile& file);

template <typename T, typename ptr_T>
void UnpackToVector(ptr_T basePointer, size_t count, std::vector<T>& structVector, BinaryFile& file);

// ~~~~~~~~~~~ WRITE OPERATIONS ~~~~~~~~~~~