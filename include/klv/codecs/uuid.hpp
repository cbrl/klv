/**
 * @file klv/codecs/uuid.hpp
 * @brief Universally unique identifier codec
 */

#pragma once

#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/field.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <expected>
#include <string>
#include <string_view>

namespace klv {

/// 16-byte universally unique identifier (RFC 4122)
using uuid = std::array<std::byte, 16>;

}  // namespace klv

namespace klv::codec {

/**
 * @brief 16-byte universally unique identifier
 *
 * The display text has the RFC 4122 form, for example "f81d4fae-7dec-11d0-a765-00a0c91e6bf6".
 */
struct uuid {
	using value_type = klv::uuid;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<klv::uuid> {
		if (in.size() != std::tuple_size_v<klv::uuid>) {
			return std::unexpected(field_error::bad_length);
		}
		auto value = klv::uuid{};
		std::ranges::copy(in, value.begin());
		return value;
	}

	[[nodiscard]]
	auto encode(klv::uuid const& value, byte_writer& out) const -> encode_result {
		out.put_bytes(value);
		return {};
	}

	[[nodiscard]]
	auto to_string(klv::uuid const& value) const -> std::string {
		constexpr auto digits = std::string_view{"0123456789abcdef"};

		auto text = std::string{};
		for (auto i = size_t{0}; i < value.size(); ++i) {
			if (i == 4 || i == 6 || i == 8 || i == 10) {
				text += '-';
			}
			auto const byte = std::to_integer<size_t>(value[i]);
			text += digits[byte >> 4U];
			text += digits[byte & 0x0FU];
		}
		return text;
	}
};

}  // namespace klv::codec
