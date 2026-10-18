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
	inMemory_(false),
	inViewport_(false),
	legal_(true)
{
	inFile_.path = NormalisePath(inFile.string());
	outFile_.path = NormalisePath(outFile.string());

	assert(inFile.stem() == outFile.stem());
}

Chunk::~Chunk() {}

// ~~~~~~~~~~~ READING IPD FILES ~~~~~~~~~~~
bool Chunk::Parse() {
	if (this->legal_) {
		inFile_.stream.open(inFile_.path, std::ios::in | std::ios::binary);
		if (!inFile_.stream) {
			std::cerr << "[ERROR]: Failed to access contents of " << inFile_.path.stem().string() + ".IPD" << std::endl;
			return false;
		}

		// IPD header
		UnpackBinary(0, sizeof(IPD_HEADER), &header_, inFile_);
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

		// Internal LM data
		if (!ParseLMData(header_.offset_LM_HEADER, internalLMData_, inFile_)) { return false; }

		// Model lookup tables, contiguous up until first scene/model buffer
		uint8_t modelLookupCount = (header_.offset_IPD_SCENE_GROUP - header_.offset_IPD_MODEL_INFO) / sizeof(IPD_MODEL_INFO);
		modelLookupTable_.resize(modelLookupCount);
		if (!UnpackToVector(header_.offset_IPD_MODEL_INFO, modelLookupCount, modelLookupTable_, inFile_)) { return false; }

		// Model render order
		if (!UnpackToVector(header_.offset_LM_MODEL_ORDER, header_.count_LM_MODEL_ORDER, renderOrder_, inFile_)) { return false; }

		// Visibility table (stored separately, even though in header)
		if (!UnpackBinary(28, sizeof(IPD_VISIBILITY_TABLE), &visibilityTable_, inFile_)) { return false; }

		// Scene/model buffer tables
		sceneGroups_.resize(header_.count_IPD_SCENE_GROUP);

		for (size_t i = 0; i < header_.count_IPD_SCENE_GROUP; ++i) {
			auto& hdr = sceneGroups_[i].header;
			
			if (!UnpackBinary(header_.offset_IPD_SCENE_GROUP + i * sizeof(IPD_SCENE_GROUP), sizeof(IPD_SCENE_GROUP), &hdr, inFile_)) { return false; }
			
			if (!UnpackToVector(hdr.offset_IPD_SCENE_GROUP_INSTANCE,	hdr.count_IPD_SCENE_GROUP_INSTANCE, sceneGroups_[i].groupInstances, inFile_) ||
				!UnpackToVector(hdr.offset_IPD_SCENE_BILLBOARD, 		hdr.count_IPD_SCENE_BILLBOARD, 		sceneGroups_[i].billboards, 	inFile_) ||
				!UnpackToVector(hdr.offset_IPD_SCENE_BOUNDING_BOX, 		hdr.count_IPD_SCENE_BOUNDING_BOX, 	sceneGroups_[i].boundingBoxes, 	inFile_)) {
				return false;
			}
		}

		// Collision
		// We will untangle this further later, keep in its original structure for now
		if (!UnpackBinary(sizeof(IPD_HEADER), sizeof(IPD_COLL_HEADER), &collisionData_.header, inFile_)) { return false; }
		if (!ParseCollision(collisionData_, inFile_)) { return false; }

		inFile_.stream.close();

		this->inMemory_ = true;
		std::cout << "[SUCCESS]: Parsed " << inFile_.path.stem().string() + ".IPD" << std::endl;

		return true;
	} else {
		std::cerr << "[WARNING]: Fix existing integrity issues before parsing " << inFile_.path.stem().string() + ".IPD" << std::endl;
		return false;
	}
}

