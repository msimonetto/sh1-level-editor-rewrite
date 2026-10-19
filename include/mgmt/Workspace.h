#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>
#include <memory>
#include <cstdint>

#include "core/Chunk.h"
#include "core/GlobalObjects.h"
#include "core/TexturePool.h"
#include "mgmt/FileTable.h"
#include "mgmt/HexOperations.h"

// Takes in extracts and output directory, manages chunk loading/offloading, required global object banks and texture pools
// Prefix-specific
class Workspace {
    private:
        // std::string sourceDir = "/mnt/data/Projects/sh1-level-editor-rewrite/data/Game Extracts/BG/THR0000.IPD"; // Linux
        std::filesystem::path   sourceDir;
        std::filesystem::path   targetDir;
        std::string             prefix;

        // Create shared pools
        std::unique_ptr<TexturePool>        texturePool;
        std::unique_ptr<GlobalObjects>      globalObjects;
        std::unique_ptr<FileTable>          fileTable;
        std::unordered_map<uint16_t, std::unique_ptr<Chunk>> chunkPool;

    public:
        Workspace(const std::filesystem::path& dir, const std::string& prefix);
        ~Workspace();

        bool AddChunk(int8_t x, int8_t z);
        bool RemoveChunk(int8_t x, int8_t z);
        bool ExportChunk(int8_t x, int8_t z);
        bool ReconcileChunk(int8_t x, int8_t z);

        bool AddGlobals();
        bool ExportGlobals();
};
