/**
 * @file klv/bytes/big_endian.hpp
 * @brief Big-endian integer decoding
 */

#pragma once

#include "klv/bytes/types.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace klv {

/// Integer types that the big-endian helpers accept
template<typename T>
concept wire_integer = std::integral<T> && !std::same_as<T, bool>;

/**
 * @brief Read a big-endian integer
 *
 * A signed type is sign-extended from the first byte, so a short input keeps its sign.
 *
 * @pre `1 <= bytes.size() <= sizeof(T)`. The codecs check this before the call.
 */
template<wire_integer T>
[[nodiscard]]
constexpr auto read_big_endian(byte_span bytes) noexcept -> T {
	using U = std::make_unsigned_t<T>;

	auto value = U{0};
	if constexpr (std::is_signed_v<T>) {
		if (!bytes.empty() && (std::to_integer<uint8_t>(bytes.front()) & 0x80U) != 0) {
			value = static_cast<U>(~U{0});
		}
	}

	for (auto const byte : bytes) {
		if constexpr (sizeof(U) == 1) {
			value = std::to_integer<U>(byte);
		}
		else {
			value = static_cast<U>((value << 8U) | std::to_integer<U>(byte));
		}
	}

	return static_cast<T>(value);
}

}  // namespace klv
