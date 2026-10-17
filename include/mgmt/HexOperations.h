#pragma once

#include <format>
#include <bit>
#include <string>
#include <concepts>

template <typename T>
std::string IntToHexLE(T x) {
    // Cast to unsigned and words swapped
    auto unsignedX = std::byteswap(static_cast<std::make_unsigned_t<T>>(x));
    return std::format("{:0{}X}", unsignedX, 2 * sizeof(T));
}

template <typename T>
std::string IntToHexBE(T x) {
    // Cast to unsigned and words swapped
    auto unsignedX = static_cast<std::make_unsigned_t<T>>(x);
    return std::format("{:0{}X}", unsignedX, 2 * sizeof(T));
}

// (-1, -1) returns FFFF as expected, this is congruent with filename
inline uint16_t ChunkHash(int8_t x, int8_t z) {
    return static_cast<uint16_t>((static_cast<uint8_t>(x) << 8) | static_cast<uint8_t>(z));
}