#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <variant>
#include <unordered_map>

#include "core/Collision.h"
#include "core/DataStructs.h"
#include "core/GlobalObjects.h"
#include "core/DataOperations.h"
#include "core/TexturePool.h"
#include "structs/IPD.h"
#include "structs/LM.h"

// 2048x2048 scene grid of models/lights (per chunk)
struct SceneGroupData {
	IPD_SCENE_GROUP							header;
	std::vector<IPD_SCENE_GROUP_INSTANCE>	instances;
	std::vector<IPD_SCENE_BILLBOARD>		billboards;
	std::vector<IPD_SCENE_BOUNDING_BOX>		boundingBoxes;
};

class Chunk {
	private:
		// ~~~~~~~~ Structs ~~~~~~~~~
		// ~~~ Models ~~~
		IPD_HEADER 			header_;
		LMData				internalLMData_;
		
		// ~~~ Scene ~~~
		std::vector<IPD_MODEL_INFO>		modelLookupTable_;
		std::vector<SceneGroupData>		sceneGroups_;
		std::vector<uint8_t>			renderOrder_;

		// ~~~ Collision ~~~
		CollisionData		collisionData_;

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

		bool UnpackChunkToMem();
		bool PackMemToIPD();

		int GetSizeInPath();
		int GetSizeInMem();
		bool IsValid();
};