// ~~~~~~~~~~~ WRITING IPD FILES ~~~~~~~~~~~
bool Chunk::Write() {
	if (this->legal_) {
		this->ReconcileHeaders();

		outFile_.stream.open(outFile_.path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!outFile_.stream) {
			std::cerr << "[WARNING]: Failed to access contents of " << outFile_.path.stem().string() << + ".IPD" << std::endl;
			return false;
		}

		// Fixed block starts at 0, 84
		if (!PackBinary(0, sizeof(IPD_HEADER), &header_, outFile_) ||										// Header
			!PackBinary(sizeof(IPD_HEADER), sizeof(IPD_COLL_HEADER), &collisionData_.header, outFile_)) {	// Collision header
			return false;
		}

		// Model lookup table w/ fixed block start at 392 (sizeof(IPD_HEADER) + sizeof(IPD_COLL_HEADER))
		for (size_t i = 0; i < modelLookupTable_.size(); ++i) {
			if (!PackBinary(sizeof(IPD_HEADER) + sizeof(IPD_COLL_HEADER) + sizeof(IPD_MODEL_INFO) * i, sizeof(IPD_MODEL_INFO), &modelLookupTable_[i], outFile_)) { return false; }
		}

		// Scene group header
		assert(header_.offset_IPD_SCENE_GROUP == header_.offset_IPD_MODEL_INFO + sizeof(IPD_MODEL_INFO) * modelLookupTable_.size());
		for (size_t i = 0; i < sceneGroups_.size(); ++i) {
			if (!PackBinary(header_.offset_IPD_SCENE_GROUP + sizeof(IPD_SCENE_GROUP) * i, sizeof(IPD_SCENE_GROUP), &sceneGroups_[i].header, outFile_)) { return false; }
		}

		// Scene group instances/payload
		assert(sceneGroups_[0].header.offset_IPD_SCENE_GROUP_INSTANCE == AlignToDWORD(header_.offset_IPD_SCENE_GROUP + sizeof(IPD_SCENE_GROUP) * sceneGroups_.size()));
		for (const auto& sceneGroup : sceneGroups_) {
			if (!PackFromVector(sceneGroup.header.offset_IPD_SCENE_GROUP_INSTANCE, 	sceneGroup.groupInstances, 	outFile_) ||
				!PackFromVector(sceneGroup.header.offset_IPD_SCENE_BILLBOARD, 		sceneGroup.billboards, 		outFile_) ||
				!PackFromVector(sceneGroup.header.offset_IPD_SCENE_BOUNDING_BOX, 	sceneGroup.boundingBoxes, 	outFile_)) {
				return false;
			}
		}

		// Model order payload
		uint8_t lastSceneGroupElement = std::max(0, (int)(sceneGroups_.size() - 1));
		assert(header_.offset_LM_MODEL_ORDER == sceneGroups_[lastSceneGroupElement].header.offset_IPD_SCENE_BOUNDING_BOX + sizeof(IPD_SCENE_BOUNDING_BOX));
		for (size_t i = 0; i < renderOrder_.size(); ++i) {
			PackBinary(header_.offset_LM_MODEL_ORDER + i, 1, &renderOrder_[i], outFile_);
		}

		// Collision data payload (relative to +IPD_COLL_HEADER so increment by 84 bytes)
		assert((uint32_t)(collisionData_.header.offset_IPD_COLL_VERTEX + sizeof(IPD_HEADER)) == AlignToDWORD(header_.offset_LM_MODEL_ORDER + renderOrder_.size()));
		WriteCollision(collisionData_, outFile_);
		uint16_t collisionDataSize = collisionData_.header.offset_floorIdxPool - collisionData_.header.offset_IPD_COLL_VERTEX;

		// Internal LM header (allowance for padding from collision data payload, might change)
		assert(collisionData_.header.offset_IPD_COLL_VERTEX + sizeof(IPD_HEADER) + collisionDataSize <= header_.offset_LM_HEADER);
		WriteLMData(header_.offset_LM_HEADER, internalLMData_, outFile_);

		AlignFileToSector(outFile_);

		outFile_.stream.close();

		std::cout << "[SUCCESS]: Wrote " << outFile_.path.stem().string() + ".IPD" << std::endl;
		return true;

	} else {
		std::cerr << "[WARNING]: Fix existing integrity issues before writing " << outFile_.path.stem().string() + ".IPD" << std::endl;
		return false;
	}
}

