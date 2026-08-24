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
#include <tuple>

namespace klv {

/// MISB ST 1201 IMAPB value and its byte length, for a format whose length the producer chooses (see
/// `codec::imapb_value`)
struct imapb_value {
	static constexpr auto default_length = size_t{4};

	double value = 0.0;
	size_t length = default_length;  ///< 1 to 8 bytes. A longer length gives a smaller step.

	// Exact comparison. std::tie compares in the standard library, which keeps -Wfloat-equal quiet.
	friend auto operator==(imapb_value const& lhs, imapb_value const& rhs) -> bool {
		return std::tie(lhs.value, lhs.length) == std::tie(rhs.value, rhs.length);
	}
};

}  // namespace klv

namespace klv::codec {

/**
 * @brief Floating-point value in [min, max] that maps to an unsigned integer of 1 to 8 bytes (MISB ST 1201 IMAPB)
 *
 * The byte length sets the precision. Decoding accepts each length from 1 to 8 bytes and uses the mapping for that
 * length. Encoding writes `length` bytes and truncates to the raw value below, as ST 1201 specifies.
 *
 * A raw value with the most significant bit and any other bit set is an ST 1201 special value. Decoding gives:
 * - `field_error::error_indicator` for infinity and NaN
 * - `field_error::out_of_range` for "below minimum" and "above maximum"
 * - `field_error::invalid_value` for user-defined and reserved values
 *
 * Encoding writes the special value for NaN, for infinity, and for a finite value outside [min, max].
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
		auto const value = scale.reverse * (static_cast<double>(raw) - scale.offset) + min_;

		// ST 1201 requires an exact zero. Remove the rounding error of the zero offset.
		return std::abs(value) < scale.reverse / 2.0 ? 0.0 : value;
	}

	/// Convert a real value in [min, max] to its raw integer of `length` bytes
	[[nodiscard]]
	auto to_raw(double value, size_t length) const noexcept -> uint64_t {
		auto const scale = scale_for(length);
		auto const scaled = std::floor(scale.forward * (value - min_) + scale.offset);

		// When max - min is a power of two, max maps to the most significant bit alone. That is a normal value.
		auto const limit = uint64_t{1} << (length * 8 - 1);
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

		auto const raw = read_big_endian<uint64_t>(in);
		auto const flag = uint64_t{1} << (in.size() * 8 - 1);
		if ((raw & flag) != 0 && raw != flag) {
			return std::unexpected(special_error(std::to_integer<uint8_t>(in.front())));
		}
		if (raw > to_raw(max_, in.size())) {
			return std::unexpected(field_error::out_of_range);
		}
		return to_real(raw, in.size());
	}

	[[nodiscard]]
	auto encode(double value, byte_writer& out) const -> encode_result {
		if (std::isnan(value)) {
			put_special(std::signbit(value) ? negative_quiet_nan : positive_quiet_nan, out);
		}
		else if (std::isinf(value)) {
			put_special(value > 0.0 ? positive_infinity : negative_infinity, out);
		}
		else if (value < min_) {
			put_special(below_minimum, out);
		}
		else if (value > max_) {
			put_special(above_maximum, out);
		}
		else {
			out.put_big_endian(to_raw(value, length_), length_);
		}
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
	// First byte of the ST 1201 special values. The top 5 bits identify the kind. The other bits of an encoded value
	// are zero.
	static constexpr auto kind_mask = uint8_t{0xF8};
	static constexpr auto positive_infinity = uint8_t{0xC8};
	static constexpr auto negative_infinity = uint8_t{0xE8};
	static constexpr auto positive_quiet_nan = uint8_t{0xD0};
	static constexpr auto negative_quiet_nan = uint8_t{0xF0};
	static constexpr auto positive_signaling_nan = uint8_t{0xD8};
	static constexpr auto negative_signaling_nan = uint8_t{0xF8};
	static constexpr auto misb_special = uint8_t{0xE0};  ///< The next 3 bits identify the MISB special value
	static constexpr auto below_minimum = uint8_t{0xE0};
	static constexpr auto above_maximum = uint8_t{0xE1};

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

	/// Decode error for a special value with the given first byte
	[[nodiscard]]
	static constexpr auto special_error(uint8_t first) noexcept -> field_error {
		switch (static_cast<uint8_t>(first & kind_mask)) {
			case positive_infinity:
			case negative_infinity:
			case positive_quiet_nan:
			case negative_quiet_nan:
			case positive_signaling_nan:
			case negative_signaling_nan:
				return field_error::error_indicator;
			case misb_special:
				return first == below_minimum || first == above_maximum ? field_error::out_of_range
				                                                        : field_error::invalid_value;
			default:
				return field_error::invalid_value;
		}
	}

	[[nodiscard]]
	auto scale_for(size_t length) const noexcept -> scale_factors {
		auto const d_pow = static_cast<int>(length * 8) - 1;
		auto const forward = std::ldexp(1.0, d_pow - b_pow_);
		auto const reverse = std::ldexp(1.0, b_pow_ - d_pow);

		// A range that contains zero gets an offset, so that zero maps to an exact raw value
		auto const offset = min_ < 0.0 && max_ > 0.0 ? forward * min_ - std::floor(forward * min_) : 0.0;
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

/**
 * @brief IMAPB value whose byte length the producer chooses at run time (for example in MISB ST 1107)
 *
 * The value is a `klv::imapb_value`. Decoding keeps the byte length, so the value encodes again to the same bytes.
 * Encoding fails with `field_error::bad_length` for a length outside 1 to 8. Special values decode and encode as in
 * `codec::imapb`. The error indicator has `klv::imapb_value::default_length` bytes.
 *
 * @code
 * static constexpr codec::imapb_value format{-1.0e9, 1.0e9};
 * @endcode
 */
class imapb_value {
public:
	using value_type = klv::imapb_value;

	constexpr imapb_value(double min, double max) :
	    min_(min),
	    max_(max) {
		detail::require(min < max, "An IMAPB format needs min < max");
	}

	[[nodiscard]]
	constexpr auto min() const noexcept -> double {
		return min_;
	}

	[[nodiscard]]
	constexpr auto max() const noexcept -> double {
		return max_;
	}

	[[nodiscard]]
	auto decode(byte_span in) const -> field<klv::imapb_value> {
		if (in.empty() || in.size() > imapb::max_length) {
			return std::unexpected(field_error::bad_length);
		}
		return imapb{min_, max_, in.size()}.decode(in).transform([&](double value) {
			return klv::imapb_value{.value = value, .length = in.size()};
		});
	}

	[[nodiscard]]
	auto encode(klv::imapb_value const& value, byte_writer& out) const -> encode_result {
		if (value.length == 0 || value.length > imapb::max_length) {
			return std::unexpected(field_error::bad_length);
		}
		return imapb{min_, max_, value.length}.encode(value.value, out);
	}

	/// Write the positive quiet NaN special value
	[[nodiscard]]
	auto encode_error_indicator(byte_writer& out) const -> encode_result {
		return imapb{min_, max_, klv::imapb_value::default_length}.encode_error_indicator(out);
	}

	[[nodiscard]]
	auto to_string(klv::imapb_value const& value) const -> std::string {
		return std::format("{:.6f}", value.value);
	}

private:
	double min_;
	double max_;
};

}  // namespace klv::codec
