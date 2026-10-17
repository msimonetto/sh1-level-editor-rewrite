#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>

#include "core/DataStructs.h"
#include "core/DataOperations.h"
#include "core/TexturePool.h"
#include "mgmt/FileOperations.h"

// Shared operations between Chunk and GlobalObjects (will be expanded further)

// ~~~~~~~~~~~ READ OPERATIONS ~~~~~~~~~~~
// Non-template version
bool UnpackBinary(uint32_t offset, size_t length, void* output, BinaryFile& inFile) {
	inFile.position = offset;
	inFile.stream.seekg(inFile.position, std::ios::beg);
	inFile.stream.read(reinterpret_cast<char*>(output), length);

	if (!(inFile.stream)) {
		std::cerr << "[ERROR]: Failed to unpack from offset " << offset << std::endl;
		return false;
	}

	return true;
}

bool UnpackLMModels(uint32_t lmBaseOffset, LMData& lmData, TexturePool& texturePool, BinaryFile& inFile) {
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
			model.header, inFile
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
				mesh.header, inFile
			);

			const auto& hdr = mesh.header;

			// Locate mesh's primitives, vertices, ... and each of their counts, then unpack
			UnpackToVector(lmBaseOffset + hdr.offset_LM_PRIMITIVE,	hdr.count_LM_PRIMITIVE,	mesh.primitives,	inFile);
			UnpackToVector(lmBaseOffset + hdr.offset_VertexXY,		hdr.count_Vertex,		mesh.verticesXY,	inFile);
			UnpackToVector(lmBaseOffset + hdr.offset_VertexZ,		hdr.count_Vertex,		mesh.verticesZ,		inFile);
			UnpackToVector(lmBaseOffset + hdr.offset_LM_NORMAL,		hdr.count_LM_NORMAL,	mesh.normals,		inFile);
			UnpackToVector(lmBaseOffset + hdr.offset_Shading,		hdr.count_Shading,		mesh.shadings,		inFile);
		}
	}

	return true;
}

bool UnpackLMMaterials(uint32_t lmBaseOffset, LMData& lmData, TexturePool& texturePool, BinaryFile& inFile) {
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
			material, inFile
		);

		// Add unique textures (material.name) to texturePool
		texturePool.AcquireTexture(material.name.str);
	}

	return true;
}

// ~~~~~~~~~~~ WRITE OPERATIONS ~~~~~~~~~~~
// Non-template version
bool PackBinary(uint32_t offset, size_t length, const void* input, BinaryFile& outFile) {
	outFile.position = offset;
	outFile.stream.seekg(outFile.position, std::ios::beg);
	outFile.stream.write(reinterpret_cast<const char*>(input), length);

	if (!(outFile.stream)) {
		std::cerr << "[ERROR]: Failed to pack to offset " << offset << std::endl;
		return false;
	}

	return true;
}

bool PackLMModels(uint32_t lmBaseOffset, const LMData& lmData, TexturePool& texturePool, BinaryFile& inFile) {
	// Locate first model and the model count
	int modelBaseOffset = lmBaseOffset + lmData.header.offset_LM_MODEL_HEADER;
	int modelCount 		= lmData.models.size();

	return true;
}

bool PackLMMaterials(uint32_t lmBaseOffset, const LMData& lmData, TexturePool& texturePool, BinaryFile& inFile) {
	return true;
}