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
	legal(true) {

	// `loaded(false)`: becomes true when successfully loaded into memory --> `UnpackIPDToMem()`
	//   `legal(true)`: stays true until shown not to be legal chunk

	// Check if specified file exists, case-sensitive!
	if (std::filesystem::exists(inFile) && inFile.extension().string() == ".IPD") {
		this->inFile_.path	= inFile;
	
		if (std::filesystem::exists(outFile)) {
			this->outFile_.path	= outFile;
		}
	} else {
		std::cerr << "[INFO]: Constructed chunk object but without path! " 
				  << "Note that file extensions are case-sensitive (.IPD)!" << std::endl;
		
		this->legal = false;
	}
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

		// Assume: Only one PLM file is used across a prefix group
		// Unpack model lookup tables, contiguous up until first scene/model buffer
		uint8_t fullModelCount = (header_.offset_IPD_SCENE_GROUP - header_.offset_IPD_MODEL_INFO) / sizeof(IPD_MODEL_INFO);
		modelLookupTable_.resize(fullModelCount);
		UnpackToVector(header_.offset_IPD_MODEL_INFO, fullModelCount, modelLookupTable_, inFile_);

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
		std::cout << "Fix existing integrity issues before unpacking!" << std::endl;
		return false;
	}
}

bool Chunk::PackChunkFromMem() {
	if (this->legal) {
		outFile_.stream.open(outFile_.path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!outFile_.stream) {
			std::cerr << "[ERROR]: Failed to open contents of new IPD file!" << std::endl;
			return false;
		}

		// Calculate all necessary offsets for header
		

		// Pack IPD header
		outFile_.stream.write("\x14\x00", 2);

		// Chunk major coordinates (XZ)
		outFile_.stream.write(reinterpret_cast<const char*>(&this->chunkMajorX_), 1);
		outFile_.stream.write(reinterpret_cast<const char*>(&this->chunkMajorZ_), 1);

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

int Chunk::GetSizeInMem() {
	// Comes later
	return false;
}

bool Chunk::IsValid() {
	return (header_.isValid() && this->legal);
}
