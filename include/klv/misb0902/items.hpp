/**
 * @file klv/misb0902/items.hpp
 * @brief MISB ST 0902 requirements (Table 1)
 */

#pragma once

#include "klv/set/profile.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace klv::misb0902 {

/// One requirement of MISB ST 0902 Table 1. A requirement with more than one tag accepts any one of its items.
enum class item : uint8_t {
	precision_time_stamp,  ///< Tag 2
	mission_id,  ///< Tag 3
	platform_heading_angle,  ///< Tag 5
	platform_pitch_angle,  ///< Tag 6 or tag 90
	platform_roll_angle,  ///< Tag 7 or tag 91
	platform_designation,  ///< Tag 10
	image_source_sensor,  ///< Tag 11
	image_coordinate_system,  ///< Tag 12
	sensor_latitude,  ///< Tag 13
	sensor_longitude,  ///< Tag 14
	sensor_altitude,  ///< Tag 15, tag 75, or tag 104
	sensor_horizontal_field_of_view,  ///< Tag 16
	sensor_vertical_field_of_view,  ///< Tag 17
	sensor_relative_azimuth_angle,  ///< Tag 18
	sensor_relative_elevation_angle,  ///< Tag 19
	sensor_relative_roll_angle,  ///< Tag 20
	slant_range,  ///< Tag 21
	target_width,  ///< Tag 22 or tag 96
	frame_center_latitude,  ///< Tag 23
	frame_center_longitude,  ///< Tag 24
	frame_center_elevation,  ///< Tag 25 or tag 78
	security_classification,  ///< Tag 48/1
	classifying_country_coding_method,  ///< Tag 48/2
	classifying_country,  ///< Tag 48/3
	object_country_coding_method,  ///< Tag 48/12
	object_country_codes,  ///< Tag 48/13
	security_metadata_version,  ///< Tag 48/22
	uas_datalink_ls_version_number,  ///< Tag 65
	miis_core_identifier,  ///< Tag 94
	count_,
};

/// Number of requirements
inline constexpr auto item_count = static_cast<size_t>(item::count_);

/// Name of the requirement, with its tags
[[nodiscard]]
constexpr auto to_string(item value) noexcept -> std::string_view {
	using enum item;
	switch (value) {
		case precision_time_stamp:
			return "Precision Time Stamp (2)";
		case mission_id:
			return "Mission ID (3)";
		case platform_heading_angle:
			return "Platform Heading Angle (5)";
		case platform_pitch_angle:
			return "Platform Pitch Angle (6 | 90)";
		case platform_roll_angle:
			return "Platform Roll Angle (7 | 91)";
		case platform_designation:
			return "Platform Designation (10)";
		case image_source_sensor:
			return "Image Source Sensor (11)";
		case image_coordinate_system:
			return "Image Coordinate System (12)";
		case sensor_latitude:
			return "Sensor Latitude (13)";
		case sensor_longitude:
			return "Sensor Longitude (14)";
		case sensor_altitude:
			return "Sensor Altitude (15 | 75 | 104)";
		case sensor_horizontal_field_of_view:
			return "Sensor Horizontal Field of View (16)";
		case sensor_vertical_field_of_view:
			return "Sensor Vertical Field of View (17)";
		case sensor_relative_azimuth_angle:
			return "Sensor Relative Azimuth Angle (18)";
		case sensor_relative_elevation_angle:
			return "Sensor Relative Elevation Angle (19)";
		case sensor_relative_roll_angle:
			return "Sensor Relative Roll Angle (20)";
		case slant_range:
			return "Slant Range (21)";
		case target_width:
			return "Target Width (22 | 96)";
		case frame_center_latitude:
			return "Frame Center Latitude (23)";
		case frame_center_longitude:
			return "Frame Center Longitude (24)";
		case frame_center_elevation:
			return "Frame Center Elevation (25 | 78)";
		case security_classification:
			return "Security Classification (48/1)";
		case classifying_country_coding_method:
			return "Classifying Country and Releasing Instructions Country Coding Method (48/2)";
		case classifying_country:
			return "Classifying Country (48/3)";
		case object_country_coding_method:
			return "Object Country Coding Method (48/12)";
		case object_country_codes:
			return "Object Country Codes (48/13)";
		case security_metadata_version:
			return "Security Metadata Version (48/22)";
		case uas_datalink_ls_version_number:
			return "UAS Datalink LS Version Number (65)";
		case miis_core_identifier:
			return "MIIS Core Identifier (94)";
		case count_:
			break;
	}
	return {};
}

using finding = klv::finding<item>;

}  // namespace klv::misb0902
