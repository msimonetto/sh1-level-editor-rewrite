#pragma once

#include <string>
#include <filesystem>

#include "structs/Shared.h"

namespace FileManager {

// Adapted from included `extract.py` from `Vatuu/silent-hill-decomp`

const std::string directories[11] {
	"1ST",
	"ANIM",
	"BG",		// Contains almost everything we need exclusively
	"CHARA",
	"ITEM",
	"MISC",
	"SND",
	"TEST", 
    "TIM",
	"VIN",
	"XA"
};

const std::string supportedExtensions[16] = {
	"TIM",		// Textures
	"VAB",		// Audio bank?
	"BIN",		// Binary overlay OR other unknown binary
	"DMS",		// ?
	"ANM",		// ?
	"PLM",		// Global objects shared across chunks
	"IPD",		// Chunk files (geometry, collisions, etc)
	"ILM",		// Untested, should contain character geometry
	"TMD",		// ?
	"DAT",		// ?
	"KDT",		// ?
	"CMP",		// ?
	"TXT",		// ?
	"UU1",		// ?
	"UU2",		// ?
	""			// Unsure where this appears
};

// ~~~~~~~~~~~ Filename operations ~~~~~~~~~~~
bool IsSupportedExtension(const std::filesystem::path& path);
std::string Convert6BitFilenameToString(const u_Filename& filename);

}