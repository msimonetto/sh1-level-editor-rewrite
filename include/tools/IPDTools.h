#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>

#include "formats/IPD.h"
#include "formats/LM.h"

namespace IPDTools {

struct MeshData {
	LM_MESH_HEADER				header;
	std::vector<LM_PRIMITIVE>	primitives;
	std::vector<DVECTOR> 		verticesXY;
	std::vector<int16_t>		verticesZ;
	std::vector<LM_NORMAL>		normals;
	std::vector<uint8_t>		shadings;		
};

struct ModelData {
	LM_MODEL_HEADER				header;
	std::vector<MeshData>		meshes;
};

struct LocalLMData {
	LM_HEADER					header;
	std::vector<LM_MATERIAL>	materials;
	std::unordered_map<uint32_t, LM_TEXTURE>	texturePool;		// Mapped by file offset (material.ptr_texture)
	std::vector<ModelData>		models;
	// std::vector<ModelOrder>		modelOrder;
};

struct ModelLookupData {
	int8_t						placeholder;
};

struct ModelBufferData {
	int8_t						placeholder;
};

class IPDChunk {
	private:
		// ~~~~~~~~ Structs ~~~~~~~~~
		IPD_HEADER 			header_;
		LocalLMData			localLMData_;
		ModelLookupData		modelLookupData_;
		ModelBufferData		modelBufferData_;

		// ~~~~~~~~ I/O ~~~~~~~~~
		std::string 		path_;
		std::fstream 		file_;
		std::streampos		offset_;

		// ~~~~~~~~ Properties ~~~~~~~~~
		std::string 		prefix_;
		int8_t 				chunkMajorX,
							chunkMajorZ;
		bool 				loaded,
							legal;

	public:
		IPDChunk(const std::string& path);
		~IPDChunk();

		template <typename T>
		void UnpackToStruct(unsigned int baseOffset, unsigned int length, T& outStruct);

		template <typename T, typename ptr_T>
		void UnpackToVector(ptr_T basePointer, size_t count, std::vector<T>& structVector);

		int UnpackIPDToMem();
		int GetSizeInPath();
		int GetSizeInMem();
		int IsValid();
};

}
