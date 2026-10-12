/**
 * @file klv/misb1403/check.hpp
 * @brief MISB ST 1403 check of one record
 */

#pragma once

#include "klv/misb0102/fields.hpp"
#include "klv/misb0601/uas_datalink_ls.hpp"
#include "klv/misb1206.hpp"
#include "klv/misb1403/items.hpp"
#include "klv/set/profile.hpp"
#include "klv/set/record.hpp"

#include <cstddef>
#include <vector>

namespace klv::misb1403 {

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
