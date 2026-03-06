/**
 * @file misb0601.hpp
 * @brief MISB ST 0601 - UAS Datalink Local Set
 *
 * The primary metadata set of Unmanned Aircraft Systems motion imagery. Each struct below is a static field
 * descriptor. `uas_datalink_ls` lists the fields in encode order: Precision Time Stamp first, Checksum last.
 */

#pragma once

#include "bytes.hpp"
#include "codec.hpp"
#include "misb0102.hpp"
#include "set.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0601 {

struct checksum {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Checksum";
	static constexpr codec::integer<uint16_t> format{};
};

struct precision_time_stamp {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Precision Time Stamp";
	static constexpr codec::timestamp format{};
};

struct mission_id {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Mission ID";
	static constexpr codec::string format{.max_length = 127};
};

struct platform_tail_number {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Platform Tail Number";
	static constexpr codec::string format{.max_length = 127};
};

struct platform_heading_angle {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "Platform Heading Angle";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<uint16_t> format{0.0, 360.0};
};

struct platform_pitch_angle {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "Platform Pitch Angle";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int16_t> format{-20.0, 20.0, codec::sentinel::reserve_min};
};

struct platform_roll_angle {
	static constexpr uint32_t tag = 7;
	static constexpr std::string_view name = "Platform Roll Angle";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int16_t> format{-50.0, 50.0, codec::sentinel::reserve_min};
};

struct platform_true_airspeed {
	static constexpr uint32_t tag = 8;
	static constexpr std::string_view name = "Platform True Airspeed";
	static constexpr std::string_view units = "m/s";
	static constexpr codec::integer<uint8_t> format{};
};

struct platform_indicated_airspeed {
	static constexpr uint32_t tag = 9;
	static constexpr std::string_view name = "Platform Indicated Airspeed";
	static constexpr std::string_view units = "m/s";
	static constexpr codec::integer<uint8_t> format{};
};

struct platform_designation {
	static constexpr uint32_t tag = 10;
	static constexpr std::string_view name = "Platform Designation";
	static constexpr codec::string format{.max_length = 127};
};

struct image_source_sensor {
	static constexpr uint32_t tag = 11;
	static constexpr std::string_view name = "Image Source Sensor";
	static constexpr codec::string format{.max_length = 127};
};

struct image_coordinate_system {
	static constexpr uint32_t tag = 12;
	static constexpr std::string_view name = "Image Coordinate System";
	static constexpr codec::string format{.max_length = 127};
};

struct sensor_latitude {
	static constexpr uint32_t tag = 13;
	static constexpr std::string_view name = "Sensor Latitude";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-90.0, 90.0, codec::sentinel::reserve_min};
};

struct sensor_longitude {
	static constexpr uint32_t tag = 14;
	static constexpr std::string_view name = "Sensor Longitude";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-180.0, 180.0, codec::sentinel::reserve_min};
};

struct sensor_true_altitude {
	static constexpr uint32_t tag = 15;
	static constexpr std::string_view name = "Sensor True Altitude";
	static constexpr std::string_view units = "m";
	static constexpr codec::mapped<uint16_t> format{-900.0, 19'000.0};
};

struct sensor_horizontal_field_of_view {
	static constexpr uint32_t tag = 16;
	static constexpr std::string_view name = "Sensor Horizontal Field of View";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<uint16_t> format{0.0, 180.0};
};

struct sensor_vertical_field_of_view {
	static constexpr uint32_t tag = 17;
	static constexpr std::string_view name = "Sensor Vertical Field of View";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<uint16_t> format{0.0, 180.0};
};

struct sensor_relative_azimuth_angle {
	static constexpr uint32_t tag = 18;
	static constexpr std::string_view name = "Sensor Relative Azimuth Angle";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<uint32_t> format{0.0, 360.0};
};

struct sensor_relative_elevation_angle {
	static constexpr uint32_t tag = 19;
	static constexpr std::string_view name = "Sensor Relative Elevation Angle";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-180.0, 180.0, codec::sentinel::reserve_min};
};

