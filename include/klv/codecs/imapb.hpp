/**
 * @file klv/codecs/imapb.hpp
 * @brief MISB ST 1201 IMAPB codec: a floating-point range mapped to an integer of variable length
 */

#pragma once

#include "klv/bytes/big_endian.hpp"
#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/error.hpp"
#include "klv/field.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <string>

namespace klv::codec {

/**
 * @brief Floating-point value in [min, max] that maps to an unsigned integer of 1 to 8 bytes (MISB ST 1201 IMAPB)
 *
 * The byte length sets the precision. Decoding accepts each length from 1 to 8 bytes and uses the mapping for that
 * length. Encoding writes `length` bytes and truncates to the raw value below, as ST 1201 specifies.
 *
 * A raw value with the most significant bit set is an ST 1201 special value. Decoding gives
 * `field_error::error_indicator` for infinity and NaN, and `field_error::invalid_value` for the other special values.
 * Encoding writes the special value for NaN and for infinity, and fails for a finite value outside [min, max].
 *
 * @code
 * static constexpr codec::imapb format{-900.0, 40'000.0, 3};
 * @endcode
 */
class imapb {
public:
	using value_type = double;

	static constexpr auto max_length = size_t{8};

	constexpr imapb(double min, double max, size_t length) :
	    min_(min),
	    max_(max),
	    length_(length),
	    b_pow_(ceil_log2(max - min)) {
		detail::require(min < max, "An IMAPB format needs min < max");
		detail::require(length >= 1 && length <= max_length, "An IMAPB format needs a length of 1 to 8 bytes");
	}

	[[nodiscard]]
	constexpr auto min() const noexcept -> double {
		return min_;
	}

	[[nodiscard]]
	constexpr auto max() const noexcept -> double {
		return max_;
	}

	/// Byte length that encoding writes
	[[nodiscard]]
	constexpr auto length() const noexcept -> size_t {
		return length_;
	}

	/// Convert a raw integer of `length` bytes to its real value
	[[nodiscard]]
	auto to_real(uint64_t raw, size_t length) const noexcept -> double {
		auto const scale = scale_for(length);
		return scale.reverse * (static_cast<double>(raw) - scale.offset) + min_;
	}

	/// Convert a real value in [min, max] to its raw integer of `length` bytes
	[[nodiscard]]
	auto to_raw(double value, size_t length) const noexcept -> uint64_t {
		auto const scale = scale_for(length);
		auto const scaled = std::floor(scale.forward * (value - min_) + scale.offset);

		// When max - min is a power of two, max maps to the special value flag. Use the largest normal value instead.
		auto const limit = largest_raw(length);
		if (scaled <= 0.0) {
			return 0;
		}
		if (scaled >= static_cast<double>(limit)) {
			return limit;
		}
		return static_cast<uint64_t>(scaled);
	}

	[[nodiscard]]
	auto decode(byte_span in) const -> field<double> {
		if (in.empty() || in.size() > max_length) {
			return std::unexpected(field_error::bad_length);
		}

		auto const first = std::to_integer<uint8_t>(in.front());
		if ((first & special_flag) != 0) {
			return std::unexpected(is_ieee_special(first) ? field_error::error_indicator : field_error::invalid_value);
		}

		auto const raw = read_big_endian<uint64_t>(in);
		if (raw > to_raw(max_, in.size())) {
			return std::unexpected(field_error::out_of_range);
		}
		return to_real(raw, in.size());
	}

	[[nodiscard]]
	auto encode(double value, byte_writer& out) const -> encode_result {
		if (std::isnan(value)) {
			put_special(std::signbit(value) ? negative_quiet_nan : positive_quiet_nan, out);
			return {};
		}
		if (std::isinf(value)) {
			put_special(value > 0.0 ? positive_infinity : negative_infinity, out);
			return {};
		}
		if (value < min_ || value > max_) {
			return std::unexpected(field_error::out_of_range);
		}

		out.put_big_endian(to_raw(value, length_), length_);
		return {};
	}

	/// Write the positive quiet NaN special value
	[[nodiscard]]
	auto encode_error_indicator(byte_writer& out) const -> encode_result {
		put_special(positive_quiet_nan, out);
		return {};
	}

	[[nodiscard]]
	auto to_string(double value) const -> std::string {
		return std::format("{:.6f}", value);
	}

private:
	// First byte of the ST 1201 special values. The other bits of the value are zero.
	static constexpr auto special_flag = uint8_t{0x80};
	static constexpr auto special_mask = uint8_t{0xF8};
	static constexpr auto positive_infinity = uint8_t{0xC8};
	static constexpr auto negative_infinity = uint8_t{0xE8};
	static constexpr auto positive_quiet_nan = uint8_t{0xD0};
	static constexpr auto negative_quiet_nan = uint8_t{0xF0};
	static constexpr auto positive_signaling_nan = uint8_t{0xD8};
	static constexpr auto negative_signaling_nan = uint8_t{0xF8};

	/// ST 1201 scale factors (sF, sR) and zero offset (zOffset) for one byte length
	struct scale_factors {
		double forward;
		double reverse;
		double offset;
	};

	/// Smallest integer p with 2^p >= value (ST 1201 bPow)
	[[nodiscard]]
	static constexpr auto ceil_log2(double value) noexcept -> int {
		auto power = 0;
		auto scale = 1.0;
		while (scale < value) {
			scale *= 2.0;
			++power;
		}
		while (scale / 2.0 >= value) {
			scale /= 2.0;
			--power;
		}
		return power;
	}

	/// Largest raw value without the special value flag
	[[nodiscard]]
	static constexpr auto largest_raw(size_t length) noexcept -> uint64_t {
		return (uint64_t{1} << (length * 8 - 1)) - 1;
	}

	[[nodiscard]]
	static constexpr auto is_ieee_special(uint8_t first) noexcept -> bool {
		switch (static_cast<uint8_t>(first & special_mask)) {
			case positive_infinity:
			case negative_infinity:
			case positive_quiet_nan:
			case negative_quiet_nan:
			case positive_signaling_nan:
			case negative_signaling_nan:
				return true;
			default:
				return false;
		}
	}

	[[nodiscard]]
	auto scale_for(size_t length) const noexcept -> scale_factors {
		auto const d_pow = static_cast<int>(length * 8) - 1;
		auto const forward = std::ldexp(1.0, d_pow - b_pow_);
		auto const reverse = std::ldexp(1.0, b_pow_ - d_pow);

		// A negative min gets an offset so that zero maps to an exact raw value
		auto const offset = min_ < 0.0 ? forward * min_ - std::floor(forward * min_) : 0.0;
		return {.forward = forward, .reverse = reverse, .offset = offset};
	}

	auto put_special(uint8_t first, byte_writer& out) const -> void {
		out.put_u8(first);
		out.put_big_endian(uint64_t{0}, length_ - 1);
	}

	double min_;
	double max_;
	size_t length_;
	int b_pow_;
};

}  // namespace klv::codec
