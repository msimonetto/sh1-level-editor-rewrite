#pragma once

#include <vector>
#include <cstdint>
#include <unordered_set>
#include <fstream>
#include <filesystem>

#include "structs/IPD.h"
#include "structs/LM.h"

// Relevant only to unpacked data, not the engine/C-type structs

// ~~~~~~~~~~~ CONSTRAINTS ~~~~~~~~~~~
template <typename T, size_t MaxSize>
class BoundedVector : public std::vector<T> {
public:
	BoundedVector() = default;
	using std::vector<T>::operator=;

	void push_back(const T& value) {
		if (this->size() < MaxSize) { std::vector<T>push_back(value); }
	}

	void push_back(T&& value) {
		if (this->size() < MaxSize) { std::vector<T>push_back(std::move(value)); }
	}

	template <typename... Args>
	auto emplace_back(Args&&... args) {
		if (this->size() >= MaxSize) {
			return this->back();
		}
		return std::vector<T>::emplace_back(std::forward<Args>(args)...);
	}
};

template <typename T> using BoundedVector_uint4 	= BoundedVector<T, 16>;
template <typename T> using BoundedVector_uint8 	= BoundedVector<T, 256>;
template <typename T> using BoundedVector_uint16 	= BoundedVector<T, 65536>;
template <typename T> using BoundedVector_fileTable = BoundedVector<T, 2074>;		// Temporary, until file table can expand

// ~~~~~~~~~~~ IN-MEMORY DATA TYPES ~~~~~~~~~~~
struct MeshData {
	LM_MESH_HEADER				header;
	BoundedVector_uint8<LM_PRIMITIVE>	primitives;
	BoundedVector_uint8<DVECTOR> 		verticesXZ;
	BoundedVector_uint8<int16_t>		verticesY;
	BoundedVector_uint8<LM_NORMAL>		normals;
	BoundedVector_uint8<uint8_t>		shadings;
};

struct ModelData {
	LM_MODEL_HEADER				header;
	BoundedVector_uint8<MeshData>		meshes;
};

struct LMData {
	LM_HEADER					header;
	BoundedVector_uint8<LM_MATERIAL>	materials;
	BoundedVector_uint8<ModelData>		models;
	BoundedVector_uint8<uint8_t>		modelOrder;
};

struct BinaryFile {
    std::filesystem::path   path;
    std::fstream            stream;
    std::streampos          position;
    uint8_t                 state = 0;
};