struct sensor_relative_roll_angle {
	static constexpr uint32_t tag = 20;
	static constexpr std::string_view name = "Sensor Relative Roll Angle";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<uint32_t> format{0.0, 360.0};
};

struct slant_range {
	static constexpr uint32_t tag = 21;
	static constexpr std::string_view name = "Slant Range";
	static constexpr std::string_view units = "m";
	static constexpr codec::mapped<uint32_t> format{0.0, 5'000'000.0};
};

struct target_width {
	static constexpr uint32_t tag = 22;
	static constexpr std::string_view name = "Target Width";
	static constexpr std::string_view units = "m";
	static constexpr codec::mapped<uint16_t> format{0.0, 10'000.0};
};

struct frame_center_latitude {
	static constexpr uint32_t tag = 23;
	static constexpr std::string_view name = "Frame Center Latitude";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-90.0, 90.0, codec::sentinel::reserve_min};
};

struct frame_center_longitude {
	static constexpr uint32_t tag = 24;
	static constexpr std::string_view name = "Frame Center Longitude";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-180.0, 180.0, codec::sentinel::reserve_min};
};

struct frame_center_elevation {
	static constexpr uint32_t tag = 25;
	static constexpr std::string_view name = "Frame Center Elevation";
	static constexpr std::string_view units = "m";
	static constexpr codec::mapped<uint16_t> format{-900.0, 19'000.0};
};

struct target_location_latitude {
	static constexpr uint32_t tag = 40;
	static constexpr std::string_view name = "Target Location Latitude";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-90.0, 90.0, codec::sentinel::reserve_min};
};

struct target_location_longitude {
	static constexpr uint32_t tag = 41;
	static constexpr std::string_view name = "Target Location Longitude";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-180.0, 180.0, codec::sentinel::reserve_min};
};

struct target_location_elevation {
	static constexpr uint32_t tag = 42;
	static constexpr std::string_view name = "Target Location Elevation";
	static constexpr std::string_view units = "m";
	static constexpr codec::mapped<uint16_t> format{-900.0, 19'000.0};
};

struct security_local_set {
	static constexpr uint32_t tag = 48;
	static constexpr std::string_view name = "Security Local Set";
	static constexpr codec::nested<misb0102::security_ls> format{};
};

struct uas_datalink_ls_version_number {
	static constexpr uint32_t tag = 65;
	static constexpr std::string_view name = "UAS Datalink LS Version Number";
	static constexpr codec::integer<uint8_t> format{};
};

// =====================================================================================================================
// Set
// =====================================================================================================================

/// MISB ST 0601 UAS Datalink Local Set
struct uas_datalink_ls {
	static constexpr std::string_view name = "UAS Datalink Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 01 01 00 00 00");

	using checksum_field = checksum;

	using fields = field_list<
	    precision_time_stamp,
	    mission_id,
	    platform_tail_number,
	    platform_heading_angle,
	    platform_pitch_angle,
	    platform_roll_angle,
	    platform_true_airspeed,
	    platform_indicated_airspeed,
	    platform_designation,
	    image_source_sensor,
	    image_coordinate_system,
	    sensor_latitude,
	    sensor_longitude,
	    sensor_true_altitude,
	    sensor_horizontal_field_of_view,
	    sensor_vertical_field_of_view,
	    sensor_relative_azimuth_angle,
	    sensor_relative_elevation_angle,
	    sensor_relative_roll_angle,
	    slant_range,
	    target_width,
	    frame_center_latitude,
	    frame_center_longitude,
	    frame_center_elevation,
	    target_location_latitude,
	    target_location_longitude,
	    target_location_elevation,
	    security_local_set,
	    uas_datalink_ls_version_number,
	    checksum>;
};

}  // namespace klv::misb0601
