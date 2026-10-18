#include <cassert>
#include <filesystem>
#include <fstream>

#include "core/GlobalObjects.h"
#include "core/DataOperations.h"
#include "core/DataStructs.h"
#include "core/TexturePool.h"
#include "mgmt/FileOperations.h"


GlobalObjects::GlobalObjects(
	const std::filesystem::path& inFile,
	const std::filesystem::path& outFile,
	TexturePool& texturePool)
	:
	texturePool_(texturePool),
	legal_(true),
	active_(true),
	inMemory_(false),
	chunkUsage_(0)
{
	inFile_.path = NormalisePath(inFile.string());
	outFile_.path = NormalisePath(outFile.string());

	assert(inFile.stem() == outFile.stem());
}

GlobalObjects::GlobalObjects(TexturePool& texturePool)
	:
	texturePool_(texturePool),
	legal_(true),
	active_(false),
	inMemory_(false),
	chunkUsage_(0) {}

GlobalObjects::~GlobalObjects() {}

bool GlobalObjects::Parse() {
	if (this->legal_ && this->active_) {
		inFile_.stream.open(inFile_.path, std::ios::in | std::ios::binary);
		if (!inFile_.stream) {
			std::cerr << "[ERROR]: Failed to access contents of " << inFile_.path.stem().string() + ".PLM" << std::endl;
			return false;
		}

		// Same treatment as internal LM data from Chunk.cpp, parses just fine
		if (!ParseLMData(0, globalLMData_, inFile_)) { return false; }

		inFile_.stream.close();

		this->inMemory_ = true;
		std::cout << "[SUCCESS]: Parsed " << inFile_.path.stem().string() + ".PLM" << std::endl;

		return true;
	} else if (!this->active_) {
		std::cerr << "[WARNING]: Selected prefix in workspace does not contain a global object bank" << std::endl;
		return false;
	} else {
		std::cerr << "[WARNING]: Fix existing integrity issues before parsing " << inFile_.path.stem().string() + ".PLM" << std::endl;
		return false;
	}
}

bool GlobalObjects::Write() {
	if (this->legal_ && this->active_) {
		outFile_.stream.open(outFile_.path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!outFile_.stream) {
			std::cerr << "[ERROR]: Failed to access contents of " << outFile_.path.stem().string() + ".PLM" << std::endl;
			return false;
		}

		// Same treatment as internal LM data from Chunk.cpp, parses just fine
		std::cout << "Writing to " << outFile_.path.string() << std::endl;
		if (!WriteLMData(0, globalLMData_, outFile_)) { return false; }
		std::cout << "Done writing to " << outFile_.path.string() << std::endl;

		AlignFileToSector(outFile_);

		outFile_.stream.close();

		std::cout << "[SUCCESS]: Wrote " << outFile_.path.stem().string() + ".PLM" << std::endl;
		return true;
	} else if (!this->active_) {
		std::cerr << "[INFO]: Selected prefix in workspace does not contain a global object bank" << std::endl;
		return false;
	} else {
		std::cerr << "[WARNING]: Fix existing integrity issues before parsing " << outFile_.path.stem().string() + ".PLM" << std::endl;
		return false;
	}
}

// bool GlobalObjects::ReconcileHeaders() {}