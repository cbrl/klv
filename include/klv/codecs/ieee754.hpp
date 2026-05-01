/**
 * @file klv/codecs/ieee754.hpp
 * @brief IEEE 754 floating-point codecs
 */

#pragma once

#include "klv/bytes/big_endian.hpp"
#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/codecs/integer.hpp"
#include "klv/codecs/pack.hpp"
#include "klv/field.hpp"

#include <bit>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <limits>
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

/**
 * @brief IEEE 754 value of either precision: binary32 (4 bytes) or binary64 (8 bytes)
 *
 * Decoding accepts both lengths. Encoding writes `length` bytes. With `variable_length`, it writes 4 bytes when a
 * binary32 value holds the value exactly, and 8 bytes otherwise.
 *
 * @code
 * static constexpr codec::ieee754_variable format{};                // 4 or 8 bytes
 * static constexpr codec::ieee754_variable format{.length = 8};     // 8 bytes
 * @endcode
 */
struct ieee754_variable {
	using value_type = double;

	size_t length = variable_length;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<double> {
		return detail::read_ieee(in);
	}

	[[nodiscard]]
	auto encode(double value, byte_writer& out) const -> encode_result {
		if (length != variable_length) {
			return detail::put_ieee(value, length, out);
		}
		// A finite value is exact in binary32 when the round trip keeps its bits
		auto const exact = !std::isfinite(value)
		    || (std::abs(value) <= double{std::numeric_limits<float>::max()}
		        && std::bit_cast<uint64_t>(static_cast<double>(static_cast<float>(value)))
		               == std::bit_cast<uint64_t>(value));
		return detail::put_ieee(value, exact ? sizeof(float) : sizeof(double), out);
	}

	[[nodiscard]]
	auto to_string(double value) const -> std::string {
		return std::format("{}", value);
	}
};

}  // namespace klv::codec
