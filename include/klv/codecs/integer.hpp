/**
 * @file klv/codecs/integer.hpp
 * @brief Integer codecs: big-endian and BER-OID
 */

#pragma once

#include "klv/bytes/big_endian.hpp"
#include "klv/bytes/byte_reader.hpp"
#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/field.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <type_traits>

namespace klv::codec {

/// Encode length of `codec::integer` that selects the shortest length that holds the value
inline constexpr auto variable_length = size_t{0};

/**
 * @brief Big-endian integer
 *
 * Decoding accepts 1 to `sizeof(T)` bytes, so a shortened encoding also decodes. A signed value is sign-extended.
 * Encoding writes `length` bytes (at most `sizeof(T)`), or the shortest length for `variable_length`. A value that
 * does not fit in `length` bytes fails with `field_error::out_of_range`.
 *
 * @code
 * static constexpr codec::integer<uint16_t> format{};                                    // 2 bytes
 * static constexpr codec::integer<uint32_t> format{.length = 3};                         // 3 bytes
 * static constexpr codec::integer<uint64_t> format{.length = codec::variable_length};    // 1 to 8 bytes
 * @endcode
 */
template<wire_integer T>
struct integer {
	using value_type = T;

	size_t length = sizeof(T);

	[[nodiscard]]
	constexpr auto decode(byte_span in) const -> field<T> {
		if (in.empty() || in.size() > sizeof(T)) {
			return std::unexpected(field_error::bad_length);
		}
		return read_big_endian<T>(in);
	}

	[[nodiscard]]
	auto encode(T value, byte_writer& out) const -> encode_result {
		auto const count = length == variable_length ? shortest_length(value) : std::min(length, sizeof(T));
		if (count < shortest_length(value)) {
			return std::unexpected(field_error::out_of_range);
		}
		out.put_big_endian(value, count);
		return {};
	}

	[[nodiscard]]
	auto to_string(T value) const -> std::string {
		return std::format("{}", value);
	}

private:
	/// Fewest bytes that hold `value`. A signed value keeps its sign bit.
	[[nodiscard]]
	static constexpr auto shortest_length(T value) noexcept -> size_t {
		auto count = size_t{1};
		while (count < sizeof(T)) {
			auto const bits = static_cast<int>(count * 8);
			if constexpr (std::is_signed_v<T>) {
				auto const limit = static_cast<T>(T{1} << (bits - 1));
				if (value >= -limit && value < limit) {
					break;
				}
			}
			else if ((value >> bits) == 0) {
				break;
			}
			++count;
		}
		return count;
	}
};

/**
 * @brief Unsigned integer as a BER-OID: 7 bits in each byte, most significant group first
 *
 * Decoding fails with `field_error::invalid_value` for a malformed value, a value wider than 32 bits, or bytes after
 * the value.
 */
struct ber_oid {
	using value_type = uint32_t;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<uint32_t> {
		auto reader = byte_reader{in};
		auto const value = reader.read_ber_oid();
		if (!value || !reader.empty()) {
			return std::unexpected(field_error::invalid_value);
		}
		return *value;
	}

	[[nodiscard]]
	auto encode(uint32_t value, byte_writer& out) const -> encode_result {
		out.put_ber_oid(value);
		return {};
	}

	[[nodiscard]]
	auto to_string(uint32_t value) const -> std::string {
		return std::format("{}", value);
	}
};

}  // namespace klv::codec
