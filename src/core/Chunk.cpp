#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <cassert>
#include <cstring>
#include <algorithm>

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

	assert(inFile.stem() == outFile.stem());
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

		// Ensure filename and chunk major coordinates are congruent
		this->chunkMajorX_ = header_.chunkX;
		this->chunkMajorZ_ = header_.chunkZ;

		std::string chunkHash = std::format("{:04X}", ChunkHash(chunkMajorX_, chunkMajorZ_));
		std::string filenameStem = inFile_.path.stem().string();
		std::string filenameChunkHash = filenameStem.substr(filenameStem.length() - 4);
		assert(chunkHash == filenameChunkHash);

		// Unpack internal LM header
		uint32_t internalLMBaseOffset = header_.offset_LM_HEADER;
		
		UnpackToStruct(internalLMBaseOffset, sizeof(LM_HEADER), internalLMData_.header, inFile_);
		
		// Internal LM data
		UnpackLMMaterials(internalLMBaseOffset, internalLMData_, texturePool_, inFile_);
		UnpackLMModels(internalLMBaseOffset, internalLMData_, texturePool_, inFile_);

		// Model lookup tables, contiguous up until first scene/model buffer
		uint8_t modelLookupCount = (header_.offset_IPD_SCENE_GROUP - header_.offset_IPD_MODEL_INFO) / sizeof(IPD_MODEL_INFO);
		modelLookupTable_.resize(modelLookupCount);
		UnpackToVector(header_.offset_IPD_MODEL_INFO, modelLookupCount, modelLookupTable_, inFile_);

		// Model render order
		UnpackToVector(header_.offset_LM_MODEL_ORDER, header_.count_LM_MODEL_ORDER, renderOrder_, inFile_);

		// Visibility table (stored separately, even though in header)
		UnpackToStruct(28, sizeof(IPD_VISIBILITY_TABLE), visibilityTable_, inFile_);

		// Scene/model buffer tables
		sceneGroups_.resize(header_.count_IPD_SCENE_GROUP);

		for (size_t i = 0; i < header_.count_IPD_SCENE_GROUP; ++i) {
			auto& hdr = sceneGroups_[i].header;
			
			UnpackToStruct(header_.offset_IPD_SCENE_GROUP + i * sizeof(IPD_SCENE_GROUP), sizeof(IPD_SCENE_GROUP), hdr, inFile_);
			
			UnpackToVector(hdr.offset_IPD_SCENE_GROUP_INSTANCE, hdr.count_IPD_SCENE_GROUP_INSTANCE, sceneGroups_[i].groupInstances, inFile_);
			UnpackToVector(hdr.offset_IPD_SCENE_BILLBOARD, 		hdr.count_IPD_SCENE_BILLBOARD, 		sceneGroups_[i].billboards, 	inFile_);
			UnpackToVector(hdr.offset_IPD_SCENE_BOUNDING_BOX, 	hdr.count_IPD_SCENE_BOUNDING_BOX, 	sceneGroups_[i].boundingBoxes, 	inFile_);
		}

		// Collision
		// We will untangle this later, keep in its original structure for now
		UnpackToStruct(sizeof(IPD_HEADER), sizeof(IPD_COLL_HEADER), collisionData_.header, inFile_);
		UnpackCollision(collisionData_, inFile_);

		inFile_.stream.close();

		this->loaded = true;
		std::cout << "Successfully unpacked " << inFile_.path << std::endl;

		return true;
	} else {
		std::cout << "[WARNING]: Fix existing integrity issues before unpacking!" << std::endl;
		return false;
	}
}

// ~~~~~~~~~~~ UPDATING VALUES UPON CHANGE ~~~~~~~~~~~
bool Chunk::ReconcileCounts() {
	// Visibility table
	std::memcpy(header_.visibilityTable.subcells, visibilityTable_.subcells, sizeof(IPD_VISIBILITY_RANGE) * 25);

	// Counts
	header_.count_LM_MODEL_ORDER                    = renderOrder_.size();
    header_.count_IPD_SCENE_GROUP                   = sceneGroups_.size();
    for (auto& sceneGroup : sceneGroups_) {
    	sceneGroup.header.count_IPD_SCENE_GROUP_INSTANCE	= sceneGroup.groupInstances.size();
    	sceneGroup.header.count_IPD_SCENE_BILLBOARD			= sceneGroup.billboards.size();
    	sceneGroup.header.count_IPD_SCENE_BOUNDING_BOX 		= sceneGroup.boundingBoxes.size();
    }

    collisionData_.header.count_IPD_COLL_VERTEX     = collisionData_.vertices.size();
    collisionData_.header.count_IPD_COLL_SURFACE    = collisionData_.surfaces.size();
    collisionData_.header.count_IPD_COLL_LINE       = collisionData_.lines.size();
    collisionData_.header.count_IPD_COLL_CYLINDER   = collisionData_.cylinders.size();
    collisionData_.header.count_verticalIdxPool   	= collisionData_.verticalIdxPool.size();
    collisionData_.header.count_floorIdxPool   		= collisionData_.floorIdxPool.size();

    internalLMData_.header.count_LM_MODEL_HEADER 	= internalLMData_.models.size();
	internalLMData_.header.count_LM_MATERIAL		= internalLMData_.materials.size();

	return true;
}

