/**
 * @file klv/misb1603/time_transfer_ls.hpp
 * @brief MISB ST 1603 Time Transfer Local Set definition
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/misb1603/fields.hpp"
#include "klv/set.hpp"

#include <string_view>

namespace klv::misb1603 {

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

}  // namespace klv::misb1603
