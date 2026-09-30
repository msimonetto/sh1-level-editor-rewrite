#include "core/GlobalObjects.h"
#include "core/DataStructs.h"
#include "core/TexturePool.h"

GlobalObjects::GlobalObjects(const std::filesystem::path& sourceDir, TexturePool& texturePool)
                           : texturePool_(texturePool) {
    this->sourceDir_    = sourceDir;
    std::cout << "[INFO]: Constructed global object bank from path!" << std::endl;
}

GlobalObjects::~GlobalObjects() {
    std::cout << "[INFO]: Destroyed global object bank!" << std::endl;
}