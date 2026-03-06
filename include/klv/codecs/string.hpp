/**
 * @file klv/codecs/string.hpp
 * @brief Character string codec
 */

#pragma once

#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/field.hpp"

#include <cstddef>
#include <expected>
#include <limits>
#include <span>
#include <string>

namespace klv::codec {

/**
 * @brief Character string (ISO 646 or UTF-8)
 *
 * Decoding removes trailing NUL bytes. Encoding fails when the value is longer than `max_length` bytes.
 */
struct string {
	using value_type = std::string;

	size_t max_length = std::numeric_limits<size_t>::max();

	[[nodiscard]]
	auto decode(byte_span in) const -> field<std::string> {
		auto size = in.size();
		while (size > 0 && in[size - 1] == std::byte{0}) {
			--size;
		}
		return std::string(reinterpret_cast<char const*>(in.data()), size);
	}

	[[nodiscard]]
	auto encode(std::string const& value, byte_writer& out) const -> encode_result {
		if (value.size() > max_length) {
			return std::unexpected(field_error::bad_length);
		}
		out.put_bytes(std::as_bytes(std::span{value}));
		return {};
	}

	[[nodiscard]]
	auto to_string(std::string const& value) const -> std::string {
		return value;
	}
};

}  // namespace klv::codec
