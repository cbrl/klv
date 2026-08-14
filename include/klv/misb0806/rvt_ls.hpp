/**
 * @file klv/misb0806/rvt_ls.hpp
 * @brief MISB ST 0806 Remote Video Terminal Local Set definition
 *
 * `rvt_ls` lists the fields in encode order. Checksum is last.
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/misb0806/fields.hpp"
#include "klv/set.hpp"

#include <string_view>

namespace klv::misb0806 {

/**
 * @brief MISB ST 0806 Remote Video Terminal Local Set
 *
 * The set usually arrives nested in MISB ST 0601 tag 73. A stand-alone packet ends with a CRC-32/MPEG-2 checksum
 * (tag 1). Packet decoding verifies it, and packet encoding computes it. A nested set has no checksum.
 */
struct rvt_ls {
	static constexpr std::string_view name = "RVT Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 01 02 00 00 00");

	using checksum_field = checksum;
	static constexpr auto checksum_function = crc32_mpeg2;

	using fields = field_list<
	    precision_time_stamp,
	    platform_true_airspeed,
	    platform_indicated_airspeed,
	    telemetry_accuracy_indicator,
	    frag_circle_radius,
	    frame_code,
	    rvt_ls_version_number,
	    video_data_rate,
	    digital_video_file_format,
	    user_defined_local_set,
	    poi_local_set,
	    aoi_local_set,
	    mgrs_zone,
	    mgrs_latitude_band_and_grid_square,
	    mgrs_easting,
	    mgrs_northing,
	    frame_center_mgrs_zone,
	    frame_center_mgrs_latitude_band_and_grid_square,
	    frame_center_mgrs_easting,
	    frame_center_mgrs_northing,
	    checksum>;
};

}  // namespace klv::misb0806
