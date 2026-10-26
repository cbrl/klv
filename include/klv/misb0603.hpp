/**
 * @file misb0603.hpp
 * @brief MISB ST 0603 - MISP Time System and Timestamps
 *
 * The MISP Time System counts SI seconds from the epoch 1970-01-01T00:00:00Z. It has no leap seconds. ST 0603 gives
 * two representations of a time in the MISP Time System. Each one is a 64-bit unsigned integer:
 * - The Precision Time Stamp counts microseconds. Use `klv::timestamp` and `codec::timestamp`.
 * - The Nano Precision Time Stamp counts nanoseconds. Use `klv::nano_timestamp` and `codec::nano_timestamp`.
 *
 * The Time Status byte gives the state of the clock that makes a time stamp. Each of the three items can be a
 * standalone KLV item with its own key:
 * @code
 * if (auto const time = klv::decode_packet<klv::misb0603::nano_precision_time_stamp_item>(bytes)) {
 *     use(time->value);
 * }
 * @endcode
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codecs/enumeration.hpp"
#include "klv/codecs/timestamp.hpp"
#include "klv/field.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <limits>
#include <optional>
#include <string>
#include <string_view>

namespace klv::misb0603 {

/**
 * @brief Precision Time Stamp of a Nano Precision Time Stamp (ST 0603 section 7.3)
 *
 * The conversion adds 500 ns, then truncates to microseconds. Thus, it rounds half up. Two different nanosecond values
 * can give the same microsecond value.
 */
[[nodiscard]]
constexpr auto to_precision(nano_timestamp value) noexcept -> timestamp {
	auto const truncated = std::chrono::floor<std::chrono::microseconds>(value);
	if (value - truncated >= std::chrono::nanoseconds{500}) {
		return truncated + std::chrono::microseconds{1};
	}
	return truncated;
}

/**
 * @brief Nano Precision Time Stamp of a Precision Time Stamp (ST 0603 section 7.3)
 *
 * The conversion multiplies by 1000, so it keeps the value. A time after 2262-04-11T23:47:16Z does not fit in
 * `nano_timestamp`, and gives no value.
 */
[[nodiscard]]
constexpr auto to_nano(timestamp value) noexcept -> std::optional<nano_timestamp> {
	constexpr auto limit = std::numeric_limits<std::chrono::nanoseconds::rep>::max() / 1000;
	auto const count = value.time_since_epoch().count();
	if (count > limit || count < -limit) {
		return std::nullopt;
	}
	return nano_timestamp{std::chrono::nanoseconds{count * 1000}};
}

/// Discontinuity in the count of time (Time Status bits 6 and 5)
enum class time_jump : uint8_t {
	none,  ///< Time increments forward in a linear fashion (bit 6 is 0)
	forward,  ///< Time jumped forward (bit 6 is 1, bit 5 is 0)
	reverse,  ///< Time jumped backward (bit 6 is 1, bit 5 is 1)
};

[[nodiscard]]
constexpr auto to_string(time_jump value) noexcept -> std::string_view {
	using enum time_jump;
	switch (value) {
		case none:
			return "None";
		case forward:
			return "Forward";
		case reverse:
			return "Reverse";
	}
	return {};
}

/// Time Status (ST 0603 Table 3): the state of the clock that makes a time stamp
struct time_status {
	/// True: the clock is locked to an absolute time reference (bit 7 is 0). False: the lock is unknown (bit 7 is 1).
	bool locked = true;
	time_jump jump = time_jump::none;

	friend auto operator==(time_status const&, time_status const&) -> bool = default;
};

/**
 * @brief Time Status byte
 *
 * Bit 7 is 1 when the lock is unknown. Bit 6 is 1 for a discontinuity. Then bit 5 is 1 for a reverse jump. Bits 4 to
 * 0 are reserved, and encoding writes them as 1. Decoding ignores them. It also ignores bit 5 when bit 6 is 0.
 */
struct time_status_codec {
	using value_type = time_status;

	static constexpr auto lock_unknown_bit = uint8_t{0x80};
	static constexpr auto discontinuity_bit = uint8_t{0x40};
	static constexpr auto reverse_bit = uint8_t{0x20};
	static constexpr auto reserved_bits = uint8_t{0x1F};

	[[nodiscard]]
	constexpr auto decode(byte_span in) const -> field<time_status> {
		if (in.size() != 1) {
			return std::unexpected(field_error::bad_length);
		}
		auto const bits = std::to_integer<uint8_t>(in[0]);
		auto jump = time_jump::none;
		if ((bits & discontinuity_bit) != 0) {
			jump = (bits & reverse_bit) != 0 ? time_jump::reverse : time_jump::forward;
		}
		return time_status{.locked = (bits & lock_unknown_bit) == 0, .jump = jump};
	}

	[[nodiscard]]
	auto encode(time_status const& value, byte_writer& out) const -> encode_result {
		if (misb0603::to_string(value.jump).empty()) {
			return std::unexpected(field_error::invalid_value);
		}
		auto bits = reserved_bits;
		if (value.jump != time_jump::none) {
			bits |= discontinuity_bit;
		}
		if (value.jump == time_jump::reverse) {
			bits |= reverse_bit;
		}
		if (!value.locked) {
			bits |= lock_unknown_bit;
		}
		out.put_u8(bits);
		return {};
	}

	[[nodiscard]]
	auto to_string(time_status const& value) const -> std::string {
		return std::format("{{locked: {}, jump: {}}}", value.locked, klv::detail::enum_text(value.jump));
	}
};

/// Standalone Precision Time Stamp item (SMPTE RP 210)
struct precision_time_stamp_item {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 03 07 02 01 01 01 05 00 00");
	static constexpr std::string_view name = "Precision Time Stamp";
	static constexpr codec::timestamp format{};
};

/// Standalone Nano Precision Time Stamp item (MISB ST 0807)
struct nano_precision_time_stamp_item {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 0E 01 01 02 0A 08 00 00");
	static constexpr std::string_view name = "Nano Precision Time Stamp";
	static constexpr codec::nano_timestamp format{};
};

/// Standalone Time Status item (MISB ST 0807)
struct time_status_item {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 0E 01 01 03 10 00 00 00");
	static constexpr std::string_view name = "Time Status";
	static constexpr time_status_codec format{};
};

}  // namespace klv::misb0603
