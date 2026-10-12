/**
 * @file klv/misb0902/check.hpp
 * @brief MISB ST 0902 check of one record
 */

#pragma once

#include "klv/misb0102/fields.hpp"
#include "klv/misb0601/uas_datalink_ls.hpp"
#include "klv/misb0902/items.hpp"
#include "klv/set/profile.hpp"
#include "klv/set/record.hpp"

#include <cstddef>
#include <vector>

namespace klv::misb0902 {

/// True when `rec` reports an item of the requirement `which`
[[nodiscard]]
inline auto reports(record<misb0601::uas_datalink_ls> const& rec, item which) noexcept -> bool {
	namespace kv = misb0601;
	namespace sec = misb0102;
	using enum item;
	switch (which) {
		case precision_time_stamp:
			return klv::reports<kv::precision_time_stamp>(rec);
		case mission_id:
			return klv::reports<kv::mission_id>(rec);
		case platform_heading_angle:
			return klv::reports<kv::platform_heading_angle>(rec);
		case platform_pitch_angle:
			return klv::reports<kv::platform_pitch_angle>(rec) || klv::reports<kv::platform_pitch_angle_full>(rec);
		case platform_roll_angle:
			return klv::reports<kv::platform_roll_angle>(rec) || klv::reports<kv::platform_roll_angle_full>(rec);
		case platform_designation:
			return klv::reports<kv::platform_designation>(rec);
		case image_source_sensor:
			return klv::reports<kv::image_source_sensor>(rec);
		case image_coordinate_system:
			return klv::reports<kv::image_coordinate_system>(rec);
		case sensor_latitude:
			return klv::reports<kv::sensor_latitude>(rec);
		case sensor_longitude:
			return klv::reports<kv::sensor_longitude>(rec);
		case sensor_altitude:
			return klv::reports<kv::sensor_true_altitude>(rec) || klv::reports<kv::sensor_ellipsoid_height>(rec)
			    || klv::reports<kv::sensor_ellipsoid_height_extended>(rec);
		case sensor_horizontal_field_of_view:
			return klv::reports<kv::sensor_horizontal_field_of_view>(rec);
		case sensor_vertical_field_of_view:
			return klv::reports<kv::sensor_vertical_field_of_view>(rec);
		case sensor_relative_azimuth_angle:
			return klv::reports<kv::sensor_relative_azimuth_angle>(rec);
		case sensor_relative_elevation_angle:
			return klv::reports<kv::sensor_relative_elevation_angle>(rec);
		case sensor_relative_roll_angle:
			return klv::reports<kv::sensor_relative_roll_angle>(rec);
		case slant_range:
			return klv::reports<kv::slant_range>(rec);
		case target_width:
			return klv::reports<kv::target_width>(rec) || klv::reports<kv::target_width_extended>(rec);
		case frame_center_latitude:
			return klv::reports<kv::frame_center_latitude>(rec);
		case frame_center_longitude:
			return klv::reports<kv::frame_center_longitude>(rec);
		case frame_center_elevation:
			return klv::reports<kv::frame_center_elevation>(rec)
			    || klv::reports<kv::frame_center_height_above_ellipsoid>(rec);
		case security_classification:
			return klv::reports<kv::security_local_set, sec::security_classification>(rec);
		case classifying_country_coding_method:
			return klv::reports<kv::security_local_set, sec::classifying_country_coding_method>(rec);
		case classifying_country:
			return klv::reports<kv::security_local_set, sec::classifying_country>(rec);
		case object_country_coding_method:
			return klv::reports<kv::security_local_set, sec::object_country_coding_method>(rec);
		case object_country_codes:
			return klv::reports<kv::security_local_set, sec::object_country_codes>(rec);
		case security_metadata_version:
			return klv::reports<kv::security_local_set, sec::version>(rec);
		case uas_datalink_ls_version_number:
			return klv::reports<kv::uas_datalink_ls_version_number>(rec);
		case miis_core_identifier:
			return klv::reports<kv::miis_core_identifier>(rec);
		case count_:
			break;
	}
	return false;
}

/// The requirements of Table 1 that `rec` does not meet, in table order
[[nodiscard]]
inline auto check(record<misb0601::uas_datalink_ls> const& rec) -> std::vector<finding> {
	auto findings = std::vector<finding>{};
	for (auto index = size_t{0}; index < item_count; ++index) {
		auto const which = static_cast<item>(index);
		if (!reports(rec, which)) {
			findings.push_back({.item = which, .reason = shortfall::missing});
		}
		else if (
		    which == item::sensor_altitude && klv::reports<misb0601::sensor_ellipsoid_height>(rec)
		    && klv::reports<misb0601::sensor_ellipsoid_height_extended>(rec)
		) {
			findings.push_back({.item = which, .reason = shortfall::conflict});
		}
	}
	return findings;
}

}  // namespace klv::misb0902
