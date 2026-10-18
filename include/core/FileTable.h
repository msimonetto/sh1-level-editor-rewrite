#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct FileTableEntry {
	// As they appear in exported JSON
	uint32_t 	lba 		= 0,
			 	size_blocks	= 0;		// Provided in 256-byte blocks
    std::string name, dir, ext;

    std::string GetInternalPath() const {
    	std::string path = ext.empty() ? name : (name + "." + ext);
    	return dir.empty() ? path : (dir + "/" + path);
    }

    size_t GetSizeInBytes() const {
    	return static_cast<uint32_t>((size_blocks) * 256);
    }

    bool SetSizeInBytes(size_t bytes) {
    	size_blocks = static_cast<uint32_t>((bytes + 255) / 256);
    	return true;
    }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FileTableEntry, lba, size_blocks, name, dir, ext);

class FileTable {
	private:
		std::vector<FileTableEntry> 			entries_;
		std::unordered_map<std::string, size_t> lookupTable_;

		bool RebuildTable();
	public:
		FileTable(const std::filesystem::path& path);
		~FileTable();

		bool ImportTable(const std::filesystem::path& path);
		bool ExportTable(const std::filesystem::path& path);

		size_t GetEntryCount() const { return entries_.size(); }
		const std::vector<FileTableEntry>& GetAllEntries() const { return entries_; }
		FileTableEntry* FindEntry(const std::string& path);

		bool UpdateSize(const std::string& path, size_t newSize);
};