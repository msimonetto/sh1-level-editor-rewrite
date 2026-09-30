#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <variant>
#include <unordered_map>

#include "core/DataStructs.h"
#include "core/GlobalObjects.h"
#include "core/MemoryOperations.h"
#include "core/TexturePool.h"
#include "structs/IPD.h"
#include "structs/LM.h"

// Chunk (stored in memory)
class Chunk {
	private:
		// ~~~~~~~~ Structs ~~~~~~~~~
		IPD_HEADER 			header_;
		LMData				internalLMData_;						// models, materials, ...
		std::vector<IPD_MODEL_INFO>		modelLookupTable_;
		std::vector<IPD_MODEL_BUFFER>	modelBufferTable_;

		TexturePool&		texturePool_;
		GlobalObjects&		globalObjects_;

		// ~~~~~~~~ I/O ~~~~~~~~~
		std::filesystem::path	sourceDir_;
		std::string 		filename_;
		BinaryFile			file_;

		// ~~~~~~~~ Properties ~~~~~~~~~
		std::string 		prefix_;
		int8_t 				chunkMajorX_,
							chunkMajorZ_;
		bool 				loaded,
							legal;

	public:
		Chunk(const std::filesystem::path& sourceDir, const std::string& filename,
			  TexturePool& texturePool, GlobalObjects& globalObjects);
		~Chunk();

		int UnpackIPDToMem();
		int PackMemToIPD();

		int GetSizeInPath();
		int GetSizeInMem();
		bool IsValid();
};
