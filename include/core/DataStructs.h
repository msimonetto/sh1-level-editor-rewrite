#pragma once

#include <vector>
#include <cstdint>
#include <unordered_map>

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
	std::unordered_map<uint32_t, LM_TEXTURE>	texturePool;		// Mapped by file offset (material.offset_texture)
	std::vector<ModelData>		models;
	// std::vector<ModelOrder>		modelOrder;
};