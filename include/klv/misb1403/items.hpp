/**
 * @file klv/misb1403/items.hpp
 * @brief MISB ST 1403 products and requirements (Table 1 and Table 2)
 */

#pragma once

#include "klv/set/profile.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace klv::misb1403 {

/// Product of a SARMI stream
enum class product : uint8_t {
	imagery,  ///< Sequential display of SAR imagery (Table 1)
	coherent_change,  ///< Sequential display of SAR coherent change products (Table 1 and Table 2)
};

/// One requirement of MISB ST 1403 Table 1 or Table 2
enum class item : uint8_t {
	// Table 1
	precision_time_stamp,  ///< Tag 2
	mission_id,  ///< Tag 3
	platform_designation,  ///< Tag 10
	image_source_sensor,  ///< Tag 11
	image_coordinate_system,  ///< Tag 12
	sensor_latitude,  ///< Tag 13
	sensor_longitude,  ///< Tag 14
	slant_range,  ///< Tag 21
	frame_center_latitude,  ///< Tag 23
	frame_center_longitude,  ///< Tag 24
	corner_latitude_point_1,  ///< Tag 82
	corner_longitude_point_1,  ///< Tag 83
	corner_latitude_point_2,  ///< Tag 84
	corner_longitude_point_2,  ///< Tag 85
	corner_latitude_point_3,  ///< Tag 86
	corner_longitude_point_3,  ///< Tag 87
	corner_latitude_point_4,  ///< Tag 88
	corner_longitude_point_4,  ///< Tag 89
	security_classification,  ///< Tag 48/1
	classifying_country_coding_method,  ///< Tag 48/2
	classifying_country,  ///< Tag 48/3
	object_country_coding_method,  ///< Tag 48/12
	object_country_codes,  ///< Tag 48/13
	security_metadata_version,  ///< Tag 48/22
	uas_datalink_ls_version_number,  ///< Tag 65
	sensor_ellipsoid_height,  ///< Tag 75 or tag 104
	frame_center_height_above_ellipsoid,  ///< Tag 78
	miis_core_identifier,  ///< Tag 94
	grazing_angle,  ///< Tag 95/1
	ground_plane_squint_angle,  ///< Tag 95/2
	look_direction,  ///< Tag 95/3
	image_plane,  ///< Tag 95/4
	range_resolution,  ///< Tag 95/5
	cross_range_resolution,  ///< Tag 95/6
	range_image_plane_pixel_size,  ///< Tag 95/7
	cross_range_image_plane_pixel_size,  ///< Tag 95/8
	image_rows,  ///< Tag 95/9
	image_columns,  ///< Tag 95/10
	range_direction_angle,  ///< Tag 95/11
	true_north,  ///< Tag 95/12
	range_layover_angle,  ///< Tag 95/13
	document_version,  ///< Tag 95/28

	// Table 2
	reference_frame_precision_time_stamp,  ///< Tag 95/23
	reference_frame_grazing_angle,  ///< Tag 95/24
	reference_frame_ground_plane_squint_angle,  ///< Tag 95/25
	reference_frame_range_direction_angle,  ///< Tag 95/26
	reference_frame_range_layover_angle,  ///< Tag 95/27

	count_,
};

/// Number of requirements in Table 1
inline constexpr auto table_1_count = static_cast<size_t>(item::document_version) + 1;

/// Number of requirements in Table 1 and Table 2
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
		case slant_range:
			return "Slant Range (21)";
		case frame_center_latitude:
			return "Frame Center Latitude (23)";
		case frame_center_longitude:
			return "Frame Center Longitude (24)";
		case corner_latitude_point_1:
			return "Corner Latitude Point 1 (82)";
		case corner_longitude_point_1:
			return "Corner Longitude Point 1 (83)";
		case corner_latitude_point_2:
			return "Corner Latitude Point 2 (84)";
		case corner_longitude_point_2:
			return "Corner Longitude Point 2 (85)";
		case corner_latitude_point_3:
			return "Corner Latitude Point 3 (86)";
		case corner_longitude_point_3:
			return "Corner Longitude Point 3 (87)";
		case corner_latitude_point_4:
			return "Corner Latitude Point 4 (88)";
		case corner_longitude_point_4:
			return "Corner Longitude Point 4 (89)";
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
		case sensor_ellipsoid_height:
			return "Sensor Ellipsoid Height (75 | 104)";
		case frame_center_height_above_ellipsoid:
			return "Frame Center Height Above Ellipsoid (78)";
		case miis_core_identifier:
			return "MIIS Core Identifier (94)";
		case grazing_angle:
			return "Grazing Angle (95/1)";
		case ground_plane_squint_angle:
			return "Ground Plane Squint Angle (95/2)";
		case look_direction:
			return "Look Direction (95/3)";
		case image_plane:
			return "Image Plane (95/4)";
		case range_resolution:
			return "Range Resolution (95/5)";
		case cross_range_resolution:
			return "Cross-Range Resolution (95/6)";
		case range_image_plane_pixel_size:
			return "Range Image Plane Pixel Size (95/7)";
		case cross_range_image_plane_pixel_size:
			return "Cross-Range Image Plane Pixel Size (95/8)";
		case image_rows:
			return "Image Rows (95/9)";
		case image_columns:
			return "Image Columns (95/10)";
		case range_direction_angle:
			return "Range Direction Angle (95/11)";
		case true_north:
			return "True North (95/12)";
		case range_layover_angle:
			return "Range Layover Angle (95/13)";
		case document_version:
			return "Document Version (95/28)";
		case reference_frame_precision_time_stamp:
			return "Reference Frame Precision Time Stamp (95/23)";
		case reference_frame_grazing_angle:
			return "Reference Frame Grazing Angle (95/24)";
		case reference_frame_ground_plane_squint_angle:
			return "Reference Frame Ground Plane Squint Angle (95/25)";
		case reference_frame_range_direction_angle:
			return "Reference Frame Range Direction Angle (95/26)";
		case reference_frame_range_layover_angle:
			return "Reference Frame Range Layover Angle (95/27)";
		case count_:
			break;
	}
	return {};
}

using finding = klv::finding<item>;

}  // namespace klv::misb1403
