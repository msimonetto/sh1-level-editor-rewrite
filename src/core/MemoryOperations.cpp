#include <iostream>
#include <vector>
#include <fstream>

#include "core/DataStructs.h"
#include "core/MemoryOperations.h"
#include "core/TexturePool.h"
#include "mgmt/FileManager.h"

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

bool UnpackLMData(LMData& lmData, size_t lmBaseOffset, TexturePool& texturePool, BinaryFile& file) {

	// Locate first material and the material count
	int materialBaseOffset 	= lmBaseOffset + lmData.header.offset_LM_MATERIAL;
	int materialCount 		= lmData.header.count_LM_MATERIAL;
	lmData.materials.resize(materialCount);

	// Unpack each material
	for (size_t i = 0; i < materialCount; ++i) {
		auto& material = lmData.materials[i];
		
		UnpackToStruct(
			materialBaseOffset + i * sizeof(LM_MATERIAL),
			sizeof(LM_MATERIAL),
			material, file
		);

		// std::cout << FileManager::Convert6BitFilenameToString(material.name) << std::endl;

		// // See if the material's texture is contained in texture pool (unordered_map), if not, add it
		// if (!lmData.texturePool.contains(material.offset_texture.string())) {
		// 	LM_TEXTURE uniqueTexture;
			
		// 	std::cout << FileManager::Convert6BitFilenameToString(material.name) << std::endl;

		// 	lmData.texturePool.insert({material.offset_texture, uniqueTexture});
		// }
	}

	// Locate first model and the model count
	int modelBaseOffset = lmBaseOffset + lmData.header.offset_LM_MODEL_HEADER;
	int modelCount 		= lmData.header.count_LM_MODEL_HEADER;
	lmData.models.resize(modelCount);

	// Unpack each model
	for (size_t i = 0; i < modelCount; ++i) {
		auto& model = lmData.models[i];

		UnpackToStruct(
			modelBaseOffset + i * sizeof(LM_MODEL_HEADER),
			sizeof(LM_MODEL_HEADER),
			model.header, file
		);

		// Locate model's first mesh and its mesh count
		int meshBaseOffset 	= lmBaseOffset + model.header.offset_LM_MESH_HEADER;
		int meshCount		= model.header.meshCount;
		model.meshes.resize(meshCount);

		// Unpack each mesh
		for (size_t j = 0; j < meshCount; ++j) {
			auto& mesh = model.meshes[j];

			UnpackToStruct(
				meshBaseOffset + j * sizeof(LM_MESH_HEADER),
				sizeof(LM_MESH_HEADER),
				mesh.header, file
			);

			const auto& hdr = mesh.header;

			// Locate mesh's primitives, vertices, ... and each of their counts, then unpack
			UnpackToVector(lmBaseOffset + hdr.offset_LM_PRIMITIVE,	hdr.count_LM_PRIMITIVE,	mesh.primitives,	file);
			UnpackToVector(lmBaseOffset + hdr.offset_VertexXY,		hdr.count_Vertex,		mesh.verticesXY,	file);
			UnpackToVector(lmBaseOffset + hdr.offset_VertexZ,		hdr.count_Vertex,		mesh.verticesZ,		file);
			UnpackToVector(lmBaseOffset + hdr.offset_LM_NORMAL,		hdr.count_LM_NORMAL,	mesh.normals,		file);
			UnpackToVector(lmBaseOffset + hdr.offset_Shading,		hdr.count_Shading,		mesh.shadings,		file);
		}
	}

	return true;
}

// ~~~~~~~~~~~ WRITE OPERATIONS ~~~~~~~~~~~
// None.