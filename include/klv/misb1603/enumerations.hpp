/**
 * @file klv/misb1603/enumerations.hpp
 * @brief MISB ST 1603 enumerations and the Time Transfer Parameters (tag 3)
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/field.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <string_view>

namespace klv::misb1603 {

/// State of the reference clock (Time Transfer Parameters bits 1 and 0, ST 1603 Table 2)
enum class reference_source : uint8_t {
	unknown = 0,
	not_synchronized = 1,  ///< The reference clock is not synchronous to an atomic source
	synchronized = 2,  ///< The reference clock is synchronous to an atomic source, for example TAI or GPS
};

[[nodiscard]]
constexpr auto to_string(reference_source value) noexcept -> std::string_view {
	using enum reference_source;
	switch (value) {
		case unknown:
			return "Unknown";
		case not_synchronized:
			return "Not Synchronous to an Atomic Source";
		case synchronized:
			return "Synchronous to an Atomic Source";
	}
	return {};
}

/// Method that corrects the receptor clock after the lock returns (Time Transfer Parameters bits 3 and 2, Table 3)
enum class correction_method : uint8_t {
	unknown = 0,  ///< No correction, or a method other than Jam Correction or Slew Correction
	jam = 1,  ///< The receptor clock changes to the reference time at once. Time can jump backward.
	slew = 2,  ///< The receptor clock runs slower until the two clocks agree. Time does not jump backward.
};

[[nodiscard]]
constexpr auto to_string(correction_method value) noexcept -> std::string_view {
	using enum correction_method;
	switch (value) {
		case unknown:
			return "Unknown";
		case jam:
			return "Jam Correction";
		case slew:
			return "Slew Correction";
	}
	return {};
}

/// System that sends the time to the receptor clock (Time Transfer Parameters bits 7 to 4, Table 4)
enum class transfer_method : uint8_t {
	unknown = 0,
	gps_pps = 1,  ///< GPS pulse per second
	ptp_v1 = 2,  ///< Precision Time Protocol, IEEE 1588-2002
	ptp_v2 = 3,  ///< Precision Time Protocol, IEEE 1588-2008
	ntp_v3 = 4,  ///< Network Time Protocol, RFC 1305
	ntp_v4 = 5,  ///< Network Time Protocol, RFC 5905
	irig_a = 6,
	irig_b = 7,
};

[[nodiscard]]
constexpr auto to_string(transfer_method value) noexcept -> std::string_view {
	using enum transfer_method;
	switch (value) {
		case unknown:
			return "Unknown";
		case gps_pps:
			return "GPS PPS";
		case ptp_v1:
			return "PTP Version 1";
		case ptp_v2:
			return "PTP Version 2";
		case ntp_v3:
			return "NTP Version 3";
		case ntp_v4:
			return "NTP Version 4";
		case irig_a:
			return "IRIG-A";
		case irig_b:
			return "IRIG-B";
	}
	return {};
}

/// Time Transfer Parameters (tag 3)
struct transfer_parameters {
	reference_source source = reference_source::unknown;
	correction_method correction = correction_method::unknown;
	transfer_method method = transfer_method::unknown;

	friend auto operator==(transfer_parameters const&, transfer_parameters const&) -> bool = default;
};

/**
 * @brief Time Transfer Parameters: three values in one byte
 *
 * Bits 7 to 4 hold the Time Transfer Method. Bits 3 and 2 hold the Correction Method. Bits 1 and 0 hold the Reference
 * Source. A later version of ST 1603 can add more significant bytes. Decoding reads the last byte and ignores the
 * bytes before it. Encoding writes one byte. A reserved value fails with `field_error::invalid_value`.
 */
struct transfer_parameters_codec {
	using value_type = transfer_parameters;

	[[nodiscard]]
	constexpr auto decode(byte_span in) const -> field<transfer_parameters> {
		if (in.empty()) {
			return std::unexpected(field_error::bad_length);
		}
		auto const bits = std::to_integer<uint8_t>(in.back());
		auto const value = transfer_parameters{
		    .source = static_cast<reference_source>(bits & 0x03U),
		    .correction = static_cast<correction_method>((bits >> 2U) & 0x03U),
		    .method = static_cast<transfer_method>(bits >> 4U),
		};
		if (!is_defined(value)) {
			return std::unexpected(field_error::invalid_value);
		}
		return value;
	}

	[[nodiscard]]
	auto encode(transfer_parameters const& value, byte_writer& out) const -> encode_result {
		if (!is_defined(value)) {
			return std::unexpected(field_error::invalid_value);
		}
		auto const method = static_cast<unsigned>(value.method) << 4U;
		auto const correction = static_cast<unsigned>(value.correction) << 2U;
		out.put_u8(static_cast<uint8_t>(method | correction | static_cast<unsigned>(value.source)));
		return {};
	}

	[[nodiscard]]
	auto to_string(transfer_parameters const& value) const -> std::string {
		return std::format(
		    "{{source: {}, correction: {}, method: {}}}",
		    misb1603::to_string(value.source),
		    misb1603::to_string(value.correction),
		    misb1603::to_string(value.method)
		);
	}

private:
	[[nodiscard]]
	static constexpr auto is_defined(transfer_parameters const& value) noexcept -> bool {
		return !misb1603::to_string(value.source).empty() && !misb1603::to_string(value.correction).empty()
		    && !misb1603::to_string(value.method).empty();
	}
};

}  // namespace klv::misb1603
