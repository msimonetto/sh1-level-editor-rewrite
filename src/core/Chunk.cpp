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
Chunk::Chunk(TexturePool& texturePool, GlobalObjects& globalObjects,
	         const std::filesystem::path& sourceDir, const std::string& filename) {

	this->loaded = false; 			// Needs to be loaded into memory via `UnpackIPDToMem()`
	this->legal  = true;			// `true` until shown not to be legal chunk

	this->texturePool_ 		= texturePool;
	this->globalObjects_ 	= globalObjects;

	// Check if specified file exists, case-sensitive!
	std::filesystem::path fpath = sourceDir / filename;
	if (std::filesystem::exists(fpath) && fpath.extension().string() == "IPD") {
		this->sourceDir_  	= sourceDir;
		this->filename_ 	= filename;
		this->file_.path	= fpath;

		std::cout << "[INFO]: Constructed chunk object from path!" << std::endl;
	}
	
	else {
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

	// Open .IPD file
	file_.stream.open(file_.path, std::ios::in | std::ios::binary);
	if (!file_.stream) {
		std::cerr << "[ERROR]: Failed to open contents of IPD file!" << std::endl;
		return -1;
	}

	// Unpack its header and verify its first word (0x14)
	UnpackToStruct(0, sizeof(IPD_HEADER), header_, file_);
	if (!(header_.isValid())) {
		std::cerr << "[ERROR]: Not a valid IPD header, offset (int) = " << (int)(header_.id) << std::endl;
		return -1;
	}

	// Unpack into `localLMData_`'s member header
	UnpackToStruct(this->header_.ptr_LM_HEADER, sizeof(LM_HEADER), localLMData_.header, file_);

	// Use `localLMData_`'s header to populate its materials vector and its texture pool
	// Resize materials (vector of LM_MATERIAL) to materialCount
	localLMData_.materials.resize(localLMData_.header.materialCount);

	// Load each material iteratively (# = materialCount)
	for (size_t i = 0; i < localLMData_.header.materialCount; ++i) {
		auto& material = localLMData_.materials[i];

		// Read from `Base + i * sizeof(LM_MATERIAL)`
		UnpackToStruct(
			localLMData_.header.ptr_materials + i * sizeof(LM_MATERIAL),
			sizeof(LM_MATERIAL),
			material, file_
		);

		// // See if the material's texture is contained in texture pool (unordered_map), if not, add it
		// if (!localLMData_.texturePool.contains(material.ptr_texture.string())) {
		// 	LM_TEXTURE uniqueTexture;
			
		// 	std::cout << FileManager::Convert6BitFilenameToString(material.name) << std::endl;

		// 	localLMData_.texturePool.insert({material.ptr_texture, uniqueTexture});
		// }
	}

	// Use `localLMData_`'s header to populate its models vector and subsequently its meshes
	// Resize models (vector of ModelData) to modelCount
	localLMData_.models.resize(localLMData_.header.modelCount);
	for (size_t i = 0; i < localLMData_.header.modelCount; ++i) {
		auto& model = localLMData_.models[i];

		// Read from `Base + i * sizeof(LM_MODEL_HEADER)`
		UnpackToStruct(
			localLMData_.header.ptr_modelHdrs + i * sizeof(LM_MODEL_HEADER),
			sizeof(LM_MODEL_HEADER),
			model.header, file_
		);

		// Resize meshes (vector of MeshData) to meshCount
		model.meshes.resize(model.header.meshCount);
		for (size_t j = 0; j < model.header.meshCount; ++j) {
			auto& mesh = model.meshes[j];

			// Read single meshes from `Base + j * sizeof(LM_MESH_HEADER)` 
			UnpackToStruct(
				model.header.ptr_meshHdrs + j * sizeof(LM_MESH_HEADER),
				sizeof(LM_MESH_HEADER),
				mesh.header, file_
			);

			const auto& hdr = mesh.header;

			// Unpack lower-level data from each mesh via its header, no further expansion needed
			UnpackToVector(hdr.ptr_primitives,	hdr.primitiveCount,	mesh.primitives,	file_);
			UnpackToVector(hdr.ptr_verticesXY,	hdr.vertexCount,	mesh.verticesXY,	file_);
			UnpackToVector(hdr.ptr_verticesZ,	hdr.vertexCount,	mesh.verticesZ,		file_);
			UnpackToVector(hdr.ptr_normals,		hdr.normalCount,	mesh.normals,		file_);
			UnpackToVector(hdr.ptr_shadings,	hdr.shadingCount,	mesh.shadings,		file_);
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
