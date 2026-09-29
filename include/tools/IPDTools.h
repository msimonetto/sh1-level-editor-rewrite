#include <string>
#include <fstream>
#include <vector>

#include "formats/IPD.h"
#include "formats/LM.h"

namespace IPDTools {

class IPDChunk {
	private:
		// ~~~~~~~~ Structs ~~~~~~~~~
		// Header
		IPD_HEADER 			header_;

		// Embedded chunk models
		// LM_HEADER -> LM_MODEL_HEADER (# = modelCount)
		LM_HEADER  			embeddedLmHeader_;
		std::vector<LM_MODEL_HEADER>	embeddedModelHeaders_;
		std::vector<LM_MESH_HEADER>		embeddedMeshHeaders_;

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
		int UnpackSectionToStruct(unsigned int baseOffset,
								  unsigned int length,
								  T& outStruct);

		int UnpackIPDToMem();
		int GetSizeInPath();
		int GetSizeInMem();
		int IsValid();
};

}
