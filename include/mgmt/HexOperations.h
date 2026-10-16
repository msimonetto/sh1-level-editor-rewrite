#include <format>
#include <bit>
#include <string>
#include <concepts>

namespace HexOperations {

template <typename T>
std::string AsHex(T x) {
    // Cast to unsigned and words swapped
    auto unsignedX = std::byteswap(static_cast<std::make_unsigned_t<T>>(x));
    return std::format("{:0{}X}", unsignedX, 2 * sizeof(T));
}

}