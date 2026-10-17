#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include "core/Chunk.h"
#include "core/Collision.h"
#include "core/DataStructs.h"
#include "core/GlobalObjects.h"
#include "core/DataOperations.h"
#include "core/TexturePool.h"
#include "mgmt/FileOperations.h"
#include "mgmt/HexOperations.h"
#include "structs/IPD.h"
#include "structs/LM.h"

// ~~~~~~~~~~~ INITIALISE BLANK CHUNK ~~~~~~~~~~~
Chunk::Chunk(
	const std::filesystem::path& inFile,
	const std::filesystem::path& outFile,
	TexturePool& texturePool,
	GlobalObjects& globalObjects)
	:
	texturePool_(texturePool),
	globalObjects_(globalObjects),
	loaded(false),
	legal(true)
{
	inFile_.path = inFile;
	outFile_.path = outFile;
}

Chunk::~Chunk() {}

// ~~~~~~~~~~~ READING IPD FILES ~~~~~~~~~~~
bool Chunk::UnpackChunkToMem() {
	if (this->legal) {
		// Open IPD file
		inFile_.stream.open(inFile_.path, std::ios::in | std::ios::binary);
		if (!inFile_.stream) {
			std::cerr << "[ERROR]: Failed to open contents of IPD file!" << std::endl;
			return false;
		}

		// Unpack IPD header
		UnpackToStruct(0, sizeof(IPD_HEADER), header_, inFile_);
		if (!(header_.isValid())) {
			std::cerr << "[ERROR]: Not a valid IPD header, value at position 0x00 is (as int) " << (int)(header_.id) << std::endl;
			return false;
		}

		this->chunkMajorX_ = header_.chunkX;
		this->chunkMajorZ_ = header_.chunkZ;

		// Unpack internal LM header
		uint32_t internalLMBaseOffset = header_.offset_LM_HEADER;
		
		UnpackToStruct(internalLMBaseOffset, sizeof(LM_HEADER), internalLMData_.header, inFile_);
		
		// Unpack internal LM data
		UnpackLMMaterials(internalLMBaseOffset, internalLMData_, texturePool_, inFile_);
		UnpackLMModels(internalLMBaseOffset, internalLMData_, texturePool_, inFile_);

		// Unpack model lookup tables, contiguous up until first scene/model buffer
		uint8_t modelLookupCount = (header_.offset_IPD_SCENE_GROUP - header_.offset_IPD_MODEL_INFO) / sizeof(IPD_MODEL_INFO);
		modelLookupTable_.resize(modelLookupCount);
		UnpackToVector(header_.offset_IPD_MODEL_INFO, modelLookupCount, modelLookupTable_, inFile_);

		// Unpack model render order
		UnpackToVector(header_.offset_LM_MODEL_ORDER, header_.count_LM_MODEL_ORDER, renderOrder_, inFile_);

		// Unpack visibility table
		UnpackToStruct(28, sizeof(IPD_VISIBILITY_TABLE), visibilityTable_, inFile_);

		// Unpack scene/model buffer tables
		sceneGroups_.resize(header_.count_IPD_SCENE_GROUP);

		for (size_t i = 0; i < header_.count_IPD_SCENE_GROUP; ++i) {
			auto& hdr = sceneGroups_[i].header;
			
			UnpackToStruct(header_.offset_IPD_SCENE_GROUP + i * sizeof(IPD_SCENE_GROUP), sizeof(IPD_SCENE_GROUP), hdr, inFile_);
			
			UnpackToVector(hdr.offset_IPD_SCENE_BILLBOARD, 		hdr.count_IPD_SCENE_BILLBOARD, 		sceneGroups_[i].billboards, 	inFile_);
			UnpackToVector(hdr.offset_IPD_SCENE_BOUNDING_BOX, 	hdr.count_IPD_SCENE_BOUNDING_BOX, 	sceneGroups_[i].boundingBoxes, 	inFile_);
			UnpackToVector(hdr.offset_IPD_SCENE_GROUP_INSTANCE, hdr.count_IPD_SCENE_GROUP_INSTANCE, sceneGroups_[i].groupInstances, inFile_);
		}

		// Unpack collision
		// We will untangle this later, keep in its original structure for now
		UnpackToStruct(sizeof(IPD_HEADER), sizeof(IPD_COLL_HEADER), collisionData_.header, inFile_);
		UnpackCollisionData(collisionData_, inFile_);

		inFile_.stream.close();

		this->loaded = true;
		std::cout << "Successfully unpacked " << inFile_.path << std::endl;

		return true;
	} else {
		std::cout << "[WARNING]: Fix existing integrity issues before unpacking!" << std::endl;
		return false;
	}
}

