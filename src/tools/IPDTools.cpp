#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <format>
#include <bit>
#include <filesystem>

#include "formats/IPD.h"
#include "formats/LM.h"
#include "tools/IPDTools.h"
#include "tools/FileMGMT.h"

namespace IPDTools
{

IPDChunk::IPDChunk(const std::string& path)
{
	this->loaded = false;

	if (std::filesystem::exists(path)
			&& FileMGMT::IsSupportedExtension(path)) {
		this->path_  = path;
		this->legal = true;
	} else {
		std::cerr << "[INFO]: Constructed chunk object but without path!" << std::endl;
		this->legal = false;
	}

	std::cout << "[INFO]: Constructed chunk object from path!" << std::endl;
}

IPDChunk::~IPDChunk()
{
	std::cout << "[INFO]: Destructed chunk object!" << std::endl;
}

// Extracting binary sections between sections
// Repeated in `UnpackIpdToMem()`
// Assume: Only reading from current chunk file (file_)
template <typename T>
int IPDChunk::UnpackSectionToStruct(unsigned int baseOffset,
									unsigned int length,
									T& outStruct)
{
	this->offset_ = baseOffset;
	file_.seekg(offset_, std::ios::beg);
	file_.read(reinterpret_cast<char*>(&outStruct), length);

	if (!(file_)) {
		std::cerr << "[ERROR]: Failed to unpack from offset " << baseOffset << std::endl;
		return -1;
	}

	return 1;
}

// Should map out the entire file (as it appears in a Hex editor)
// using known structs in `formats/` header folder
int IPDChunk::UnpackIPDToMem()
{
	// Open IPD from pre-specified `path_` variable (rw)
	file_.open(path_, std::ios::in | std::ios::binary);
	if (!file_) {
		std::cerr << "[ERROR]: Unable to open IPD file!" << std::endl;
		return -1;
	}

	// Trivially read from offset 0 and offload into IPD_HEADER struct
	// Ensure that first word == 0x14
	IPDChunk::UnpackSectionToStruct(0, sizeof(IPD_HEADER), header_);
	if (!(header_.isValid())) {
		std::cerr << "[ERROR]: Not a valid IPD header, offset = " << (int)(header_.id) << std::endl;
		return -1;
	}

	// Incomplete, this requires an advanced recursive routine
	// Should this make use of a file like `LMTools.cpp`?
	// For now, we can stick to `IPDTools.cpp` and then carry over later

	// Embedded chunk models (geometry definitively stored inside IPD)
	// Extract LM_HEADER
	IPDChunk::UnpackSectionToStruct(this->header_.ptr_LM_HEADER,
			sizeof(LM_HEADER), embeddedLmHeader_);
	
	// Iterate through LM_MODEL_HEADERS
	embeddedModelHeaders_.resize(embeddedLmHeader_.modelCount);
	for (size_t i = 0; i < embeddedLmHeader_.modelCount; ++i) {
		IPDChunk::UnpackSectionToStruct(
				embeddedLmHeader_.ptr_modelHdrs + i * sizeof(LM_MODEL_HEADER),
				sizeof(LM_MODEL_HEADER),
				embeddedModelHeaders_.at(i));
	}



	return 0;
}

int IPDChunk::GetSizeInPath()
{
	std::error_code errorCode;
	auto size = std::filesystem::file_size(path_, errorCode);

	if (errorCode) {
		std::cerr << "[ERROR]: " << errorCode.message() << std::endl;
		return 0;
	}

	return static_cast<int>(size);
}

int IPDChunk::GetSizeInMem()
{
	// Comes later
	return 0;
}

int IPDChunk::IsValid()
{
	return header_.isValid();
}

}