bool Chunk::GetDependencies() {
	// Identifies dependent global models
	// std::cout << "e" << std::endl;
	// for (const auto& modelEntry : modelLookupTable_) {
	// 	std::cout << "c" << std::endl;
	// 	if (modelEntry.isGlobalPlm) {
	// 		std::cout << "a" << std::endl;
	// 		dependentGlobalObjects_.insert(modelEntry.name.str);
	// 	}
	// }
	//
	// Identifies dependent textures from global models
	//
	// Identifies dependent textures from internally embedded models
	// for (const auto& material : internalLMData_.materials) {
	//	std::cout << "b" << std::endl;
	//	std::cout << material.name.str << std::endl;
	//	dependentTextures_.insert(material.name.str);
	//	texturePool_.AcquireTexture(material.name.str);
	//}

	return true;
}

// ~~~~~~~~~~~ DYNAMIC EDITING ~~~~~~~~~~~
// Not immediately necessary for unpacking/packing, but will be important for dynamic packing when file size expands or shrinks
bool Chunk::ReconcileHeaders() {
	uint32_t currentOffset;

	// Visibility table
	std::memcpy(header_.visibilityTable.subcells, visibilityTable_.subcells, sizeof(IPD_VISIBILITY_RANGE) * 25);

	// Scene groups
    header_.count_IPD_SCENE_GROUP                   		= sceneGroups_.size();
    for (auto& sceneGroup : sceneGroups_) {
    	sceneGroup.header.count_IPD_SCENE_GROUP_INSTANCE	= sceneGroup.groupInstances.size();
    	sceneGroup.header.count_IPD_SCENE_BILLBOARD			= sceneGroup.billboards.size();
    	sceneGroup.header.count_IPD_SCENE_BOUNDING_BOX 		= sceneGroup.boundingBoxes.size();
    }

	header_.offset_IPD_SCENE_GROUP 					= AlignToDWORD(header_.offset_IPD_MODEL_INFO + sizeof(IPD_MODEL_INFO) * modelLookupTable_.size());
	currentOffset 									= AlignToDWORD(header_.offset_IPD_SCENE_GROUP + sizeof(IPD_SCENE_GROUP) * sceneGroups_.size());
	for (auto& sceneGroup : sceneGroups_) {
		ReconcilePush(currentOffset, sceneGroup.header.offset_IPD_SCENE_GROUP_INSTANCE,		sceneGroup.groupInstances);
		ReconcilePush(currentOffset, sceneGroup.header.offset_IPD_SCENE_BILLBOARD,			sceneGroup.billboards);
		ReconcilePush(currentOffset, sceneGroup.header.offset_IPD_SCENE_BOUNDING_BOX,		sceneGroup.boundingBoxes);
	}

    // Model order
	header_.count_LM_MODEL_ORDER                    = renderOrder_.size();

	uint8_t lastSceneGroupElement 					= std::max(0, (int)(sceneGroups_.size() - 1));
	header_.offset_LM_MODEL_ORDER 					= sceneGroups_[lastSceneGroupElement].header.offset_IPD_SCENE_BOUNDING_BOX + sizeof(IPD_SCENE_BOUNDING_BOX);

    // Collision
    currentOffset                                   = AlignToDWORD(header_.offset_LM_MODEL_ORDER + renderOrder_.size());
	currentOffset 									= ReconcileCollision(currentOffset, collisionData_);

    // Internal LM
    header_.offset_LM_HEADER 						= AlignToDWORD(currentOffset);
    currentOffset									= sizeof(LM_HEADER);
    currentOffset									= ReconcileLMData(currentOffset, internalLMData_);

	return true;
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
	return (header_.isValid() && this->legal_);
}