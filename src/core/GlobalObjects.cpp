#include "core/GlobalObjects.h"
#include "core/DataStructs.h"
#include "core/TexturePool.h"

GlobalObjects::GlobalObjects(TexturePool& texturePool, const std::filesystem::path& sourceDir) {
    std::cout << "[INFO]: Constructed global object bank from path!" << std::endl;
}