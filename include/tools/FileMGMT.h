#include <string>

namespace FileMGMT {

const std::string supportedExtensions[4] = {
	"IPD",		// Chunk files (geometry, collisions, etc)
	"PLM",		// Global objects shared across chunks
	"TIM",		// Textures
	"BIN"		// Binary overlay OR other unknown binary
};

bool IsSupportedExtension(const std::string& path);

}
