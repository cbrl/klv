/**
 * @file misb1403.hpp
 * @brief MISB ST 1403 - SAR Motion Imagery (SARMI) Threshold Metadata Sets
 *
 * MISB ST 1403 is a profile of the MISB ST 0601 UAS Datalink Local Set for SARMI streams. Each image frame of a
 * stream must report all of the items (requirement ST 1403-05). The MISB ST 1206 SAR Motion Imagery Local Set in tag
 * 95 holds the SAR items. The items depend on the product of the stream:
 * - SAR imagery: the items of Table 1.
 * - SAR coherent change products: the items of Table 1 and Table 2.
 *
 * `check` gives the requirements that one record does not meet. Rules of the check:
 * - An empty item (length of zero) does not meet a requirement (requirement ST 1403.2-07).
 * - Items 48/4, 48/5, and 48/6 are conditional in MISB ST 0102. The check does not require them.
 * - The packet functions verify and write the Checksum (tag 1). The check does not require it.
 * - Tag 75 and tag 104 exclude each other. A record with both items gives a `conflict` finding.
 * - The check reads the items of the top-level set. To check one segment of MISB ST 1607, merge the segment first.
 *
 * @code
 * for (auto const& finding : klv::misb1403::check(uas, klv::misb1403::product::imagery)) {
 *     // to_string(finding.item), to_string(finding.reason)
 * }
 * @endcode
 */

#pragma once

#include "klv/misb0102.hpp"
#include "klv/misb0601.hpp"
#include "klv/misb1206.hpp"
#include "klv/set/profile.hpp"
#include "klv/set/record.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

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

/// True when `rec` reports an item of the requirement `which`
[[nodiscard]]
inline auto reports(record<misb0601::uas_datalink_ls> const& rec, item which) noexcept -> bool {
	namespace kv = misb0601;
	namespace sec = misb0102;
	namespace sar = misb1206;
	using sar_set = kv::sar_motion_imagery_local_set;
	using enum item;
	switch (which) {
		case precision_time_stamp:
			return klv::reports<kv::precision_time_stamp>(rec);
		case mission_id:
			return klv::reports<kv::mission_id>(rec);
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
		case slant_range:
			return klv::reports<kv::slant_range>(rec);
		case frame_center_latitude:
			return klv::reports<kv::frame_center_latitude>(rec);
		case frame_center_longitude:
			return klv::reports<kv::frame_center_longitude>(rec);
		case corner_latitude_point_1:
			return klv::reports<kv::corner_latitude_point_1_full>(rec);
		case corner_longitude_point_1:
			return klv::reports<kv::corner_longitude_point_1_full>(rec);
		case corner_latitude_point_2:
			return klv::reports<kv::corner_latitude_point_2_full>(rec);
		case corner_longitude_point_2:
			return klv::reports<kv::corner_longitude_point_2_full>(rec);
		case corner_latitude_point_3:
			return klv::reports<kv::corner_latitude_point_3_full>(rec);
		case corner_longitude_point_3:
			return klv::reports<kv::corner_longitude_point_3_full>(rec);
		case corner_latitude_point_4:
			return klv::reports<kv::corner_latitude_point_4_full>(rec);
		case corner_longitude_point_4:
			return klv::reports<kv::corner_longitude_point_4_full>(rec);
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
		case sensor_ellipsoid_height:
			return klv::reports<kv::sensor_ellipsoid_height>(rec)
			    || klv::reports<kv::sensor_ellipsoid_height_extended>(rec);
		case frame_center_height_above_ellipsoid:
			return klv::reports<kv::frame_center_height_above_ellipsoid>(rec);
		case miis_core_identifier:
			return klv::reports<kv::miis_core_identifier>(rec);
		case grazing_angle:
			return klv::reports<sar_set, sar::grazing_angle>(rec);
		case ground_plane_squint_angle:
			return klv::reports<sar_set, sar::ground_plane_squint_angle>(rec);
		case look_direction:
			return klv::reports<sar_set, sar::look_direction>(rec);
		case image_plane:
			return klv::reports<sar_set, sar::image_plane>(rec);
		case range_resolution:
			return klv::reports<sar_set, sar::range_resolution>(rec);
		case cross_range_resolution:
			return klv::reports<sar_set, sar::cross_range_resolution>(rec);
		case range_image_plane_pixel_size:
			return klv::reports<sar_set, sar::range_image_plane_pixel_size>(rec);
		case cross_range_image_plane_pixel_size:
			return klv::reports<sar_set, sar::cross_range_image_plane_pixel_size>(rec);
		case image_rows:
			return klv::reports<sar_set, sar::image_rows>(rec);
		case image_columns:
			return klv::reports<sar_set, sar::image_columns>(rec);
		case range_direction_angle:
			return klv::reports<sar_set, sar::range_direction_angle>(rec);
		case true_north:
			return klv::reports<sar_set, sar::true_north>(rec);
		case range_layover_angle:
			return klv::reports<sar_set, sar::range_layover_angle>(rec);
		case document_version:
			return klv::reports<sar_set, sar::document_version>(rec);
		case reference_frame_precision_time_stamp:
			return klv::reports<sar_set, sar::reference_frame_precision_time_stamp>(rec);
		case reference_frame_grazing_angle:
			return klv::reports<sar_set, sar::reference_frame_grazing_angle>(rec);
		case reference_frame_ground_plane_squint_angle:
			return klv::reports<sar_set, sar::reference_frame_ground_plane_squint_angle>(rec);
		case reference_frame_range_direction_angle:
			return klv::reports<sar_set, sar::reference_frame_range_direction_angle>(rec);
		case reference_frame_range_layover_angle:
			return klv::reports<sar_set, sar::reference_frame_range_layover_angle>(rec);
		case count_:
			break;
	}
	return false;
}

/// The requirements for `kind` that `rec` does not meet, in table order
[[nodiscard]]
inline auto check(record<misb0601::uas_datalink_ls> const& rec, product kind) -> std::vector<finding> {
	auto const count = kind == product::imagery ? table_1_count : item_count;
	auto findings = std::vector<finding>{};
	for (auto index = size_t{0}; index < count; ++index) {
		auto const which = static_cast<item>(index);
		if (!reports(rec, which)) {
			findings.push_back({.item = which, .reason = shortfall::missing});
		}
		else if (
		    which == item::sensor_ellipsoid_height && klv::reports<misb0601::sensor_ellipsoid_height>(rec)
		    && klv::reports<misb0601::sensor_ellipsoid_height_extended>(rec)
		) {
			findings.push_back({.item = which, .reason = shortfall::conflict});
		}
	}
	return findings;
}

}  // namespace klv::misb1403
