/**
 * @file misb0902.hpp
 * @brief MISB ST 0902 - Motion Imagery Sensor Minimum Metadata Set (MISMMS)
 *
 * The MISMMS is a profile of the MISB ST 0601 UAS Datalink Local Set. Table 1 lists the items that a stream must
 * report. The stream reports each item at least once every 30 seconds (requirement ST 0902.8-05). Thus, one packet
 * does not have to hold all of the items.
 * - `check` gives the requirements that one record does not meet.
 * - `report_monitor` checks the 30-second rule over a stream of records.
 *
 * Rules of the check:
 * - An empty item (length of zero) does not meet a requirement.
 * - Items 48/4, 48/5, and 48/6 are conditional in MISB ST 0102. The check does not require them.
 * - The packet functions verify and write the Checksum (tag 1). The check does not require it.
 * - Tag 75 and tag 104 exclude each other. A record with both items gives a `conflict` finding.
 * - The check reads the items of the top-level set. To check one segment of MISB ST 1607, merge the segment first.
 *
 * @code
 * for (auto const& finding : klv::misb0902::check(uas)) {
 *     // to_string(finding.item), to_string(finding.reason)
 * }
 * @endcode
 */

#pragma once

#include "klv/bytes/types.hpp"
#include "klv/misb0102.hpp"
#include "klv/misb0601.hpp"
#include "klv/set/profile.hpp"
#include "klv/set/record.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

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

/**
 * @brief Checks requirement ST 0902.8-05 over a stream: a report of each item at least once every 30 seconds
 *
 * Add the records in stream order. The Precision Time Stamp (tag 2) of a record gives its time. The interval of each
 * requirement starts at the first record. The monitor keeps the latest time, so an earlier time stamp does not move
 * the time back.
 * @code
 * auto monitor = klv::misb0902::report_monitor{};
 * for (auto const& uas : records) {
 *     monitor.add(uas);
 *     for (auto const late : monitor.overdue()) {
 *         // to_string(late)
 *     }
 * }
 * @endcode
 */
class report_monitor {
public:
	/// Longest interval that ST 0902 allows between two reports of an item
	static constexpr auto max_interval = std::chrono::seconds{30};

	/// Note the requirements that `rec` meets. A record without a Precision Time Stamp changes nothing and gives false.
	auto add(record<misb0601::uas_datalink_ls> const& rec) -> bool {
		auto const& time = rec.get<misb0601::precision_time_stamp>();
		if (!time) {
			return false;
		}
		if (!now_) {
			last_.fill(*time);
		}
		now_ = std::max(now_.value_or(*time), *time);
		for (auto index = size_t{0}; index < item_count; ++index) {
			if (reports(rec, static_cast<item>(index))) {
				last_[index] = std::max(last_[index], *time);
			}
		}
		return true;
	}

	/// The requirements with no report in the 30 seconds before the latest time stamp, in table order
	[[nodiscard]]
	auto overdue() const -> std::vector<item> {
		auto late = std::vector<item>{};
		if (!now_) {
			return late;
		}
		for (auto index = size_t{0}; index < item_count; ++index) {
			if (*now_ - last_[index] > max_interval) {
				late.push_back(static_cast<item>(index));
			}
		}
		return late;
	}

private:
	std::array<timestamp, item_count> last_{};
	std::optional<timestamp> now_;
};

}  // namespace klv::misb0902
