#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <format>
#include <bit>
#include <filesystem>
#include <unordered_map>

#include "core/FileManager.h"
#include "formats/IPD.h"
#include "formats/LM.h"
#include "tools/IPDTools.h"

namespace IPDTools {

IPDChunk::IPDChunk(const std::string& path) {
	this->loaded = false;

	// Check if file name/path is valid
	if (std::filesystem::exists(path)
			&& FileManager::IsSupportedExtension(path)) {
		this->path_  = path;
		this->legal = true;
		std::cout << "[INFO]: Constructed chunk object from path!" << std::endl;
	} else {
		std::cerr << "[INFO]: Constructed chunk object but without path!" << std::endl;
		this->legal = false;
	}
}

IPDChunk::~IPDChunk() {
	std::cout << "[INFO]: Destroyed chunk object!" << std::endl;
}

// ~~~~~~~~~~~ READING IPD FILES ~~~~~~~~~~~
// Extracting binary sections between sections, repeated steps used in `UnpackIpdToMem()`
// Assume: Only reading from current chunk file (file_)
template <typename T>
void IPDChunk::UnpackToStruct(unsigned int baseOffset, unsigned int length, T& outStruct) {
	this->offset_ = baseOffset;
	file_.seekg(offset_, std::ios::beg);
	file_.read(reinterpret_cast<char*>(&outStruct), length);

	if (!(file_)) {
		std::cerr << "[ERROR]: Failed to unpack from offset " << baseOffset << std::endl;
	}
}

template <typename T, typename ptr_T>
void IPDChunk::UnpackToVector(ptr_T basePointer, size_t count, std::vector<T>& structVector) {
	structVector.resize(count);
	if (count == 0 || !basePointer) { return; }

	for (size_t i = 0; i < count; ++i) {
		IPDChunk::UnpackToStruct(basePointer + i * sizeof(T), sizeof(T), structVector[i]);
	}
}

// Binary unpacking, following hierarchial order and using dynamically allocated data structs
int IPDChunk::UnpackIPDToMem() {
	// Opens 'valid' IPD file (`path_`)
	file_.open(path_, std::ios::in | std::ios::binary);
	if (!file_) {
		std::cerr << "[ERROR]: Unable to open IPD file!" << std::endl;
		return -1;
	}

	// Unpack into `header_` and verify that the first word is 0x14
	// That was my first word as a baby
	IPDChunk::UnpackToStruct(0, sizeof(IPD_HEADER), header_);
	if (!(header_.isValid())) {
		std::cerr << "[ERROR]: Not a valid IPD header, offset (int) = " << (int)(header_.id) << std::endl;
		return -1;
	}

	// Unpack into `localLMData_`'s member header
	IPDChunk::UnpackToStruct(this->header_.ptr_LM_HEADER, sizeof(LM_HEADER), localLMData_.header);

	// Use `localLMData_`'s member header to populate material and model data structs
	// This will be performed iteratively

	// 1. Local LM -> Materials -> Textures (later, careful with mapping as it is not one-to-one)
	// Resize materials to materialCount
	localLMData_.materials.resize(localLMData_.header.materialCount);

	for (size_t i = 0; i < localLMData_.header.materialCount; ++i) {
		auto& material = localLMData_.materials[i];

		// Take base position AND count of Local LM's materials (via header) to set individual materials
		IPDChunk::UnpackToStruct(
			localLMData_.header.ptr_materials + i * sizeof(LM_MATERIAL),
			sizeof(LM_MATERIAL),
			material
		);

		// See if the material's texture is contained in texture pool (unordered_map), if not, add it
		if (!localLMData_.texturePool.contains(material.ptr_texture)) {
			LM_TEXTURE uniqueTexture;

			IPDChunk::UnpackToStruct(
				material.ptr_texture,
				sizeof(LM_TEXTURE),
				uniqueTexture
			);

			localLMData_.texturePool.insert({material.ptr_texture, uniqueTexture});
		}
	}

	// 2. Local LM -> Models -> Meshes
	// Resize models (ModelData) to modelCount
	localLMData_.models.resize(localLMData_.header.modelCount);
	for (size_t i = 0; i < localLMData_.header.modelCount; ++i) {
		auto& model = localLMData_.models[i];

		// Take base position AND count of Local LM's models (via header) to set individual model headers
		// models (ModelData) is initialised with headers (LM_MODEL_HEADER)
		IPDChunk::UnpackToStruct(
			localLMData_.header.ptr_modelHdrs + i * sizeof(LM_MODEL_HEADER),
			sizeof(LM_MODEL_HEADER),
			model.header
		);

		// Resize meshes (MeshData) to meshCount
		model.meshes.resize(model.header.meshCount);
		for (size_t j = 0; j < model.header.meshCount; ++j) {
			auto& mesh = model.meshes[j];

			// Take base position AND count of models.at(i)'s meshes (via header) to set individual mesh headers
			// Unpack a single mesh header
			IPDChunk::UnpackToStruct(
				model.header.ptr_meshHdrs + j * sizeof(LM_MESH_HEADER),
				sizeof(LM_MESH_HEADER),
				mesh.header
			);

			const auto& hdr = mesh.header;

			// Unpack the constituent parts of each mesh from its header
			UnpackToVector(hdr.ptr_primitives,	hdr.primitiveCount,		mesh.primitives);
			UnpackToVector(hdr.ptr_verticesXY,	hdr.vertexCount,		mesh.verticesXY);
			UnpackToVector(hdr.ptr_verticesZ,	hdr.vertexCount,		mesh.verticesZ);
			UnpackToVector(hdr.ptr_normals,		hdr.normalCount,		mesh.normals);
			UnpackToVector(hdr.ptr_shadings,	hdr.shadingCount,		mesh.shadings);
		}
	}


	return 0;
}

int IPDChunk::GetSizeInPath() {
	std::error_code errorCode;
	auto size = std::filesystem::file_size(path_, errorCode);

	if (errorCode) {
		std::cerr << "[ERROR]: " << errorCode.message() << std::endl;
		return 0;
	}

	return static_cast<int>(size);
}

int IPDChunk::GetSizeInMem() {
	// Comes later
	return 0;
}

int IPDChunk::IsValid() {
	return header_.isValid();
}

}
