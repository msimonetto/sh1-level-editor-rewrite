#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <format>
#include <bit>
#include <filesystem>
#include <unordered_map>
#include <iomanip>

#include "core/Chunk.h"
#include "core/DataStructs.h"
#include "core/GlobalObjects.h"
#include "core/MemoryOperations.h"
#include "core/TexturePool.h"
#include "mgmt/FileManager.h"
#include "structs/IPD.h"
#include "structs/LM.h"

// ~~~~~~~~~~~ INITIALISE BLANK CHUNK ~~~~~~~~~~~
Chunk::Chunk(const std::filesystem::path& sourceDir, const std::string& filename,
	         TexturePool& texturePool, GlobalObjects& globalObjects)
		   : texturePool_(texturePool), globalObjects_(globalObjects), loaded(false), legal(true) {

	// `loaded(false)`: needs to be loaded into memory via `UnpackIPDToMem()`
	//   `legal(true)`: until shown not to be legal chunk

	// Check if specified file exists, case-sensitive!
	std::filesystem::path fpath = sourceDir / filename;
	if (std::filesystem::exists(fpath) && fpath.extension().string() == ".IPD") {
		this->sourceDir_  	= sourceDir;
		this->filename_ 	= filename;
		this->file_.path	= fpath;

		std::cout << "[INFO]: Constructed chunk object from path!" << std::endl;
	} else {
		std::cerr << "[INFO]: Constructed chunk object but without path! " 
				  << "Note that file extensions are case-sensitive (.IPD)!" << std::endl;
		
		this->legal = false;
	}
}

Chunk::~Chunk() {
	// Need to decrement value in texture pool!
	std::cout << "[INFO]: Destroyed chunk object!" << std::endl;
}

// ~~~~~~~~~~~ READING IPD FILES ~~~~~~~~~~~
int Chunk::UnpackIPDToMem() {

	// Open IPD file
	file_.stream.open(file_.path, std::ios::in | std::ios::binary);
	if (!file_.stream) {
		std::cerr << "[ERROR]: Failed to open contents of IPD file!" << std::endl;
		return -1;
	}

	// Unpack IPD header
	UnpackToStruct(0, sizeof(IPD_HEADER), header_, file_);
	if (!(header_.isValid())) {
		std::cerr << "[ERROR]: Not a valid IPD header, value at position 0x00 is (as int) " << (int)(header_.id) << std::endl;
		return -1;
	}

	// Unpack Internal LM header
	int internalLMBaseOffset	= header_.offset_LM_HEADER;
	UnpackToStruct(internalLMBaseOffset, sizeof(LM_HEADER), internalLMData_.header, file_);

	// Locate first material and the material count
	int materialBaseOffset 	= internalLMBaseOffset + internalLMData_.header.offset_LM_MATERIAL;
	int materialCount 		= internalLMData_.header.count_LM_MATERIAL;
	internalLMData_.materials.resize(materialCount);

	// Unpack each material
	for (size_t i = 0; i < materialCount; ++i) {
		auto& material = internalLMData_.materials[i];
		
		UnpackToStruct(
			materialBaseOffset + i * sizeof(LM_MATERIAL),
			sizeof(LM_MATERIAL),
			material, file_
		);

		// // See if the material's texture is contained in texture pool (unordered_map), if not, add it
		// if (!internalLMData_.texturePool.contains(material.offset_texture.string())) {
		// 	LM_TEXTURE uniqueTexture;
			
		// 	std::cout << FileManager::Convert6BitFilenameToString(material.name) << std::endl;

		// 	internalLMData_.texturePool.insert({material.offset_texture, uniqueTexture});
		// }
	}

	// Locate first model and the model count
	int modelBaseOffset = internalLMBaseOffset + internalLMData_.header.offset_LM_MODEL_HEADER;
	int modelCount 		= internalLMData_.header.count_LM_MODEL_HEADER;
	internalLMData_.models.resize(modelCount);

	// Unpack each model
	for (size_t i = 0; i < modelCount; ++i) {
		auto& model = internalLMData_.models[i];

		UnpackToStruct(
			modelBaseOffset + i * sizeof(LM_MODEL_HEADER),
			sizeof(LM_MODEL_HEADER),
			model.header, file_
		);

		// Locate model's first mesh and its mesh count
		int meshBaseOffset 	= internalLMBaseOffset + model.header.offset_LM_MESH_HEADER;
		int meshCount		= model.header.meshCount;
		model.meshes.resize(meshCount);

		// Unpack each mesh
		for (size_t j = 0; j < meshCount; ++j) {
			auto& mesh = model.meshes[j];

			UnpackToStruct(
				meshBaseOffset + j * sizeof(LM_MESH_HEADER),
				sizeof(LM_MESH_HEADER),
				mesh.header, file_
			);

			const auto& hdr = mesh.header;

			// Locate mesh's primitives, vertices, ... and each of their counts, then unpack
			UnpackToVector(internalLMBaseOffset + hdr.offset_LM_PRIMITIVE,	hdr.count_LM_PRIMITIVE,	mesh.primitives,	file_);
			UnpackToVector(internalLMBaseOffset + hdr.offset_VertexXY,		hdr.count_Vertex,		mesh.verticesXY,	file_);
			UnpackToVector(internalLMBaseOffset + hdr.offset_VertexZ,		hdr.count_Vertex,		mesh.verticesZ,		file_);
			UnpackToVector(internalLMBaseOffset + hdr.offset_LM_NORMAL,		hdr.count_LM_NORMAL,	mesh.normals,		file_);
			UnpackToVector(internalLMBaseOffset + hdr.offset_Shading,		hdr.count_Shading,		mesh.shadings,		file_);
		}
	}

	// TODO: Model lookup table
	// Decide as to how PLMs should be loaded into memory
	// Assume: Global objects are only prefix-related (exclusively drawn from THR)

	// TODO: Model buffer table

	// TODO: Collision

	return 0;
}

int Chunk::PackMemToIPD() {
	return 0;
}

int Chunk::GetSizeInPath() {
	std::filesystem::path fpath = sourceDir_ / filename_;
	
	std::error_code errorCode;
	auto size = std::filesystem::file_size(fpath);

	if (errorCode) {
		std::cerr << "[ERROR]: " << errorCode.message() << std::endl;
		return 0;
	}

	return static_cast<int>(size);
}

int Chunk::GetSizeInMem() {
	// Comes later
	return 0;
}

bool Chunk::IsValid() {
	return (header_.isValid() && this->legal);
}
