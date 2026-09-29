#include <iostream>
#include <string>
#include <regex>

#include "tools/FileMGMT.h"

namespace FileMGMT {

// General purpose
bool IsSupportedExtension(const std::string& path) {
	// Use regex to match against file extension array
	std::string regexPattern = R"(\.()";
	for (size_t i = 0; i < 4; ++i) {
		regexPattern += FileMGMT::supportedExtensions[i] + (i < 3 ? "|" : "");
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
