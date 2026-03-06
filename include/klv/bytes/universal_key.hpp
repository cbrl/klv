/**
 * @file klv/bytes/universal_key.hpp
 * @brief SMPTE 336 universal keys: the key type, compile-time key literals, and key comparison
 */

#pragma once

#include "klv/bytes/hex.hpp"
#include "klv/bytes/types.hpp"
#include "klv/error.hpp"

#include <array>
#include <cstddef>
#include <string_view>

namespace klv {

/// SMPTE 336 universal key
using universal_key = std::array<std::byte, 16>;

/**
 * @brief Parse a universal key literal at compile time
 *
 * A malformed literal does not compile.
 *
 * @code
 * static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 01 01 00 00 00");
 * @endcode
 */
[[nodiscard]]
consteval auto ul(std::string_view hex) -> universal_key {
	auto key = universal_key{};
	auto count = size_t{0};

	auto i = size_t{0};
	while (i < hex.size()) {
		if (detail::is_hex_space(hex[i])) {
			++i;
			continue;
		}

		detail::require(i + 1 < hex.size(), "Incomplete hex byte in universal key");
		detail::require(count < key.size(), "Universal key has more than 16 bytes");

		auto const high = detail::hex_digit(hex[i]);
		auto const low = detail::hex_digit(hex[i + 1]);
		detail::require(high >= 0 && low >= 0, "Invalid hex digit in universal key");

		key[count++] = static_cast<std::byte>((high << 4) | low);
		i += 2;
	}

	detail::require(count == key.size(), "Universal key has fewer than 16 bytes");
	return key;
}

/**
 * @brief Compare a universal key with key bytes from a packet
 *
 * SMPTE 336 says that a decoder ignores the version byte (byte 8, index 7) when it compares two universal labels.
 */
[[nodiscard]]
constexpr auto key_matches(universal_key const& expected, byte_span actual) noexcept -> bool {
	constexpr auto version_index = size_t{7};

	if (actual.size() != expected.size()) {
		return false;
	}

	for (auto i = size_t{0}; i < expected.size(); ++i) {
		if (i != version_index && expected[i] != actual[i]) {
			return false;
		}
	}
	return true;
}

/// The first four bytes of every SMPTE universal label. The packet splitter uses them to resynchronize.
inline constexpr auto universal_label_prefix = std::array{
	std::byte{0x06},
	std::byte{0x0E},
	std::byte{0x2B},
	std::byte{0x34}
};

}  // namespace klv
