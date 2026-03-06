/**
 * @file klv/codecs/mapped.hpp
 * @brief Codec for fixed-point integers that map linearly to a floating-point range
 */

#pragma once

#include "klv/bytes/big_endian.hpp"
#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/error.hpp"
#include "klv/field.hpp"

#include <cmath>
#include <cstdint>
#include <expected>
#include <format>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>

namespace klv::codec {

/// Error value policy of a mapped field
enum class sentinel : uint8_t {
	none,  ///< Every raw value is a valid value
	reserve_min,  ///< Signed types only: the minimum raw value is the error value, and the domain is [-max, max]
};

/**
 * @brief Fixed-point integer that maps linearly to a floating-point range (MISB ST 0601 "mapped" format)
 *
 * The integer domain comes from `Int` and the sentinel policy:
 * - unsigned: [0, max], no error value
 * - signed with `sentinel::reserve_min`: [-max, max], `min` is the error value
 * - signed with `sentinel::none`: [min, max], no error value
 *
 * Decoding needs exactly `sizeof(Int)` bytes. Encoding rounds to the nearest integer. A NaN or out-of-range value
 * encodes as the error value when the format has one, and fails otherwise.
 *
 * @code
 * static constexpr codec::mapped<uint16_t> format{0.0, 360.0};
 * static constexpr codec::mapped<int32_t> format{-90.0, 90.0, codec::sentinel::reserve_min};
 * @endcode
 */
template<wire_integer Int>
class mapped {
public:
	using value_type = double;
	using int_type = Int;

	/// Map the default domain of `Int` (see the class description) to [min, max]
	constexpr mapped(double min, double max, sentinel error = sentinel::none) :
	    min_(min),
	    max_(max),
	    domain_min_(default_domain_min(error)),
	    domain_max_(std::numeric_limits<Int>::max()),
	    error_value_(
	        error == sentinel::reserve_min ? std::optional<Int>{std::numeric_limits<Int>::min()} : std::nullopt
	    ) {
		detail::require(min < max, "A mapped format needs min < max");
		detail::require(
		    std::is_signed_v<Int> || error != sentinel::reserve_min,
		    "sentinel::reserve_min needs a signed integer type"
		);
	}

	/// Map an explicit integer domain to [min, max]
	constexpr mapped(double min, double max, Int domain_min, Int domain_max, std::optional<Int> error_value) :
	    min_(min),
	    max_(max),
	    domain_min_(domain_min),
	    domain_max_(domain_max),
	    error_value_(error_value) {
		detail::require(min < max, "A mapped format needs min < max");
		detail::require(domain_min < domain_max, "A mapped format needs domain_min < domain_max");
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
	constexpr auto domain_min() const noexcept -> Int {
		return domain_min_;
	}

	[[nodiscard]]
	constexpr auto domain_max() const noexcept -> Int {
		return domain_max_;
	}

	[[nodiscard]]
	constexpr auto error_value() const noexcept -> std::optional<Int> {
		return error_value_;
	}

	/// Convert a raw integer in the domain to its real value
	[[nodiscard]]
	constexpr auto to_real(Int raw) const noexcept -> double {
		auto const offset = static_cast<double>(raw) - static_cast<double>(domain_min_);
		return offset * (max_ - min_) / domain_span() + min_;
	}

	/// Convert a real value in [min, max] to the nearest raw integer
	[[nodiscard]]
	auto to_raw(double value) const noexcept -> Int {
		auto const scaled = (value - min_) * domain_span() / (max_ - min_) + static_cast<double>(domain_min_);
		auto const rounded = std::round(scaled);

		if (rounded <= static_cast<double>(domain_min_)) {
			return domain_min_;
		}
		if (rounded >= static_cast<double>(domain_max_)) {
			return domain_max_;
		}
		return static_cast<Int>(rounded);
	}

	[[nodiscard]]
	constexpr auto decode(byte_span in) const -> field<double> {
		if (in.size() != sizeof(Int)) {
			return std::unexpected(field_error::bad_length);
		}

		auto const raw = read_big_endian<Int>(in);
		if (error_value_ && raw == *error_value_) {
			return std::unexpected(field_error::error_indicator);
		}
		if (raw < domain_min_ || raw > domain_max_) {
			return std::unexpected(field_error::out_of_range);
		}
		return to_real(raw);
	}

	[[nodiscard]]
	auto encode(double value, byte_writer& out) const -> encode_result {
		if (std::isnan(value) || value < min_ || value > max_) {
			if (error_value_) {
				out.put_big_endian(*error_value_);
				return {};
			}
			return std::unexpected(std::isnan(value) ? field_error::invalid_value : field_error::out_of_range);
		}

		out.put_big_endian(to_raw(value));
		return {};
	}

	[[nodiscard]]
	auto encode_error_indicator(byte_writer& out) const -> encode_result {
		if (!error_value_) {
			return std::unexpected(field_error::invalid_value);
		}
		out.put_big_endian(*error_value_);
		return {};
	}

	[[nodiscard]]
	auto to_string(double value) const -> std::string {
		return std::format("{:.6f}", value);
	}

private:
	[[nodiscard]]
	static constexpr auto default_domain_min(sentinel error) noexcept -> Int {
		if constexpr (std::is_unsigned_v<Int>) {
			return Int{0};
		}
		else {
			return error == sentinel::reserve_min ? static_cast<Int>(-std::numeric_limits<Int>::max())
			                                      : std::numeric_limits<Int>::min();
		}
	}

	[[nodiscard]]
	constexpr auto domain_span() const noexcept -> double {
		return static_cast<double>(domain_max_) - static_cast<double>(domain_min_);
	}

	double min_;
	double max_;
	Int domain_min_;
	Int domain_max_;
	std::optional<Int> error_value_;
};

}  // namespace klv::codec
