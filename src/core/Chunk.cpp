#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include "core/Chunk.h"
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
	
	// Unpack Internal LM data
	UnpackLMData(internalLMData_, internalLMBaseOffset, texturePool_, file_);

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
