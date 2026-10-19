#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <nlohmann/json.hpp>

#include "mgmt/FileTable.h"
#include "mgmt/FileOperations.h"

FileTable::FileTable(const std::filesystem::path& path) {
	if (!ImportTable(path)) {
		std::cerr << "[ERROR]: File table could not be accessed. Please run 'exportFileTable.py' and direct to the Game Extracts root folder." << std::endl;
	}
}

FileTable::~FileTable() {}

bool FileTable::ImportTable(const std::filesystem::path& path) {
	if (!std::filesystem::exists(path) || path.extension() != ".json") {
		std::cerr << "[ERROR]: File table does not exist at " << path.string() << std::endl;
		return false;
	} // potential case sensitivity!

	std::ifstream inFile(path);
	if (!inFile.is_open()) {
		std::cerr << "[ERROR]: File table exists at " << path.string() << " but it is inaccessible." << std::endl;
	}

	try {
		nlohmann::json jTable;
		inFile >> jTable;

		entries_ = jTable.get<std::vector<FileTableEntry>>();
		RebuildTable();

		std::cout << "[SUCCESS]: Imported " << entries_.size() << " entries from file table." << std::endl;
		return true;
	} catch (const nlohmann::json::exception& e) {
		std::cerr << "[ERROR]: Unable to parse JSON, caught exception " << e.what() << std::endl;
		return false;
	}
}

bool FileTable::ExportTable(const std::filesystem::path& path) {
	std::ofstream outFile(path);
	if (!outFile.is_open()) {
		std::cerr << "[ERROR]: Attempted file table access at " << path.string() << " but it is inaccessible." << std::endl;
	}

	try {
		nlohmann::json jTable = entries_;
		outFile << jTable.dump(2);

		std::cout << "[SUCCESS]: Exported " << entries_.size() << " entries from file table." << std::endl;
		return true;
	} catch (const nlohmann::json::exception& e) {
		std::cerr << "[ERROR]: Unable to dump JSON, caught exception " << e.what() << std::endl;
		return false;
	}
}

bool FileTable::RebuildTable() {
	lookupTable_.clear();

	for (size_t i = 0; i < entries_.size(); ++i) {
		lookupTable_.insert({NormalisePath(entries_[i].GetInternalPath()), i});
	}

	return true;
}

FileTableEntry* FileTable::FindEntry(const std::string& path) {
	auto i = lookupTable_.find(NormalisePath(path));
	return (i != lookupTable_.end()) ? &entries_[i->second] : nullptr;
}

bool FileTable::UpdateSize(const std::string& path, size_t newSize) {
	auto* entry = FindEntry(NormalisePath(path));
	if (!entry) { return false; }

	entry->SetSizeInBytes(newSize);
	return true;
}
