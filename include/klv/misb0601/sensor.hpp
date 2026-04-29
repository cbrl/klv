/**
 * @file klv/misb0601/sensor.hpp
 * @brief MISB ST 0601 fields: sensor position, pointing, field of view, and payloads
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/misb0601/enumerations.hpp"
#include "klv/misb0601/packs/frame_rate.hpp"
#include "klv/misb0601/packs/payloads.hpp"
#include "klv/misb0601/packs/view_ranges.hpp"
#include "klv/misb0601/packs/wavelength_record.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0601 {

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

struct generic_flag_data {
	static constexpr uint32_t tag = 47;
	static constexpr std::string_view name = "Generic Flag Data";
	static constexpr codec::integer<uint8_t> format{};
};

struct sensor_field_of_view_name {
	static constexpr uint32_t tag = 63;
	static constexpr std::string_view name = "Sensor Field of View Name";
	static constexpr codec::enumeration<field_of_view_name> format{.name_of = &to_string};
};

struct sensor_ellipsoid_height {
	static constexpr uint32_t tag = 75;
	static constexpr std::string_view name = "Sensor Ellipsoid Height";
	static constexpr std::string_view units = "m";
	static constexpr codec::mapped<uint16_t> format{-900.0, 19'000.0};
};

struct sensor_north_velocity {
	static constexpr uint32_t tag = 79;
	static constexpr std::string_view name = "Sensor North Velocity";
	static constexpr std::string_view units = "m/s";
	static constexpr codec::mapped<int16_t> format{-327.0, 327.0, codec::sentinel::reserve_min};
};

struct sensor_east_velocity {
	static constexpr uint32_t tag = 80;
	static constexpr std::string_view name = "Sensor East Velocity";
	static constexpr std::string_view units = "m/s";
	static constexpr codec::mapped<int16_t> format{-327.0, 327.0, codec::sentinel::reserve_min};
};

struct sensor_ellipsoid_height_extended {
	static constexpr uint32_t tag = 104;
	static constexpr std::string_view name = "Sensor Ellipsoid Height Extended";
	static constexpr std::string_view units = "m";
	static constexpr codec::imapb format{-900.0, 40'000.0, 3};
};

struct sensor_azimuth_rate {
	static constexpr uint32_t tag = 117;
	static constexpr std::string_view name = "Sensor Azimuth Rate";
	static constexpr std::string_view units = "deg/s";
	static constexpr codec::imapb format{-1'000.0, 1'000.0, 3};
};

struct sensor_elevation_rate {
	static constexpr uint32_t tag = 118;
	static constexpr std::string_view name = "Sensor Elevation Rate";
	static constexpr std::string_view units = "deg/s";
	static constexpr codec::imapb format{-1'000.0, 1'000.0, 3};
};

struct sensor_roll_rate {
	static constexpr uint32_t tag = 119;
	static constexpr std::string_view name = "Sensor Roll Rate";
	static constexpr std::string_view units = "deg/s";
	static constexpr codec::imapb format{-1'000.0, 1'000.0, 3};
};

struct onboard_mi_storage_percent_full {
	static constexpr uint32_t tag = 120;
	static constexpr std::string_view name = "On-board MI Storage Percent Full";
	static constexpr std::string_view units = "%";
	static constexpr codec::imapb format{0.0, 100.0, 2};
};

struct active_wavelength_list {
	static constexpr uint32_t tag = 121;
	static constexpr std::string_view name = "Active Wavelength List";
	static constexpr codec::ber_oid_list format{};
};

struct sensor_control_mode {
	static constexpr uint32_t tag = 126;
	static constexpr std::string_view name = "Sensor Control Mode";
	static constexpr codec::enumeration<control_mode> format{.name_of = &to_string};
};

struct sensor_frame_rate_pack {
	static constexpr uint32_t tag = 127;
	static constexpr std::string_view name = "Sensor Frame Rate Pack";
	static constexpr std::string_view units = "fps";
	static constexpr frame_rate_codec format{};
};

struct wavelengths_list {
	static constexpr uint32_t tag = 128;
	static constexpr std::string_view name = "Wavelengths List";
	static constexpr std::string_view units = "nm";
	static constexpr codec::series<wavelength_record_codec> format{};
};

struct onboard_mi_storage_capacity {
	static constexpr uint32_t tag = 133;
	static constexpr std::string_view name = "On-board MI Storage Capacity";
	static constexpr std::string_view units = "GB";
	static constexpr codec::integer<uint32_t> format{};
};

struct zoom_percentage {
	static constexpr uint32_t tag = 134;
	static constexpr std::string_view name = "Zoom Percentage";
	static constexpr std::string_view units = "%";
	static constexpr codec::imapb format{0.0, 100.0, 1};
};

struct payload_list {
	static constexpr uint32_t tag = 138;
	static constexpr std::string_view name = "Payload List";
	static constexpr payloads_codec format{};
};

/// Bit n set: payload n of the Payload List (tag 138) is active
struct active_payloads {
	static constexpr uint32_t tag = 139;
	static constexpr std::string_view name = "Active Payloads";
	static constexpr codec::integer<uint64_t> format{};
};

struct view_domain {
	static constexpr uint32_t tag = 142;
	static constexpr std::string_view name = "View Domain";
	static constexpr std::string_view units = "deg";
	static constexpr view_ranges_codec format{};
};

}  // namespace klv::misb0601
