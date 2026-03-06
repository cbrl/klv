/**
 * @file klv/bytes/hex.hpp
 * @brief Conversion between bytes and hex text
 */

#pragma once

#include "klv/bytes/types.hpp"
#include "klv/error.hpp"

#include <cstddef>
#include <format>
#include <string>
#include <string_view>

namespace klv {

/**
 * @brief Convert bytes to an upper-case hex string
 * @param bytes Bytes to convert
 * @param separator Text between two bytes
 */
[[nodiscard]]
inline auto to_hex(byte_span bytes, std::string_view separator = " ") -> std::string {
	constexpr auto digits = std::string_view{"0123456789ABCDEF"};

	auto text = std::string{};
	text.reserve(bytes.size() * (2 + separator.size()));

	for (auto i = size_t{0}; i < bytes.size(); ++i) {
		if (i != 0) {
			text += separator;
		}
		auto const value = std::to_integer<size_t>(bytes[i]);
		text += digits[value >> 4U];
		text += digits[value & 0x0FU];
	}

	return text;
}

namespace detail {

[[nodiscard]]
constexpr auto hex_digit(char c) noexcept -> int {
	if (c >= '0' && c <= '9') {
		return c - '0';
	}
	if (c >= 'a' && c <= 'f') {
		return c - 'a' + 10;
	}
	if (c >= 'A' && c <= 'F') {
		return c - 'A' + 10;
	}
	return -1;
}

[[nodiscard]]
constexpr auto is_hex_space(char c) noexcept -> bool {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

}  // namespace detail

/**
 * @brief Convert a hex string to bytes
 * @param hex Hex text, with or without white space between bytes (for example "06 0E 2B" or "060E2B")
 */
[[nodiscard]]
inline auto from_hex(std::string_view hex) -> result<byte_vector> {
	auto bytes = byte_vector{};
	bytes.reserve(hex.size() / 2);

	auto i = size_t{0};
	while (i < hex.size()) {
		if (detail::is_hex_space(hex[i])) {
			++i;
			continue;
		}

		if (i + 1 >= hex.size()) {
			return detail::make_error(error_code::invalid_value, i, "Incomplete hex byte");
		}

		auto const high = detail::hex_digit(hex[i]);
		auto const low = detail::hex_digit(hex[i + 1]);
		if (high < 0 || low < 0) {
			return detail::make_error(error_code::invalid_value, i, std::format("Invalid hex digit at position {}", i));
		}

		bytes.push_back(static_cast<std::byte>((high << 4) | low));
		i += 2;
	}

	return bytes;
}

}  // namespace klv
