#pragma once

#include <vector>
#include <cstdint>
#include <unordered_map>
#include <fstream>
#include <filesystem>

#include "structs/IPD.h"
#include "structs/LM.h"

struct MeshData {
	LM_MESH_HEADER				header;
	std::vector<LM_PRIMITIVE>	primitives;
	std::vector<DVECTOR> 		verticesXY;
	std::vector<int16_t>		verticesZ;
	std::vector<LM_NORMAL>		normals;
	std::vector<uint8_t>		shadings;		
};

struct ModelData {
	LM_MODEL_HEADER				header;
	std::vector<MeshData>		meshes;
};

struct LMData {
	LM_HEADER					header;
	std::vector<LM_MATERIAL>	materials;
	std::vector<ModelData>		models;
	// std::vector<ModelOrder>		modelOrder;
};

struct BinaryFile {
    std::filesystem::path   path;
    std::fstream            stream;
    std::streampos          position;
    uint8_t                 state = 0;
};