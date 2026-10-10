/**
 * @file misb1603.hpp
 * @brief MISB ST 1603 - Time Transfer Metadata
 *
 * Time Transfer sends accurate time from a reference clock to a receptor clock. For example, a GPS receiver sends time
 * to a camera. ST 1603 defines two constructs:
 * - The Time Transfer Local Set gives the state of the receptor clock. It applies to the time stamp of the set or the
 *   pack that holds it: the parent time.
 * - The Nano Time Transfer Pack holds a Nano Precision Time Stamp (MISB ST 0603) and a Time Transfer Local Set.
 *
 * The Time Transfer Local Set is always inside another set or pack, so it has no checksum. Some items use the units
 * of the parent time. These are microseconds for a Precision Time Stamp, and nanoseconds in a Nano Time Transfer Pack.
 *
 * A Nano Time Transfer Pack can be a standalone KLV item (`nano_time_transfer_item`):
 * @code
 * if (auto const pack = klv::decode_packet<klv::misb1603::nano_time_transfer_item>(bytes)) {
 *     use(pack->value.time, pack->value.transfer);
 * }
 * @endcode
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/set.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <string_view>
#include <utility>

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

// Time Transfer Local Set items

/// Minor version of ST 1603, for example 3 for ST 1603.3
struct document_version {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Document Version";
	static constexpr codec::integer<uint8_t> format{.length = codec::variable_length};
};

/// Leap seconds that convert the parent time to UTC. For GPS, the value is the GPS leap seconds plus 11.
struct utc_leap_second_offset {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "UTC Leap Second Offset";
	static constexpr std::string_view units = "s";
	static constexpr codec::integer<int32_t> format{.length = codec::variable_length};
};

struct time_transfer_parameters {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Time Transfer Parameters";
	static constexpr transfer_parameters_codec format{};
};

/// Frequency of the pulse of the reference clock. Without this item, the frequency is 1.0 Hz.
struct synchronization_pulse_frequency {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Synchronization Pulse Frequency";
	static constexpr std::string_view units = "Hz";
	static constexpr codec::ieee754_variable format{};
};

/// Time since the receptor clock was last locked to the reference clock, in the units of the parent time. Without
/// this item, the time is 0 (locked).
struct unlock_time {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "Unlock Time";
	static constexpr codec::integer<uint64_t> format{.length = codec::variable_length};
};

/// During Slew Correction: the difference between the receptor clock and the reference clock, in the units of the
/// parent time. Without this item, the difference is 0.
struct last_synchronization_difference {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "Last Synchronization Difference";
	static constexpr codec::integer<uint64_t> format{.length = codec::variable_length};
};

/// Largest drift of the receptor clock when it is not locked. Without this item, the drift rate is unknown.
struct drift_rate {
	static constexpr uint32_t tag = 7;
	static constexpr std::string_view name = "Drift Rate";
	static constexpr std::string_view units = "us/s";
	static constexpr codec::ieee754_variable format{};
};

/// Latency of the time signal from the reference clock to the receptor clock. Without this item, the delay is 0.
struct signal_source_delay {
	static constexpr uint32_t tag = 8;
	static constexpr std::string_view name = "Signal Source Delay";
	static constexpr std::string_view units = "ns";
	static constexpr codec::integer<uint64_t> format{.length = codec::variable_length};
};

/// Standard deviation of the receptor clock time, in the units of the parent time. Without this item, the
/// uncertainty is unknown.
struct receptor_clock_uncertainty {
	static constexpr uint32_t tag = 9;
	static constexpr std::string_view name = "Receptor Clock Uncertainty";
	static constexpr codec::integer<uint64_t> format{.length = codec::variable_length};
};

/// MISB ST 1603 Time Transfer Local Set
struct time_transfer_ls {
	static constexpr std::string_view name = "Time Transfer Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 02 02 00 00 00");

	using fields = field_list<
	    document_version,
	    utc_leap_second_offset,
	    time_transfer_parameters,
	    synchronization_pulse_frequency,
	    unlock_time,
	    last_synchronization_difference,
	    drift_rate,
	    signal_source_delay,
	    receptor_clock_uncertainty>;
};

// Nano Time Transfer Pack

/// A Nano Precision Time Stamp and the Time Transfer Local Set that applies to it
struct nano_time_transfer {
	nano_timestamp time{};
	record<time_transfer_ls> transfer;  ///< Empty when the pack holds only the time stamp

	friend auto operator==(nano_time_transfer const&, nano_time_transfer const&) -> bool = default;
};

/**
 * @brief Nano Time Transfer Pack: an 8-byte Nano Precision Time Stamp, then the value of a Time Transfer Local Set
 *
 * The pack is a truncation pack, so the set is optional. A pack of 8 bytes decodes with an empty set. Encoding an
 * empty set writes only the time stamp.
 */
struct nano_time_transfer_codec {
	using value_type = nano_time_transfer;

	static constexpr auto time_length = sizeof(uint64_t);

	[[nodiscard]]
	auto decode(byte_span in) const -> field<nano_time_transfer> {
		if (in.size() < time_length) {
			return std::unexpected(field_error::bad_length);
		}
		auto const time = codec::nano_timestamp{}.decode(in.first(time_length));
		if (!time) {
			return std::unexpected(time.error());
		}
		auto transfer = codec::nested<time_transfer_ls>{}.decode(in.subspan(time_length));
		if (!transfer) {
			return std::unexpected(transfer.error());
		}
		return nano_time_transfer{.time = *time, .transfer = std::move(*transfer)};
	}

	[[nodiscard]]
	auto encode(nano_time_transfer const& value, byte_writer& out) const -> encode_result {
		if (auto const time = codec::nano_timestamp{}.encode(value.time, out); !time) {
			return time;
		}
		return codec::nested<time_transfer_ls>{}.encode(value.transfer, out);
	}

	[[nodiscard]]
	auto to_string(nano_time_transfer const& value) const -> std::string {
		return std::format(
		    "{{time: {}, transfer: {{{}}}}}",
		    codec::nano_timestamp{}.to_string(value.time),
		    codec::nested<time_transfer_ls>{}.to_string(value.transfer)
		);
	}
};

/// Standalone Nano Time Transfer Pack
struct nano_time_transfer_item {
	static constexpr universal_key key = ul("06 0E 2B 34 02 05 01 01 0E 01 03 02 09 00 00 00");
	static constexpr std::string_view name = "Nano Time Transfer Pack";
	static constexpr nano_time_transfer_codec format{};
};

}  // namespace klv::misb1603
