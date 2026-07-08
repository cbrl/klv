/**
 * @file klv/misb0903/vmti_ls.hpp
 * @brief MISB ST 0903 Video Moving Target Indicator Local Set definition
 *
 * `vmti_ls` lists the fields in encode order: Precision Time Stamp first, Checksum last.
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/misb0903/fields.hpp"
#include "klv/set.hpp"

#include <string_view>

namespace klv::misb0903 {

/**
 * @brief MISB ST 0903 VMTI Local Set
 *
 * A stand-alone packet ends with the MISB ST 0601 checksum. The set has no checksum when it is nested in ST 0601 tag
 * 74.
 */
struct vmti_ls {
	static constexpr std::string_view name = "VMTI Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 03 06 00 00 00");

	using checksum_field = checksum;

	using fields = field_list<
	    precision_time_stamp,
	    vmti_system_name,
	    vmti_ls_version,
	    total_number_of_targets_detected,
	    number_of_targets_reported,
	    frame_number,
	    frame_width,
	    frame_height,
	    vmti_source_sensor,
	    vmti_horizontal_field_of_view,
	    vmti_vertical_field_of_view,
	    miis_id,
	    vtarget_series,
	    algorithm_series,
	    ontology_series,
	    checksum>;
};

}  // namespace klv::misb0903
