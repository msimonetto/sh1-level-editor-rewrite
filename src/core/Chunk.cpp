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
#include "mgmt/FileManager.h"
#include "structs/IPD.h"
#include "structs/LM.h"

// ~~~~~~~~~~~ INITIALISE BLANK CHUNK ~~~~~~~~~~~
Chunk::Chunk(const std::filesystem::path& sourceDir, const std::string& filename,
	         TexturePool& texturePool, GlobalObjects& globalObjects)
		   : texturePool_(texturePool), globalObjects_(globalObjects), loaded(false), legal(true) {

	// `loaded(false)`: becomes true when successfully loaded into memory --> `UnpackIPDToMem()`
	//   `legal(true)`: stays true until shown not to be legal chunk

	// Check if specified file exists, case-sensitive!
	std::filesystem::path fpath = sourceDir / filename;
	if (std::filesystem::exists(fpath) && fpath.extension().string() == ".IPD") {
		this->sourceDir_  	= sourceDir;
		this->filename_ 	= filename;
		this->file_.path	= fpath;

		// std::cout << "[INFO]: Constructed chunk object from path!" << std::endl;
	} else {
		std::cerr << "[INFO]: Constructed chunk object but without path! " 
				  << "Note that file extensions are case-sensitive (.IPD)!" << std::endl;
		
		this->legal = false;
	}
}

Chunk::~Chunk() {
	// Need to decrement value in texture pool!
	// std::cout << "[INFO]: Destroyed chunk object!" << std::endl;
}

// ~~~~~~~~~~~ READING IPD FILES ~~~~~~~~~~~
bool Chunk::UnpackChunkToMem() {

	// Open IPD file
	file_.stream.open(file_.path, std::ios::in | std::ios::binary);
	if (!file_.stream) {
		std::cerr << "[ERROR]: Failed to open contents of IPD file!" << std::endl;
		return false;
	}

	// Unpack IPD header
	UnpackToStruct(0, sizeof(IPD_HEADER), header_, file_);
	if (!(header_.isValid())) {
		std::cerr << "[ERROR]: Not a valid IPD header, value at position 0x00 is (as int) " << (int)(header_.id) << std::endl;
		return false;
	}

	this->chunkMajorX_ = header_.chunkX;
	this->chunkMajorZ_ = header_.chunkZ;

	// Unpack internal LM header
	uint32_t internalLMBaseOffset = header_.offset_LM_HEADER;
	UnpackToStruct(internalLMBaseOffset, sizeof(LM_HEADER), internalLMData_.header, file_);
	
	// Unpack internal LM data
	UnpackLMMaterials(internalLMBaseOffset, internalLMData_, texturePool_, file_);
	UnpackLMModels(internalLMBaseOffset, internalLMData_, texturePool_, file_);

	// Assume: Only one PLM file is used across a prefix group
	// Unpack model lookup tables, contiguous up until first scene/model buffer
	uint8_t fullModelCount = (header_.offset_IPD_SCENE_GROUP - header_.offset_IPD_MODEL_INFO) / sizeof(IPD_MODEL_INFO);
	modelLookupTable_.resize(fullModelCount);
	UnpackToVector(header_.offset_IPD_MODEL_INFO, fullModelCount, modelLookupTable_, file_);

	// Unpack model render order
	UnpackToVector(header_.offset_LM_MODEL_ORDER, header_.count_LM_MODEL_ORDER, renderOrder_, file_);

	// Unpack visibility table
	UnpackToStruct(28, sizeof(IPD_VISIBILITY_TABLE), visibilityTable_, file_);

	// Unpack scene/model buffer tables
	sceneGroups_.resize(header_.count_IPD_SCENE_GROUP);

	for (size_t i = 0; i < header_.count_IPD_SCENE_GROUP; ++i) {
		auto& hdr = sceneGroups_[i].header;
		
		UnpackToStruct(header_.offset_IPD_SCENE_GROUP + i * sizeof(IPD_SCENE_GROUP), sizeof(IPD_SCENE_GROUP), hdr, file_);
		
		UnpackToVector(hdr.offset_IPD_SCENE_BILLBOARD, 		hdr.count_IPD_SCENE_BILLBOARD, 		sceneGroups_[i].billboards, 	file_);
		UnpackToVector(hdr.offset_IPD_SCENE_BOUNDING_BOX, 	hdr.count_IPD_SCENE_BOUNDING_BOX, 	sceneGroups_[i].boundingBoxes, 	file_);
		UnpackToVector(hdr.offset_IPD_SCENE_GROUP_INSTANCE, hdr.count_IPD_SCENE_GROUP_INSTANCE, sceneGroups_[i].groupInstances, file_);
	}

	// Unpack collision
	// We will untangle this later, keep in its original structure for now
	UnpackToStruct(sizeof(IPD_HEADER), sizeof(IPD_COLL_HEADER), collisionData_.header, file_);
	UnpackCollisionData(collisionData_, file_);

	this->loaded = true;

	return true;
}

bool Chunk::PackMemToIPD() {
	return false;
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
	return false;
}

bool Chunk::IsValid() {
	return (header_.isValid() && this->legal);
}
