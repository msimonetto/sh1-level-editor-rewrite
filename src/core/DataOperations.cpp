#include <iostream>
#include <vector>
#include <fstream>
#include <numeric>
#include <unordered_map>
#include <cassert>

#include "core/DataStructs.h"
#include "core/DataOperations.h"
#include "core/TexturePool.h"
#include "mgmt/FileOperations.h"
#include "mgmt/HexOperations.h"

// Shared operations between Chunk and GlobalObjects (will be expanded further)

// ~~~~~~~~~~~ ALIGNMENT ~~~~~~~~~~~
uint32_t AlignToDWORD(const uint32_t& offset) {
	return static_cast<uint32_t>((offset + 3) & ~3);
}

uint32_t AlignToQWORD(const uint32_t& offset) {
	return static_cast<uint32_t>((offset + 7) & ~7);
}

// Zero out until alignment to next 2048 byte sector (just how the file table works)
bool AlignFileToSector(BinaryFile& outFile) {
	outFile.stream.seekg(0, std::ios::end);
	uint32_t finalPosition 		= static_cast<uint32_t>(outFile.stream.tellp());
	uint32_t alignedPosition 	= (finalPosition + 2047) & ~2047;

	if (alignedPosition > finalPosition) {
		std::vector<char> pad(alignedPosition - finalPosition, 0);
		if (!PackBinary(finalPosition, pad.size(), pad.data(), outFile)) { return false; }
	}

	return true;
}

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

bool ParseLMMaterials(uint32_t lmBaseOffset, LMData& lmData, TexturePool& texturePool, BinaryFile& inFile) {
	// Locate first material and the material count
	int materialBaseOffset 	= lmBaseOffset + lmData.header.offset_LM_MATERIAL;
	int materialCount 		= lmData.header.count_LM_MATERIAL;
	lmData.materials.resize(materialCount);

	// Unpack each material
	for (size_t i = 0; i < materialCount; ++i) {
		auto& material = lmData.materials[i];
		
		if (!UnpackBinary(
				materialBaseOffset + i * sizeof(LM_MATERIAL),
				sizeof(LM_MATERIAL),
				&material, inFile)) {
			return false;
		}

		// Add unique textures (material.name) to texturePool
		if (!texturePool.AcquireTexture(material.name.str)) {
			return false;
		}
	}

	return true;
}

bool ParseLMModels(uint32_t lmBaseOffset, LMData& lmData, TexturePool& texturePool, BinaryFile& inFile) {
	// Locate first model and the model count
	int modelBaseOffset = lmBaseOffset + lmData.header.offset_LM_MODEL_HEADER;
	int modelCount 		= lmData.header.count_LM_MODEL_HEADER;
	lmData.models.resize(modelCount);

	// Unpack each model
	for (size_t i = 0; i < modelCount; ++i) {
		auto& model = lmData.models[i];

		if (!UnpackBinary(
				modelBaseOffset + i * sizeof(LM_MODEL_HEADER),
				sizeof(LM_MODEL_HEADER),
				&model.header, inFile)) {
			return false;
		}


		// Locate model's first mesh and its mesh count
		int meshBaseOffset 	= lmBaseOffset + model.header.offset_LM_MESH_HEADER;
		int meshCount		= model.header.count_LM_MESH_HEADER;
		model.meshes.resize(meshCount);

		// Unpack each mesh
		for (size_t j = 0; j < meshCount; ++j) {
			auto& mesh = model.meshes[j];

			if (!UnpackBinary(
					meshBaseOffset + j * sizeof(LM_MESH_HEADER),
					sizeof(LM_MESH_HEADER),
					&mesh.header, inFile)) {
				return false;
			}

			const auto& hdr = mesh.header;

			// Locate mesh's primitives, vertices, ... and each of their counts, then unpack
			if (!UnpackToVector(lmBaseOffset + hdr.offset_LM_PRIMITIVE,	hdr.count_LM_PRIMITIVE,	mesh.primitives,	inFile) ||
				!UnpackToVector(lmBaseOffset + hdr.offset_VertexXZ,		hdr.count_Vertex,		mesh.verticesXZ,	inFile) ||
				!UnpackToVector(lmBaseOffset + hdr.offset_VertexY,		hdr.count_Vertex,		mesh.verticesY,		inFile) ||
				!UnpackToVector(lmBaseOffset + hdr.offset_LM_NORMAL,	hdr.count_LM_NORMAL,	mesh.normals,		inFile) ||
				!UnpackToVector(lmBaseOffset + hdr.offset_Shading,		hdr.count_Shading,		mesh.shadings,		inFile)) {
				return false;
			}
		}
	}

	return true;
}

