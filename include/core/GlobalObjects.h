#pragma once

#include <string>
#include <fstream>
#include <filesystem>

#include "core/DataStructs.h"
#include "core/MemoryOperations.h"
#include "core/TexturePool.h"

// Prefix-related cluster of geometry objects (stored in memory)
class GlobalObjects {
	private:
		// ~~~~~~~~ Structs ~~~~~~~~~
		LM_HEADER			header_;				// Contains count_LM_MODEL_HEADER and count_LM_MATERIAL
		LMData				globalLMData_;

		TexturePool&		texturePool_;

		// ~~~~~~~~ I/O ~~~~~~~~~
		std::filesystem::path	sourceDir_;
		std::string			filename_;
		BinaryFile			file_;

		// ~~~~~~~~ Properties ~~~~~~~~~
		std::string			prefix_;
		uint8_t				chunkUsage;				// Increments per chunk object
													// Safeguard this when destroying
		bool				loaded,
							legal;

	public:
		GlobalObjects(const std::filesystem::path& sourceDir, TexturePool& texturePool);
		~GlobalObjects();
};