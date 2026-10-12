/**
 * @file klv/misb1504/state_vector_ls.hpp
 * @brief MISB ST 1504 State Vector Local Set definition
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/misb1504/fields.hpp"
#include "klv/misb1504/sdcc.hpp"
#include "klv/set.hpp"

#include <string_view>

namespace klv::misb1504 {

/**
 * @brief MISB ST 1504 State Vector Local Set
 *
 * The set needs the Precision Time Stamp, the Document Version, and the State Vector Pack. Packet decoding verifies
 * the CRC-16-CCITT (tag 3), and packet encoding computes it.
 */
struct state_vector_ls {
	static constexpr std::string_view name = "State Vector Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 01 03 00 00 00");

	using checksum_field = crc;
	static constexpr auto checksum_function = crc16_ccitt;

	using fields = field_list<
	    precision_time_stamp,
	    document_version,
	    crc,
	    state_vector_pack,
	    anomaly_type,
	    mean_anomaly_epoch,
	    state_vector_troc_pack,
	    eccentricity_vector_pack,
	    node_vector_pack,
	    specific_angular_momentum_pack,
	    mean_motion,
	    mu,
	    semilatus_rectum,
	    specific_energy,
	    sdcc_flp>;
};

}  // namespace klv::misb1504