bool Chunk::RecalculateOffsets() {
	// For all headers, used as verification layer but only reasonable to implement once we successfully pack back to IPD
	// In-place editing may be preferred for small changes, if offsets stay the same, otherwise needs to recalculate all offsets and redo file

	// uint8_t  modelCount    = modelInfoSize / sizeof(IPD_MODEL_INFO);
	// header_.offset_IPD_SCENE_GROUP = 392 + 

	return true;
}

bool Chunk::PackChunkFromMem() {
	if (this->legal) {
		outFile_.stream.open(outFile_.path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!outFile_.stream) {

			std::cerr << "[WARNING]: Failed to open contents of new IPD file!" << std::endl;
			return false;
		}

		// Call recalculation of header/data offsets and sizes
		// By this point, we assume that headers/data are correctly resized (might change by adding new members to in-memory structs / cleaning up)

		// Need to fix padding/alignment! It is all implicit so far.
		// This is just a rough sketch and seems terrible/prone to breaking actually due to struct interdependence, and in its overall clarity.

		// Fixed block starts at 0, 84
		PackBinary(0, sizeof(IPD_HEADER), &header_, outFile_);							// Header
		PackBinary(84, sizeof(IPD_COLL_HEADER), &collisionData_.header, outFile_);		// Collision header

		// Model lookup table w/ fixed block start at 392
		for (size_t i = 0; i < modelLookupTable_.size(); ++i) {
			PackBinary(392 + sizeof(IPD_MODEL_INFO) * i, sizeof(IPD_MODEL_INFO), &modelLookupTable_[i], outFile_);
		}

		// Scene group header
		for (size_t i = 0; i < sceneGroups_.size(); ++i) {
			PackBinary(header_.offset_IPD_SCENE_GROUP + sizeof(IPD_SCENE_GROUP) * i, sizeof(IPD_SCENE_GROUP), &sceneGroups_[i].header, outFile_);
		}

		// Scene group instances/payload
		for (const auto& sceneGroup : sceneGroups_) {
			PackFromVector(sceneGroup.header.offset_IPD_SCENE_GROUP_INSTANCE, sceneGroup.groupInstances, outFile_);
			PackFromVector(sceneGroup.header.offset_IPD_SCENE_BILLBOARD, sceneGroup.billboards, outFile_);
			PackFromVector(sceneGroup.header.offset_IPD_SCENE_BOUNDING_BOX, sceneGroup.boundingBoxes, outFile_);
		}

		// Model order
		std::cout << IntToHexBE(header_.offset_LM_MODEL_ORDER) << std::endl;
		for (size_t i = 0; i < renderOrder_.size(); ++i) {
			PackBinary(header_.offset_LM_MODEL_ORDER + i, 1, &renderOrder_[i], outFile_);
		}

		// Internal LM header
		std::cout << IntToHexBE(header_.offset_LM_HEADER) << std::endl;
		uint32_t internalLMBaseOffset = header_.offset_LM_MODEL_ORDER + renderOrder_.size();
		PackBinary(internalLMBaseOffset, sizeof(LM_HEADER), &internalLMData_.header, outFile_);

		// ~~~~ PAYLOADS ~~~~ (coming later)		

		// Calculate offset_LM_HEADER
		outFile_.stream.close();
		return true;

	} else {
		std::cout << "Fix existing integrity issues before packing!" << std::endl;
		return false;
	}
}

int Chunk::GetSizeInPath() {
	std::error_code errorCode;
	auto size = std::filesystem::file_size(inFile_.path);

	if (errorCode) {
		std::cerr << "[ERROR]: " << errorCode.message() << std::endl;
		return 0;
	}

	return static_cast<int>(size);
}

bool Chunk::IsValid() {
	return (header_.isValid() && this->legal);
}