bool Chunk::ReconcileOffsets() {
	// Offsets
	header_.offset_IPD_SCENE_GROUP 							= AlignToDWORD(header_.offset_IPD_MODEL_INFO + sizeof(IPD_MODEL_INFO) * modelLookupTable_.size());
	uint32_t currentOffset 									= AlignToDWORD(header_.offset_IPD_SCENE_GROUP + sizeof(IPD_SCENE_GROUP) * sceneGroups_.size());

	for (auto& sceneGroup : sceneGroups_) {
		currentOffset = AlignToDWORD(currentOffset);
	    sceneGroup.header.offset_IPD_SCENE_GROUP_INSTANCE 	= currentOffset;
	    currentOffset += sceneGroup.groupInstances.size() 	* sizeof(IPD_SCENE_GROUP_INSTANCE);

	    currentOffset = AlignToDWORD(currentOffset);
	    sceneGroup.header.offset_IPD_SCENE_BILLBOARD 		= currentOffset;
	    currentOffset += sceneGroup.billboards.size() 		* sizeof(IPD_SCENE_BILLBOARD);

	    currentOffset = AlignToDWORD(currentOffset);
	    sceneGroup.header.offset_IPD_SCENE_BOUNDING_BOX 	= currentOffset;
	    currentOffset += sceneGroup.boundingBoxes.size() 	* sizeof(IPD_SCENE_BOUNDING_BOX);
	}

	uint8_t lastSceneGroupElement = std::max(0, (int)(sceneGroups_.size()-1));
	header_.offset_LM_MODEL_ORDER 							= sceneGroups_[lastSceneGroupElement].header.offset_IPD_SCENE_BOUNDING_BOX + sizeof(IPD_SCENE_BOUNDING_BOX);

	uint32_t collisionPayloadOffset = AlignToDWORD(header_.offset_LM_MODEL_ORDER + renderOrder_.size());
	collisionData_.header.offset_IPD_COLL_VERTEX = (uint32_t)(collisionPayloadOffset - sizeof(IPD_HEADER));

	return true;
}

bool Chunk::PackChunkFromMem() {
	if (this->legal) {
		this->ReconcileCounts();
		this->ReconcileOffsets();

		outFile_.stream.open(outFile_.path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!outFile_.stream) {

			std::cerr << "[WARNING]: Failed to open contents of new IPD file!" << std::endl;
			return false;
		}

		// Fixed block starts at 0, 84
		PackBinary(0, sizeof(IPD_HEADER), &header_, outFile_);											// Header
		PackBinary(sizeof(IPD_HEADER), sizeof(IPD_COLL_HEADER), &collisionData_.header, outFile_);		// Collision header

		// Model lookup table w/ fixed block start at 392 (sizeof(IPD_HEADER) + sizeof(IPD_COLL_HEADER))
		for (size_t i = 0; i < modelLookupTable_.size(); ++i) {
			PackBinary(sizeof(IPD_HEADER) + sizeof(IPD_COLL_HEADER) + sizeof(IPD_MODEL_INFO) * i, sizeof(IPD_MODEL_INFO), &modelLookupTable_[i], outFile_);
		}

		// Scene group header
		assert(header_.offset_IPD_SCENE_GROUP == header_.offset_IPD_MODEL_INFO + sizeof(IPD_MODEL_INFO) * modelLookupTable_.size());
		for (size_t i = 0; i < sceneGroups_.size(); ++i) {
			PackBinary(header_.offset_IPD_SCENE_GROUP + sizeof(IPD_SCENE_GROUP) * i, sizeof(IPD_SCENE_GROUP), &sceneGroups_[i].header, outFile_);
		}

		// Scene group instances/payload
		assert(sceneGroups_[0].header.offset_IPD_SCENE_GROUP_INSTANCE == header_.offset_IPD_SCENE_GROUP + sizeof(IPD_SCENE_GROUP) * modelLookupTable_.size());
		for (const auto& sceneGroup : sceneGroups_) {
			PackFromVector(sceneGroup.header.offset_IPD_SCENE_GROUP_INSTANCE, 	sceneGroup.groupInstances, 	outFile_);
			PackFromVector(sceneGroup.header.offset_IPD_SCENE_BILLBOARD, 		sceneGroup.billboards, 		outFile_);
			PackFromVector(sceneGroup.header.offset_IPD_SCENE_BOUNDING_BOX, 	sceneGroup.boundingBoxes, 	outFile_);
		}

		// Model order payload
		uint8_t lastSceneGroupElement = std::max(0, (int)(sceneGroups_.size()-1));
		assert(header_.offset_LM_MODEL_ORDER == sceneGroups_[lastSceneGroupElement].header.offset_IPD_SCENE_BOUNDING_BOX + sizeof(IPD_SCENE_BOUNDING_BOX));
		for (size_t i = 0; i < renderOrder_.size(); ++i) {
			PackBinary(header_.offset_LM_MODEL_ORDER + i, 1, &renderOrder_[i], outFile_);
		}

		// Collision data payload (relative to +IPD_COLL_HEADER so increment by 84 bytes)
		assert((uint32_t)(collisionData_.header.offset_IPD_COLL_VERTEX + sizeof(IPD_HEADER)) == AlignToDWORD(header_.offset_LM_MODEL_ORDER + renderOrder_.size()));
		PackCollision(collisionData_, outFile_);
		uint16_t collisionDataSize = collisionData_.header.offset_floorIdxPool - collisionData_.header.offset_IPD_COLL_VERTEX;

		// Internal LM header (allowance for padding from collision data payload)
		assert(collisionData_.header.offset_IPD_COLL_VERTEX + sizeof(IPD_HEADER) + collisionDataSize <= header_.offset_LM_HEADER);
		PackBinary(header_.offset_LM_HEADER, sizeof(LM_HEADER), &internalLMData_.header, outFile_);

		// Internal LM payload
		PackLMMaterials(header_.offset_LM_HEADER, internalLMData_, texturePool_, outFile_);
		PackLMModels(header_.offset_LM_HEADER, internalLMData_, texturePool_, outFile_);

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
