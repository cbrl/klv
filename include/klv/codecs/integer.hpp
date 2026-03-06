/**
 * @file klv/codecs/integer.hpp
 * @brief Big-endian integer codec
 */

#pragma once

#include "klv/bytes/big_endian.hpp"
#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/field.hpp"

#include <expected>
#include <format>
#include <string>

namespace klv::codec {

/**
 * @brief Big-endian integer
 *
 * Decoding accepts 1 to `sizeof(T)` bytes, so a shortened encoding also decodes. A signed value is sign-extended.
 * Encoding always writes `sizeof(T)` bytes.
 */
template<wire_integer T>
struct integer {
	using value_type = T;

	[[nodiscard]]
	constexpr auto decode(byte_span in) const -> field<T> {
		if (in.empty() || in.size() > sizeof(T)) {
			return std::unexpected(field_error::bad_length);
		}
		return read_big_endian<T>(in);
	}

	[[nodiscard]]
	auto encode(T value, byte_writer& out) const -> encode_result {
		out.put_big_endian(value);
		return {};
	}

	[[nodiscard]]
	auto to_string(T value) const -> std::string {
		return std::format("{}", value);
	}
};

}  // namespace klv::codec
