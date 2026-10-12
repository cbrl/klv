/**
 * @file klv/misb1603/fields.hpp
 * @brief MISB ST 1603 Time Transfer Local Set field descriptors
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/misb1603/enumerations.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb1603 {

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

}  // namespace klv::misb1603
