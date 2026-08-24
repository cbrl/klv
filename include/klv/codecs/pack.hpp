/**
 * @file klv/codecs/pack.hpp
 * @brief Read and write helpers for codecs of packs: values with an inner structure
 *
 * The read helpers return a `field_error` on failure, so a pack codec can pass the error on.
 */

#pragma once

#include "klv/bytes/big_endian.hpp"
#include "klv/bytes/byte_reader.hpp"
#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/field.hpp"

#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <span>
#include <string>
#include <string_view>

namespace klv::detail {

[[nodiscard]]
inline auto read_oid(byte_reader& reader) -> field<uint32_t> {
	auto const value = reader.read_ber_oid();
	if (!value) {
		return std::unexpected(field_error::invalid_value);
	}
	return *value;
}

[[nodiscard]]
inline auto read_exact(byte_reader& reader, size_t count) -> field<byte_span> {
	auto const bytes = reader.take(count);
	if (!bytes) {
		return std::unexpected(field_error::bad_length);
	}
	return *bytes;
}

/// Read a BER length and the bytes that it counts
[[nodiscard]]
inline auto read_prefixed(byte_reader& reader) -> field<byte_span> {
	auto const length = reader.read_ber_length();
	if (!length) {
		return std::unexpected(field_error::invalid_value);
	}
	return read_exact(reader, *length);
}

/// IEEE 754 value of 4 or 8 bytes. Another length fails with `field_error::bad_length`.
[[nodiscard]]
inline auto read_ieee(byte_span bytes) -> field<double> {
	if (bytes.size() == sizeof(float)) {
		return static_cast<double>(std::bit_cast<float>(read_big_endian<uint32_t>(bytes)));
	}
	if (bytes.size() == sizeof(double)) {
		return std::bit_cast<double>(read_big_endian<uint64_t>(bytes));
	}
	return std::unexpected(field_error::bad_length);
}

/// IEEE 754 binary16 value of 2 bytes
[[nodiscard]]
inline auto read_binary16(byte_span bytes) -> double {
	auto const bits = read_big_endian<uint16_t>(bytes);
	auto const exponent = (bits >> 10U) & 0x1FU;
	auto const fraction = bits & 0x3FFU;

	auto value = 0.0;
	if (exponent == 0) {
		// Zero or a subnormal value
		value = std::ldexp(static_cast<double>(fraction), -24);
	}
	else if (exponent == 0x1F) {
		value = fraction == 0 ? std::numeric_limits<double>::infinity() : std::numeric_limits<double>::quiet_NaN();
	}
	else {
		value = std::ldexp(static_cast<double>(fraction + 1024U), static_cast<int>(exponent) - 25);
	}
	return (bits & 0x8000U) != 0 ? -value : value;
}

/**
 * @brief Write an IEEE 754 value of 4 or 8 bytes
 *
 * Another length fails with `field_error::bad_length`. A finite value outside the binary32 range fails with
 * `field_error::out_of_range` for 4 bytes.
 */
[[nodiscard]]
inline auto put_ieee(double value, size_t length, byte_writer& out) -> encode_result {
	if (length == sizeof(float)) {
		if (std::isfinite(value) && std::abs(value) > double{std::numeric_limits<float>::max()}) {
			return std::unexpected(field_error::out_of_range);
		}
		out.put_big_endian(std::bit_cast<uint32_t>(static_cast<float>(value)));
		return {};
	}
	if (length == sizeof(double)) {
		out.put_big_endian(std::bit_cast<uint64_t>(value));
		return {};
	}
	return std::unexpected(field_error::bad_length);
}

[[nodiscard]]
inline auto to_text(byte_span bytes) -> std::string {
	return std::string(reinterpret_cast<char const*>(bytes.data()), bytes.size());
}

inline auto put_text(std::string_view text, byte_writer& out) -> void {
	out.put_bytes(std::as_bytes(std::span{text}));
}

/// Write a BER length and the text
inline auto put_prefixed_text(std::string_view text, byte_writer& out) -> void {
	out.put_ber_length(text.size());
	put_text(text, out);
}

/// Fail when bytes remain after the last element of a pack
template<typename T>
[[nodiscard]]
auto finish(byte_reader const& reader, T value) -> field<T> {
	if (!reader.empty()) {
		return std::unexpected(field_error::invalid_value);
	}
	return value;
}

}  // namespace klv::detail
