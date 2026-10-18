#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <variant>
#include <unordered_set>

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
	std::vector<IPD_SCENE_GROUP_INSTANCE>	groupInstances;
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
		IPD_VISIBILITY_TABLE			visibilityTable_;
		std::vector<IPD_MODEL_INFO>		modelLookupTable_;
		std::vector<SceneGroupData>		sceneGroups_;
		std::vector<uint8_t>			renderOrder_;

		// ~~~ Collision ~~~
		CollisionData		collisionData_;

		// Globals
		std::unordered_set<std::string>		dependentTextures_;			// Consider limiting size based on engine constraints
		std::unordered_set<std::string>		dependentGlobalObjects_;
		TexturePool&		texturePool_;
		GlobalObjects&		globalObjects_;

		// ~~~~~~~~ I/O ~~~~~~~~~
		BinaryFile			inFile_;
		BinaryFile			outFile_;

		// ~~~~~~~~ Properties ~~~~~~~~~
		bool 				inMemory_,
							inViewport_,
							legal_;

	public:
		int8_t 				chunkMajorX_,
							chunkMajorZ_;

		Chunk(const std::filesystem::path& inFile, const std::filesystem::path& outFile,
			  TexturePool& texturePool, GlobalObjects& globalObjects);
		~Chunk();

		bool Parse();
		bool Write();
		bool GetDependencies();
		bool ReconcileHeaders();
};
