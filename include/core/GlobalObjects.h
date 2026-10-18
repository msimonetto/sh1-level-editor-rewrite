#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <cstdint>

#include "core/DataStructs.h"
#include "core/DataOperations.h"
#include "core/TexturePool.h"

// Prefix-related cluster of geometry objects (stored in memory)
class GlobalObjects {
	private:
		// ~~~~~~~~ Structs ~~~~~~~~~
		LM_HEADER			header_;				// Contains count_LM_MODEL_HEADER and count_LM_MATERIAL
		LMData				globalLMData_;

		TexturePool&		texturePool_;

		// ~~~~~~~~ I/O ~~~~~~~~~
		BinaryFile			inFile_;
		BinaryFile			outFile_;

		// ~~~~~~~~ Properties ~~~~~~~~~
		uint8_t				chunkUsage_;
		bool				active_,
							inMemory_,
							legal_;

	public:
		// Active, if XYZ_GLB.PLM exists
		GlobalObjects(const std::filesystem::path& inFile, const std::filesystem::path& outFile,
									 TexturePool& texturePool);

		// Inactive, if XYZ_GLM.PLM does not exist
		GlobalObjects(TexturePool& texturePool);
		~GlobalObjects();

		bool Parse();
		bool Write();
		bool ReconcileHeaders();
};