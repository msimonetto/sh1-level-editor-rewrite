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
		UnpackLMModelOrder(internalLMBaseOffset, internalLMData_, inFile_);

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
		// We will untangle this further later, keep in its original structure for now
		UnpackToStruct(sizeof(IPD_HEADER), sizeof(IPD_COLL_HEADER), collisionData_.header, inFile_);
		UnpackCollision(collisionData_, inFile_);

		inFile_.stream.close();

		this->loaded = true;
		std::cout << "[SUCCESS]: Unpacked " << inFile_.path << " to memory!" << std::endl;

		return true;
	} else {
		std::cout << "[WARNING]: Fix existing integrity issues before unpacking!" << std::endl;
		return false;
	}
}

// ~~~~~~~~~~~ DYNAMIC EDITING ~~~~~~~~~~~
// Not immediately necessary for unpacking/packing, but will be important for dynamic packing when file expands or shrinks
bool Chunk::ReconcileHeaders() {
	// ~~ Miscellaneous ~~
	// Visibility table
	std::memcpy(header_.visibilityTable.subcells, visibilityTable_.subcells, sizeof(IPD_VISIBILITY_RANGE) * 25);

	// ~~ Counts ~~
	// Scene groups
    header_.count_IPD_SCENE_GROUP                   = sceneGroups_.size();
    for (auto& sceneGroup : sceneGroups_) {
    	sceneGroup.header.count_IPD_SCENE_GROUP_INSTANCE	= sceneGroup.groupInstances.size();
    	sceneGroup.header.count_IPD_SCENE_BILLBOARD			= sceneGroup.billboards.size();
    	sceneGroup.header.count_IPD_SCENE_BOUNDING_BOX 		= sceneGroup.boundingBoxes.size();
    }

    // Model order
	header_.count_LM_MODEL_ORDER                    = renderOrder_.size();

    // Collision
    collisionData_.header.count_IPD_COLL_VERTEX     = collisionData_.vertices.size();
    collisionData_.header.count_IPD_COLL_SURFACE    = collisionData_.surfaces.size();
    collisionData_.header.count_IPD_COLL_LINE       = collisionData_.lines.size();
    collisionData_.header.count_IPD_COLL_CYLINDER   = collisionData_.cylinders.size();
    collisionData_.header.count_verticalIdxPool   	= collisionData_.verticalIdxPool.size();
    collisionData_.header.count_floorIdxPool   		= collisionData_.floorIdxPool.size();

    // Internal LM
    internalLMData_.header.count_LM_MODEL_HEADER 	= internalLMData_.models.size();
	internalLMData_.header.count_LM_MATERIAL		= internalLMData_.materials.size();

	// ~~ Offsets ~~
	uint32_t currentOffset;
	auto ReconcileByVector = [&currentOffset](auto& assignedOffset, const auto& container, uint32_t anchor = 0) {
		currentOffset 	= AlignToDWORD(currentOffset);
		assignedOffset 	= currentOffset - anchor;
		currentOffset  += container.size() * sizeof(typename std::decay_t<decltype(container)>::value_type);
	};

	// Scene groups
	header_.offset_IPD_SCENE_GROUP 					= AlignToDWORD(header_.offset_IPD_MODEL_INFO + sizeof(IPD_MODEL_INFO) * modelLookupTable_.size());
	currentOffset 									= AlignToDWORD(header_.offset_IPD_SCENE_GROUP + sizeof(IPD_SCENE_GROUP) * sceneGroups_.size());
	for (auto& sceneGroup : sceneGroups_) {
		ReconcileByVector(sceneGroup.header.offset_IPD_SCENE_GROUP_INSTANCE,	sceneGroup.groupInstances);
		ReconcileByVector(sceneGroup.header.offset_IPD_SCENE_BILLBOARD,			sceneGroup.billboards);
		ReconcileByVector(sceneGroup.header.offset_IPD_SCENE_BOUNDING_BOX,		sceneGroup.boundingBoxes);
	}

	// Model order
	uint8_t lastSceneGroupElement 					= std::max(0, (int)(sceneGroups_.size() - 1));
	header_.offset_LM_MODEL_ORDER 					= sceneGroups_[lastSceneGroupElement].header.offset_IPD_SCENE_BOUNDING_BOX + sizeof(IPD_SCENE_BOUNDING_BOX);

	// Collision
	currentOffset 									= AlignToDWORD(header_.offset_LM_MODEL_ORDER + renderOrder_.size());
	ReconcileByVector(collisionData_.header.offset_IPD_COLL_VERTEX,         collisionData_.vertices, 		sizeof(IPD_HEADER));
	ReconcileByVector(collisionData_.header.offset_IPD_COLL_SURFACE,        collisionData_.surfaces,		sizeof(IPD_HEADER));
	ReconcileByVector(collisionData_.header.offset_IPD_COLL_LINE,           collisionData_.lines,			sizeof(IPD_HEADER));
	ReconcileByVector(collisionData_.header.offset_IPD_COLL_CYLINDER,       collisionData_.cylinders,		sizeof(IPD_HEADER));
	ReconcileByVector(collisionData_.header.offset_IPD_COLL_SUBCELL_LOOKUP,	collisionData_.subcellLookup,	sizeof(IPD_HEADER));
	ReconcileByVector(collisionData_.header.offset_verticalIdxPool,         collisionData_.verticalIdxPool, sizeof(IPD_HEADER));
	ReconcileByVector(collisionData_.header.offset_floorIdxPool,            collisionData_.floorIdxPool,	sizeof(IPD_HEADER));

	// Internal LM
	header_.offset_LM_HEADER 						= AlignToDWORD(currentOffset);

	// Reset offset relative to start of LM_HEADER (these are relative offsets onwards)
	currentOffset 									= sizeof(LM_HEADER);
	ReconcileByVector(internalLMData_.header.offset_LM_MATERIAL, 			internalLMData_.materials);
	ReconcileByVector(internalLMData_.header.offset_LM_MODEL_HEADER, 		internalLMData_.models);

	for (auto& model : internalLMData_.models) {
	    currentOffset = AlignToDWORD(currentOffset);
	    model.header.offset_LM_MESH_HEADER = currentOffset;

	    currentOffset += model.meshes.size() * sizeof(LM_MESH_HEADER);

	    for (auto& mesh : model.meshes) {
	        ReconcileByVector(mesh.header.offset_LM_PRIMITIVE, mesh.primitives);
	        ReconcileByVector(mesh.header.offset_VertexXZ,     mesh.verticesXZ);
	        ReconcileByVector(mesh.header.offset_VertexY,      mesh.verticesY);
	        ReconcileByVector(mesh.header.offset_LM_NORMAL,    mesh.normals);
	        ReconcileByVector(mesh.header.offset_Shading,      mesh.shadings);
	    }
	}

    ReconcileByVector(internalLMData_.header.offset_LM_MODEL_ORDER,			internalLMData_.modelOrder);

	return true;
}

bool Chunk::PackChunkFromMem() {
	if (this->legal) {
		this->ReconcileHeaders();

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
		assert(sceneGroups_[0].header.offset_IPD_SCENE_GROUP_INSTANCE == AlignToDWORD(header_.offset_IPD_SCENE_GROUP + sizeof(IPD_SCENE_GROUP) * sceneGroups_.size()));
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
		PackLMModelOrder(header_.offset_LM_HEADER, internalLMData_, outFile_);

		// Zero out until alignment to next 2048 byte sector (just how the file table works)
		outFile_.stream.seekg(0, std::ios::end);
		uint32_t finalPosition 		= static_cast<uint32_t>(outFile_.stream.tellp());
		uint32_t alignedPosition 	= (finalPosition + 2047) & ~2047;
		if (alignedPosition > finalPosition) {
			std::vector<char> pad(alignedPosition - finalPosition, 0);
			PackBinary(finalPosition, pad.size(), pad.data(), outFile_);
		}

		outFile_.stream.close();

		std::cout << "[SUCCESS]: Packed " << inFile_.path << " to IPD!" << std::endl;
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