bool ParseLMModelOrder(uint32_t lmBaseOffset, LMData& lmData, BinaryFile& inFile) {
	uint32_t modelOrderBaseOffset = lmBaseOffset + lmData.header.offset_LM_MODEL_ORDER;
	uint8_t modelOrderCount = lmData.header.count_LM_MODEL_HEADER;

	if (!UnpackToVector(modelOrderBaseOffset, modelOrderCount, lmData.modelOrder, inFile)) {
		return false;
	}

	return true;
}

bool ParseLMData(uint32_t lmBaseOffset, LMData& lmData, TexturePool& texturePool, BinaryFile& inFile) {
	// Internal LM header
	if (!UnpackBinary(lmBaseOffset, sizeof(LM_HEADER), &lmData.header, inFile)) {
		return false;
	}
		
	// Internal LM data
	if (!ParseLMMaterials(lmBaseOffset, 	lmData, texturePool, 	inFile) ||
		!ParseLMModels(lmBaseOffset, 		lmData, texturePool, 	inFile) ||
		!ParseLMModelOrder(lmBaseOffset, 	lmData, 				inFile)) {
		return false;
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

bool WriteLMMaterials(uint32_t lmBaseOffset, const LMData& lmData, TexturePool& texturePool, BinaryFile& outFile) {
	// Locate first material and the material count
	int materialBaseOffset 	= lmBaseOffset + lmData.header.offset_LM_MATERIAL;
	int materialCount 		= lmData.header.count_LM_MATERIAL;

	// Pack each material
	for (size_t i = 0; i < materialCount; ++i) {
		auto& material = lmData.materials[i];
		
		if (!PackBinary(
				materialBaseOffset + i * sizeof(LM_MATERIAL),
				sizeof(LM_MATERIAL),
				&material, outFile)) {
			return false;
		}
	}

	return true;
}

bool WriteLMModels(uint32_t lmBaseOffset, const LMData& lmData, TexturePool& texturePool, BinaryFile& outFile) {
	// Locate first model and the model count (already reconciled)
	int modelBaseOffset = lmBaseOffset + lmData.header.offset_LM_MODEL_HEADER;
	int modelCount 		= lmData.header.count_LM_MODEL_HEADER;

	for (uint8_t i = 0; i < modelCount; ++i) {
		auto& model = lmData.models[i];

		// All LM_MODEL_HEADER structs are contiguous with each other, then payload comes later
		if (!PackBinary(
				modelBaseOffset + i * sizeof(LM_MODEL_HEADER),
				sizeof(LM_MODEL_HEADER),
				&model.header, outFile)) {
			return false;
		}

		// Locate model's first mesh and its mesh count (already reconciled)
		int meshBaseOffset 	= lmBaseOffset + model.header.offset_LM_MESH_HEADER;
		int meshCount		= model.header.count_LM_MESH_HEADER;

		for (uint8_t j = 0; j < meshCount; ++j) {
			auto& mesh = model.meshes[j];

			if(!PackBinary(
					meshBaseOffset + j * sizeof(LM_MESH_HEADER),
					sizeof(LM_MESH_HEADER),
					&mesh.header, outFile)) {
				return false;
			}

			const auto& hdr = mesh.header;

			// Locate mesh's primitives, vertices etc then pack
			if (!PackFromVector(lmBaseOffset + hdr.offset_LM_PRIMITIVE,	mesh.primitives,	outFile) ||
				!PackFromVector(lmBaseOffset + hdr.offset_VertexXZ,		mesh.verticesXZ,	outFile) ||
				!PackFromVector(lmBaseOffset + hdr.offset_VertexY,		mesh.verticesY,		outFile) ||
				!PackFromVector(lmBaseOffset + hdr.offset_LM_NORMAL,	mesh.normals,		outFile) ||
				!PackFromVector(lmBaseOffset + hdr.offset_Shading,		mesh.shadings,		outFile)) {
				return false;
			}
		}
	}

	return true;
}

bool WriteLMModelOrder(uint32_t lmBaseOffset, const LMData& lmData, BinaryFile& outFile) {
	uint32_t modelOrderBaseOffset = lmBaseOffset + lmData.header.offset_LM_MODEL_ORDER;
	uint8_t modelOrderCount = lmData.header.count_LM_MODEL_HEADER;

	if (!PackFromVector(modelOrderBaseOffset, lmData.modelOrder, outFile)) {
		return false;
	}

	return true;
}

bool WriteLMData(uint32_t lmHeaderOffset, const LMData& lmData, TexturePool& texturePool, BinaryFile& outFile) {
	// Header
	if (!PackBinary(lmHeaderOffset, sizeof(LM_HEADER), &lmData.header, outFile)) { return false; }

	// Payload
	if (!WriteLMMaterials(lmHeaderOffset, 	lmData, texturePool, 	outFile) ||
		!WriteLMModels(lmHeaderOffset, 		lmData, texturePool, 	outFile) ||
		!WriteLMModelOrder(lmHeaderOffset, 	lmData, 				outFile)) {
		return false;
	}

	return true;
}

// ~~~~~~~~~~~ RECONCILIATION ~~~~~~~~~~~
uint32_t ReconcileLMData(uint32_t currentOffset, LMData& lmData) {
	lmData.header.count_LM_MODEL_HEADER 	= lmData.models.size();
	lmData.header.count_LM_MATERIAL			= lmData.materials.size();

	ReconcilePush(currentOffset, lmData.header.offset_LM_MATERIAL, lmData.materials);
	ReconcilePush<LM_MODEL_HEADER>(currentOffset, lmData.header.offset_LM_MODEL_HEADER, lmData.models);

	if (!lmData.modelOrder.empty()) {
		uint32_t modelOrderPaddedSize = (lmData.modelOrder.size() + 15) & ~15;
		currentOffset = lmData.header.offset_LM_MODEL_ORDER + modelOrderPaddedSize - 16;
	}

	ReconcilePush(currentOffset, lmData.header.offset_LM_MODEL_ORDER, lmData.modelOrder);

	for (auto& model : lmData.models) {
		model.header.count_LM_MESH_HEADER			= model.meshes.size();
		ReconcilePush<LM_MESH_HEADER>(currentOffset, model.header.offset_LM_MESH_HEADER, model.meshes);

		for (auto& mesh : model.meshes) {
			assert(mesh.verticesXZ.size() == mesh.verticesY.size());

			mesh.header.count_LM_PRIMITIVE			= mesh.primitives.size();
			mesh.header.count_Vertex				= mesh.verticesXZ.size();
			mesh.header.count_LM_NORMAL				= mesh.normals.size();
			mesh.header.count_Shading				= mesh.shadings.size();

			ReconcilePush(currentOffset, mesh.header.offset_LM_PRIMITIVE, mesh.primitives);
	        ReconcilePush(currentOffset, mesh.header.offset_VertexXZ,     mesh.verticesXZ);
	        ReconcilePush(currentOffset, mesh.header.offset_VertexY,      mesh.verticesY);
	        ReconcilePush(currentOffset, mesh.header.offset_LM_NORMAL,    mesh.normals);
	        ReconcilePush(currentOffset, mesh.header.offset_Shading,      mesh.shadings);
		}
	}

	return currentOffset;
}
