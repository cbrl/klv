/**
 * @file klv/misb1002/range_image_ls.hpp
 * @brief MISB ST 1002 Range Image Local Set definition
 *
 * `range_image_ls` lists the fields in encode order: Precision Time Stamp first, Checksum last.
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/misb1002/fields.hpp"
#include "klv/set.hpp"

#include <string_view>

namespace klv::misb1002 {

/**
 * @brief MISB ST 1002 Range Image Local Set
 *
 * The set usually arrives nested in MISB ST 0601 tag 97. A stand-alone packet ends with a CRC-16-CCITT checksum (tag
 * 21). This library treats it as a plain field. Packet decoding does not verify it, and packet encoding does not
 * compute it.
 */
struct range_image_ls {
	static constexpr std::string_view name = "Range Image Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 03 0C 00 00 00");

	using fields = field_list<
	    precision_time_stamp,
	    document_version,
	    range_image_enumerations_pack,
	    single_point_range_measurement,
	    single_point_range_measurement_uncertainty,
	    single_point_range_measurement_row,
	    single_point_range_measurement_column,
	    number_of_sections_in_x,
	    number_of_sections_in_y,
	    generalized_transformation_local_set,
	    section_data_pack,
	    checksum>;
};

}  // namespace klv::misb1002
