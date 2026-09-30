#include <iostream>
#include <string>
#include <regex>
#include <filesystem>

#include "mgmt/FileManager.h"
#include "structs/Shared.h"

namespace FileManager {

bool IsSupportedExtension(const std::filesystem::path& path) {
	// Use regex to match against file extension array
	std::string regexPattern = R"(\.()";
	for (size_t i = 0; i < 16; ++i) {
		regexPattern += FileManager::supportedExtensions[i] + (i < 15 ? "|" : "");
	}
	regexPattern += ")$";

	// Match against regex (works)
	if (std::regex_search(path.string(), std::regex(regexPattern))) {
		return true;
	} else {
		std::cerr << "[ERROR]: Unsupported file extension or regex issue! Note that file extensions are case-sensitive (.IPD)!" << std::endl;
		return false;
	}
}

std::string Convert6BitFilenameToString(const u_Filename& filename) {
	std::string newFilename;
	newFilename.reserve(8);

	for (int shift = 4; shift < 28; shift += 6) {
		newFilename.push_back(static_cast<char>(32 + ((filename.u32[0] >> shift) & 63)));
	}

	for (int shift = 0; shift < 24; shift += 6) {
		newFilename.push_back(static_cast<char>(32 + ((filename.u32[1] >> shift) & 63)));
	}

	while (!newFilename.empty() && newFilename.back() == ' ') {
		newFilename.pop_back();
	}

	return newFilename;
}

}