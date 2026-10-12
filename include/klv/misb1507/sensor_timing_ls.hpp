/**
 * @file klv/misb1507/sensor_timing_ls.hpp
 * @brief MISB ST 1507 Sensor Timing Local Set definition
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/misb1507/fields.hpp"
#include "klv/set.hpp"

#include <string_view>

namespace klv::misb1507 {

/**
 * @brief MISB ST 1507 Sensor Timing Local Set
 *
 * The set holds the Window Corners Pack, the Region List, or both. Packet decoding verifies the CRC-16-CCITT checksum
 * (tag 4), and packet encoding computes it.
 */
struct sensor_timing_ls {
	static constexpr std::string_view name = "Sensor Timing Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 02 01 00 00 00");

	using checksum_field = checksum;
	static constexpr auto checksum_function = crc16_ccitt;

	using fields = field_list<reference_time_pack, window_corners_pack, region_list, checksum>;
};

}  // namespace klv::misb1507
