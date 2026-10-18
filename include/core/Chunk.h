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
	BoundedVector_uint8<IPD_SCENE_GROUP_INSTANCE>	groupInstances;
	BoundedVector_uint8<IPD_SCENE_BILLBOARD>		billboards;
	BoundedVector_uint8<IPD_SCENE_BOUNDING_BOX>		boundingBoxes;
};

class Chunk {
	private:
		// ~~~~~~~~ Structs ~~~~~~~~~
		// ~~~ Models ~~~
		IPD_HEADER 			header_;
		LMData				internalLMData_;
		
		// ~~~ Scene ~~~
		IPD_VISIBILITY_TABLE			visibilityTable_;
		BoundedVector_uint8<IPD_MODEL_INFO>		modelLookupTable_;
		BoundedVector_uint8<SceneGroupData>		sceneGroups_;
		BoundedVector_uint8<uint8_t>			renderOrder_;

		// ~~~ Collision ~~~
		CollisionData		collisionData_;

		// Globals
		BoundedVector_uint4<std::string>		dependentTextures_;
		BoundedVector_uint8<std::string>		dependentGlobalObjects_;	// Maybe demote that down from 256 -> 16
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
