/**
 * @file klv/codecs/timestamp.hpp
 * @brief MISB ST 0603 time stamp codecs: the Precision Time Stamp and the Nano Precision Time Stamp
 */

#pragma once

#include "klv/bytes/big_endian.hpp"
#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/field.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <limits>
#include <string>

namespace klv::codec {

/**
 * @brief MISB ST 0603 time stamp: 64-bit unsigned count of `Duration` ticks since the POSIX epoch
 *
 * Decoding accepts `min_length` to 8 bytes. A count above the `int64_t` range fails with `field_error::out_of_range`.
 * Encoding always writes 8 bytes.
 *
 * @code
 * static constexpr codec::timestamp format{};                  // Exactly 8 bytes
 * static constexpr codec::timestamp format{.min_length = 1};  // 1 to 8 bytes
 * static constexpr codec::nano_timestamp format{};             // Nanoseconds, exactly 8 bytes
 * @endcode
 */
template<typename Duration>
struct time_count {
	using value_type = std::chrono::sys_time<Duration>;

	size_t min_length = sizeof(uint64_t);

	[[nodiscard]]
	constexpr auto decode(byte_span in) const -> field<value_type> {
		if (in.size() < min_length || in.empty() || in.size() > sizeof(uint64_t)) {
			return std::unexpected(field_error::bad_length);
		}

		auto const raw = read_big_endian<uint64_t>(in);
		if (raw > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
			return std::unexpected(field_error::out_of_range);
		}
		return value_type{Duration{static_cast<int64_t>(raw)}};
	}

	[[nodiscard]]
	auto encode(value_type value, byte_writer& out) const -> encode_result {
		auto const count = value.time_since_epoch().count();
		if (count < 0) {
			return std::unexpected(field_error::out_of_range);
		}
		out.put_big_endian(static_cast<uint64_t>(count));
		return {};
	}

	[[nodiscard]]
	auto to_string(value_type value) const -> std::string {
		return std::format("{:%FT%T}Z", value);
	}
};

/// MISB Precision Time Stamp: microseconds since the POSIX epoch
using timestamp = time_count<std::chrono::microseconds>;

/// MISB Nano Precision Time Stamp: nanoseconds since the POSIX epoch
using nano_timestamp = time_count<std::chrono::nanoseconds>;

}  // namespace klv::codec
