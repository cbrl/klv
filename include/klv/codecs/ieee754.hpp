/**
 * @file klv/codecs/ieee754.hpp
 * @brief IEEE 754 floating-point codec
 */

#pragma once

#include "klv/bytes/big_endian.hpp"
#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/field.hpp"

#include <bit>
#include <concepts>
#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <type_traits>

namespace klv::codec {

/**
 * @brief Big-endian IEEE 754 binary32 or binary64 value
 */
template<std::floating_point F>
requires(sizeof(F) == 4 || sizeof(F) == 8)
struct ieee754 {
	using value_type = F;
	using bits_type = std::conditional_t<sizeof(F) == 4, uint32_t, uint64_t>;

	[[nodiscard]]
	constexpr auto decode(byte_span in) const -> field<F> {
		if (in.size() != sizeof(F)) {
			return std::unexpected(field_error::bad_length);
		}
		return std::bit_cast<F>(read_big_endian<bits_type>(in));
	}

	[[nodiscard]]
	auto encode(F value, byte_writer& out) const -> encode_result {
		out.put_big_endian(std::bit_cast<bits_type>(value));
		return {};
	}

	[[nodiscard]]
	auto to_string(F value) const -> std::string {
		return std::format("{}", value);
	}
};

}  // namespace klv::codec
