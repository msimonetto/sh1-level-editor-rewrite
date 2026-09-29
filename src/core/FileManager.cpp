#include <iostream>
#include <string>
#include <regex>

#include "core/FileManager.h"

namespace FileManager {

bool IsSupportedExtension(const std::string& path) {
	// Use regex to match against file extension array
	std::string regexPattern = R"(\.()";
	for (size_t i = 0; i < 4; ++i) {
		regexPattern += FileManager::supportedExtensions[i] + (i < 3 ? "|" : "");
	}
	regexPattern += ")$";

	// Case insensitivity
	std::regex regexPatternCI(regexPattern, std::regex_constants::icase);

	// Match against regex (works)
	if (std::regex_search(path, regexPatternCI)) {
		return true;
	} else {
		std::cerr << "[ERROR]: Unsupported file extension or regex issue!" << std::endl;
		return false;
	}
}

}