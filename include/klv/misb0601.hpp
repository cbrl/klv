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

// =====================================================================================================================
// Enumerations
// =====================================================================================================================

/// Icing detector state (tag 34)
enum class icing_state : uint8_t {
	detector_off = 0,
	no_icing_detected = 1,
	icing_detected = 2,
};

[[nodiscard]]
constexpr auto to_string(icing_state value) -> std::string_view {
	using enum icing_state;
	switch (value) {
		case detector_off:
			return "Detector Off";
		case no_icing_detected:
			return "No Icing Detected";
		case icing_detected:
			return "Icing Detected";
	}
	return {};
}

/// Sensor field of view name (tag 63)
enum class field_of_view_name : uint8_t {
	ultranarrow = 0,
	narrow = 1,
	medium = 2,
	wide = 3,
	ultrawide = 4,
	narrow_medium = 5,
	ultranarrow_2x = 6,
	ultranarrow_4x = 7,
	continuous_zoom = 8,
};

[[nodiscard]]
constexpr auto to_string(field_of_view_name value) -> std::string_view {
	using enum field_of_view_name;
	switch (value) {
		case ultranarrow:
			return "Ultranarrow";
		case narrow:
			return "Narrow";
		case medium:
			return "Medium";
		case wide:
			return "Wide";
		case ultrawide:
			return "Ultrawide";
		case narrow_medium:
			return "Narrow Medium";
		case ultranarrow_2x:
			return "2x Ultranarrow";
		case ultranarrow_4x:
			return "4x Ultranarrow";
		case continuous_zoom:
			return "Continuous Zoom";
	}
	return {};
}

/// Operational mode of the platform (tag 77)
enum class operation_type : uint8_t {
	other = 0,
	operational = 1,
	training = 2,
	exercise = 3,
	maintenance = 4,
	test = 5,
};

[[nodiscard]]
constexpr auto to_string(operation_type value) -> std::string_view {
	using enum operation_type;
	switch (value) {
		case other:
			return "Other";
		case operational:
			return "Operational";
		case training:
			return "Training";
		case exercise:
			return "Exercise";
		case maintenance:
			return "Maintenance";
		case test:
			return "Test";
	}
	return {};
}

/// Operational state of the platform (tag 125)
enum class platform_state : uint8_t {
	active = 0,
	preflight = 1,
	preflight_taxiing = 2,
	run_up = 3,
	take_off = 4,
	ingress = 5,
	manual_operation = 6,
	automated_orbit = 7,
	transitioning = 8,
	egress = 9,
	landing = 10,
	landed_taxiing = 11,
	landed_parked = 12,
};

[[nodiscard]]
constexpr auto to_string(platform_state value) -> std::string_view {
	using enum platform_state;
	switch (value) {
		case active:
			return "Active";
		case preflight:
			return "Pre-flight";
		case preflight_taxiing:
			return "Pre-flight-taxiing";
		case run_up:
			return "Run-up";
		case take_off:
			return "Take-off";
		case ingress:
			return "Ingress";
		case manual_operation:
			return "Manual operation";
		case automated_orbit:
			return "Automated-orbit";
		case transitioning:
			return "Transitioning";
		case egress:
			return "Egress";
		case landing:
			return "Landing";
		case landed_taxiing:
			return "Landed-taxiing";
		case landed_parked:
			return "Landed-Parked";
	}
	return {};
}

/// Control mode of the sensor (tag 126)
enum class control_mode : uint8_t {
	off = 0,
	home_position = 1,
	uncontrolled = 2,
	manual_control = 3,
	calibrating = 4,
	auto_holding_position = 5,
	auto_tracking = 6,
};

[[nodiscard]]
constexpr auto to_string(control_mode value) -> std::string_view {
	using enum control_mode;
	switch (value) {
		case off:
			return "Off";
		case home_position:
			return "Home Position";
		case uncontrolled:
			return "Uncontrolled";
		case manual_control:
			return "Manual Control";
		case calibrating:
			return "Calibrating";
		case auto_holding_position:
			return "Auto - Holding Position";
		case auto_tracking:
			return "Auto - Tracking";
	}
	return {};
}

// =====================================================================================================================
// Fields
// =====================================================================================================================

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

struct offset_corner_latitude_point_1 {
	static constexpr uint32_t tag = 26;
	static constexpr std::string_view name = "Offset Corner Latitude Point 1";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int16_t> format{-0.075, 0.075, codec::sentinel::reserve_min};
};

struct offset_corner_longitude_point_1 {
	static constexpr uint32_t tag = 27;
	static constexpr std::string_view name = "Offset Corner Longitude Point 1";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int16_t> format{-0.075, 0.075, codec::sentinel::reserve_min};
};

struct offset_corner_latitude_point_2 {
	static constexpr uint32_t tag = 28;
	static constexpr std::string_view name = "Offset Corner Latitude Point 2";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int16_t> format{-0.075, 0.075, codec::sentinel::reserve_min};
};

struct offset_corner_longitude_point_2 {
	static constexpr uint32_t tag = 29;
	static constexpr std::string_view name = "Offset Corner Longitude Point 2";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int16_t> format{-0.075, 0.075, codec::sentinel::reserve_min};
};

struct offset_corner_latitude_point_3 {
	static constexpr uint32_t tag = 30;
	static constexpr std::string_view name = "Offset Corner Latitude Point 3";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int16_t> format{-0.075, 0.075, codec::sentinel::reserve_min};
};

struct offset_corner_longitude_point_3 {
	static constexpr uint32_t tag = 31;
	static constexpr std::string_view name = "Offset Corner Longitude Point 3";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int16_t> format{-0.075, 0.075, codec::sentinel::reserve_min};
};

struct offset_corner_latitude_point_4 {
	static constexpr uint32_t tag = 32;
	static constexpr std::string_view name = "Offset Corner Latitude Point 4";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int16_t> format{-0.075, 0.075, codec::sentinel::reserve_min};
};

struct offset_corner_longitude_point_4 {
	static constexpr uint32_t tag = 33;
	static constexpr std::string_view name = "Offset Corner Longitude Point 4";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int16_t> format{-0.075, 0.075, codec::sentinel::reserve_min};
};

struct icing_detected {
	static constexpr uint32_t tag = 34;
	static constexpr std::string_view name = "Icing Detected";
	static constexpr codec::enumeration<icing_state> format{.name_of = &to_string};
};

struct wind_direction {
	static constexpr uint32_t tag = 35;
	static constexpr std::string_view name = "Wind Direction";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<uint16_t> format{0.0, 360.0};
};

struct wind_speed {
	static constexpr uint32_t tag = 36;
	static constexpr std::string_view name = "Wind Speed";
	static constexpr std::string_view units = "m/s";
	static constexpr codec::mapped<uint8_t> format{0.0, 100.0};
};

struct static_pressure {
	static constexpr uint32_t tag = 37;
	static constexpr std::string_view name = "Static Pressure";
	static constexpr std::string_view units = "mbar";
	static constexpr codec::mapped<uint16_t> format{0.0, 5'000.0};
};

struct density_altitude {
	static constexpr uint32_t tag = 38;
	static constexpr std::string_view name = "Density Altitude";
	static constexpr std::string_view units = "m";
	static constexpr codec::mapped<uint16_t> format{-900.0, 19'000.0};
};

struct outside_air_temperature {
	static constexpr uint32_t tag = 39;
	static constexpr std::string_view name = "Outside Air Temperature";
	static constexpr std::string_view units = "degC";
	static constexpr codec::integer<int8_t> format{};
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

struct target_track_gate_width {
	static constexpr uint32_t tag = 43;
	static constexpr std::string_view name = "Target Track Gate Width";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::mapped<uint8_t> format{0.0, 510.0};
};

struct target_track_gate_height {
	static constexpr uint32_t tag = 44;
	static constexpr std::string_view name = "Target Track Gate Height";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::mapped<uint8_t> format{0.0, 510.0};
};

struct target_error_estimate_ce90 {
	static constexpr uint32_t tag = 45;
	static constexpr std::string_view name = "Target Error Estimate - CE90";
	static constexpr std::string_view units = "m";
	static constexpr codec::mapped<uint16_t> format{0.0, 4'095.0};
};

struct target_error_estimate_le90 {
	static constexpr uint32_t tag = 46;
	static constexpr std::string_view name = "Target Error Estimate - LE90";
	static constexpr std::string_view units = "m";
	static constexpr codec::mapped<uint16_t> format{0.0, 4'095.0};
};

struct generic_flag_data {
	static constexpr uint32_t tag = 47;
	static constexpr std::string_view name = "Generic Flag Data";
	static constexpr codec::integer<uint8_t> format{};
};

struct security_local_set {
	static constexpr uint32_t tag = 48;
	static constexpr std::string_view name = "Security Local Set";
	static constexpr codec::nested<misb0102::security_ls> format{};
};

struct differential_pressure {
	static constexpr uint32_t tag = 49;
	static constexpr std::string_view name = "Differential Pressure";
	static constexpr std::string_view units = "mbar";
	static constexpr codec::mapped<uint16_t> format{0.0, 5'000.0};
};

struct platform_angle_of_attack {
	static constexpr uint32_t tag = 50;
	static constexpr std::string_view name = "Platform Angle of Attack";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int16_t> format{-20.0, 20.0, codec::sentinel::reserve_min};
};

struct platform_vertical_speed {
	static constexpr uint32_t tag = 51;
	static constexpr std::string_view name = "Platform Vertical Speed";
	static constexpr std::string_view units = "m/s";
	static constexpr codec::mapped<int16_t> format{-180.0, 180.0, codec::sentinel::reserve_min};
};

struct platform_sideslip_angle {
	static constexpr uint32_t tag = 52;
	static constexpr std::string_view name = "Platform Sideslip Angle";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int16_t> format{-20.0, 20.0, codec::sentinel::reserve_min};
};

struct airfield_barometric_pressure {
	static constexpr uint32_t tag = 53;
	static constexpr std::string_view name = "Airfield Barometric Pressure";
	static constexpr std::string_view units = "mbar";
	static constexpr codec::mapped<uint16_t> format{0.0, 5'000.0};
};

struct airfield_elevation {
	static constexpr uint32_t tag = 54;
	static constexpr std::string_view name = "Airfield Elevation";
	static constexpr std::string_view units = "m";
	static constexpr codec::mapped<uint16_t> format{-900.0, 19'000.0};
};

struct relative_humidity {
	static constexpr uint32_t tag = 55;
	static constexpr std::string_view name = "Relative Humidity";
	static constexpr std::string_view units = "%";
	static constexpr codec::mapped<uint8_t> format{0.0, 100.0};
};

struct platform_ground_speed {
	static constexpr uint32_t tag = 56;
	static constexpr std::string_view name = "Platform Ground Speed";
	static constexpr std::string_view units = "m/s";
	static constexpr codec::integer<uint8_t> format{};
};

struct ground_range {
	static constexpr uint32_t tag = 57;
	static constexpr std::string_view name = "Ground Range";
	static constexpr std::string_view units = "m";
	static constexpr codec::mapped<uint32_t> format{0.0, 5'000'000.0};
};

struct platform_fuel_remaining {
	static constexpr uint32_t tag = 58;
	static constexpr std::string_view name = "Platform Fuel Remaining";
	static constexpr std::string_view units = "kg";
	static constexpr codec::mapped<uint16_t> format{0.0, 10'000.0};
};

struct platform_call_sign {
	static constexpr uint32_t tag = 59;
	static constexpr std::string_view name = "Platform Call Sign";
	static constexpr codec::string format{.max_length = 127};
};

struct weapon_load {
	static constexpr uint32_t tag = 60;
	static constexpr std::string_view name = "Weapon Load";
	static constexpr codec::integer<uint16_t> format{};
};

struct weapon_fired {
	static constexpr uint32_t tag = 61;
	static constexpr std::string_view name = "Weapon Fired";
	static constexpr codec::integer<uint8_t> format{};
};

struct laser_prf_code {
	static constexpr uint32_t tag = 62;
	static constexpr std::string_view name = "Laser PRF Code";
	static constexpr codec::integer<uint16_t> format{};
};

struct sensor_field_of_view_name {
	static constexpr uint32_t tag = 63;
	static constexpr std::string_view name = "Sensor Field of View Name";
	static constexpr codec::enumeration<field_of_view_name> format{.name_of = &to_string};
};

struct platform_magnetic_heading {
	static constexpr uint32_t tag = 64;
	static constexpr std::string_view name = "Platform Magnetic Heading";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<uint16_t> format{0.0, 360.0};
};

struct uas_datalink_ls_version_number {
	static constexpr uint32_t tag = 65;
	static constexpr std::string_view name = "UAS Datalink LS Version Number";
	static constexpr codec::integer<uint8_t> format{};
};

struct alternate_platform_latitude {
	static constexpr uint32_t tag = 67;
	static constexpr std::string_view name = "Alternate Platform Latitude";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-90.0, 90.0, codec::sentinel::reserve_min};
};

struct alternate_platform_longitude {
	static constexpr uint32_t tag = 68;
	static constexpr std::string_view name = "Alternate Platform Longitude";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-180.0, 180.0, codec::sentinel::reserve_min};
};

struct alternate_platform_altitude {
	static constexpr uint32_t tag = 69;
	static constexpr std::string_view name = "Alternate Platform Altitude";
	static constexpr std::string_view units = "m";
	static constexpr codec::mapped<uint16_t> format{-900.0, 19'000.0};
};

struct alternate_platform_name {
	static constexpr uint32_t tag = 70;
	static constexpr std::string_view name = "Alternate Platform Name";
	static constexpr codec::string format{.max_length = 127};
};

struct alternate_platform_heading {
	static constexpr uint32_t tag = 71;
	static constexpr std::string_view name = "Alternate Platform Heading";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<uint16_t> format{0.0, 360.0};
};

struct event_start_time_utc {
	static constexpr uint32_t tag = 72;
	static constexpr std::string_view name = "Event Start Time - UTC";
	static constexpr codec::timestamp format{};
};

struct sensor_ellipsoid_height {
	static constexpr uint32_t tag = 75;
	static constexpr std::string_view name = "Sensor Ellipsoid Height";
	static constexpr std::string_view units = "m";
	static constexpr codec::mapped<uint16_t> format{-900.0, 19'000.0};
};

struct alternate_platform_ellipsoid_height {
	static constexpr uint32_t tag = 76;
	static constexpr std::string_view name = "Alternate Platform Ellipsoid Height";
	static constexpr std::string_view units = "m";
	static constexpr codec::mapped<uint16_t> format{-900.0, 19'000.0};
};

struct operational_mode {
	static constexpr uint32_t tag = 77;
	static constexpr std::string_view name = "Operational Mode";
	static constexpr codec::enumeration<operation_type> format{.name_of = &to_string};
};

struct frame_center_height_above_ellipsoid {
	static constexpr uint32_t tag = 78;
	static constexpr std::string_view name = "Frame Center Height Above Ellipsoid";
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

struct corner_latitude_point_1_full {
	static constexpr uint32_t tag = 82;
	static constexpr std::string_view name = "Corner Latitude Point 1 (Full)";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-90.0, 90.0, codec::sentinel::reserve_min};
};

struct corner_longitude_point_1_full {
	static constexpr uint32_t tag = 83;
	static constexpr std::string_view name = "Corner Longitude Point 1 (Full)";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-180.0, 180.0, codec::sentinel::reserve_min};
};

struct corner_latitude_point_2_full {
	static constexpr uint32_t tag = 84;
	static constexpr std::string_view name = "Corner Latitude Point 2 (Full)";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-90.0, 90.0, codec::sentinel::reserve_min};
};

struct corner_longitude_point_2_full {
	static constexpr uint32_t tag = 85;
	static constexpr std::string_view name = "Corner Longitude Point 2 (Full)";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-180.0, 180.0, codec::sentinel::reserve_min};
};

struct corner_latitude_point_3_full {
	static constexpr uint32_t tag = 86;
	static constexpr std::string_view name = "Corner Latitude Point 3 (Full)";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-90.0, 90.0, codec::sentinel::reserve_min};
};

struct corner_longitude_point_3_full {
	static constexpr uint32_t tag = 87;
	static constexpr std::string_view name = "Corner Longitude Point 3 (Full)";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-180.0, 180.0, codec::sentinel::reserve_min};
};

struct corner_latitude_point_4_full {
	static constexpr uint32_t tag = 88;
	static constexpr std::string_view name = "Corner Latitude Point 4 (Full)";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-90.0, 90.0, codec::sentinel::reserve_min};
};

struct corner_longitude_point_4_full {
	static constexpr uint32_t tag = 89;
	static constexpr std::string_view name = "Corner Longitude Point 4 (Full)";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-180.0, 180.0, codec::sentinel::reserve_min};
};

struct platform_pitch_angle_full {
	static constexpr uint32_t tag = 90;
	static constexpr std::string_view name = "Platform Pitch Angle (Full)";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-90.0, 90.0, codec::sentinel::reserve_min};
};

struct platform_roll_angle_full {
	static constexpr uint32_t tag = 91;
	static constexpr std::string_view name = "Platform Roll Angle (Full)";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-90.0, 90.0, codec::sentinel::reserve_min};
};

struct platform_angle_of_attack_full {
	static constexpr uint32_t tag = 92;
	static constexpr std::string_view name = "Platform Angle of Attack (Full)";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-90.0, 90.0, codec::sentinel::reserve_min};
};

struct platform_sideslip_angle_full {
	static constexpr uint32_t tag = 93;
	static constexpr std::string_view name = "Platform Sideslip Angle (Full)";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-180.0, 180.0, codec::sentinel::reserve_min};
};

struct miis_core_identifier {
	static constexpr uint32_t tag = 94;
	static constexpr std::string_view name = "MIIS Core Identifier";
	static constexpr codec::bytes format{};
};

struct stream_designator {
	static constexpr uint32_t tag = 106;
	static constexpr std::string_view name = "Stream Designator";
	static constexpr codec::string format{.max_length = 127};
};

struct operational_base {
	static constexpr uint32_t tag = 107;
	static constexpr std::string_view name = "Operational Base";
	static constexpr codec::string format{.max_length = 127};
};

struct broadcast_source {
	static constexpr uint32_t tag = 108;
	static constexpr std::string_view name = "Broadcast Source";
	static constexpr codec::string format{.max_length = 127};
};

struct time_airborne {
	static constexpr uint32_t tag = 110;
	static constexpr std::string_view name = "Time Airborne";
	static constexpr std::string_view units = "s";
	static constexpr codec::integer<uint32_t> format{};
};

struct propulsion_unit_speed {
	static constexpr uint32_t tag = 111;
	static constexpr std::string_view name = "Propulsion Unit Speed";
	static constexpr std::string_view units = "RPM";
	static constexpr codec::integer<uint32_t> format{};
};

struct number_of_navsats_in_view {
	static constexpr uint32_t tag = 123;
	static constexpr std::string_view name = "Number of NAVSATs in View";
	static constexpr codec::integer<uint8_t> format{};
};

struct positioning_method_source {
	static constexpr uint32_t tag = 124;
	static constexpr std::string_view name = "Positioning Method Source";
	static constexpr codec::integer<uint8_t> format{};
};

struct platform_status {
	static constexpr uint32_t tag = 125;
	static constexpr std::string_view name = "Platform Status";
	static constexpr codec::enumeration<platform_state> format{.name_of = &to_string};
};

struct sensor_control_mode {
	static constexpr uint32_t tag = 126;
	static constexpr std::string_view name = "Sensor Control Mode";
	static constexpr codec::enumeration<control_mode> format{.name_of = &to_string};
};

struct target_id {
	static constexpr uint32_t tag = 129;
	static constexpr std::string_view name = "Target ID";
	static constexpr codec::string format{.max_length = 32};
};

struct take_off_time {
	static constexpr uint32_t tag = 131;
	static constexpr std::string_view name = "Take-off Time";
	static constexpr codec::timestamp format{};
};

struct onboard_mi_storage_capacity {
	static constexpr uint32_t tag = 133;
	static constexpr std::string_view name = "On-board MI Storage Capacity";
	static constexpr std::string_view units = "GB";
	static constexpr codec::integer<uint32_t> format{};
};

struct communications_method {
	static constexpr uint32_t tag = 135;
	static constexpr std::string_view name = "Communications Method";
	static constexpr codec::string format{.max_length = 127};
};

struct leap_seconds {
	static constexpr uint32_t tag = 136;
	static constexpr std::string_view name = "Leap Seconds";
	static constexpr std::string_view units = "s";
	static constexpr codec::integer<int32_t> format{};
};

struct correction_offset {
	static constexpr uint32_t tag = 137;
	static constexpr std::string_view name = "Correction Offset";
	static constexpr std::string_view units = "us";
	static constexpr codec::integer<int64_t> format{};
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
	    offset_corner_latitude_point_1,
	    offset_corner_longitude_point_1,
	    offset_corner_latitude_point_2,
	    offset_corner_longitude_point_2,
	    offset_corner_latitude_point_3,
	    offset_corner_longitude_point_3,
	    offset_corner_latitude_point_4,
	    offset_corner_longitude_point_4,
	    icing_detected,
	    wind_direction,
	    wind_speed,
	    static_pressure,
	    density_altitude,
	    outside_air_temperature,
	    target_location_latitude,
	    target_location_longitude,
	    target_location_elevation,
	    target_track_gate_width,
	    target_track_gate_height,
	    target_error_estimate_ce90,
	    target_error_estimate_le90,
	    generic_flag_data,
	    security_local_set,
	    differential_pressure,
	    platform_angle_of_attack,
	    platform_vertical_speed,
	    platform_sideslip_angle,
	    airfield_barometric_pressure,
	    airfield_elevation,
	    relative_humidity,
	    platform_ground_speed,
	    ground_range,
	    platform_fuel_remaining,
	    platform_call_sign,
	    weapon_load,
	    weapon_fired,
	    laser_prf_code,
	    sensor_field_of_view_name,
	    platform_magnetic_heading,
	    uas_datalink_ls_version_number,
	    alternate_platform_latitude,
	    alternate_platform_longitude,
	    alternate_platform_altitude,
	    alternate_platform_name,
	    alternate_platform_heading,
	    event_start_time_utc,
	    sensor_ellipsoid_height,
	    alternate_platform_ellipsoid_height,
	    operational_mode,
	    frame_center_height_above_ellipsoid,
	    sensor_north_velocity,
	    sensor_east_velocity,
	    corner_latitude_point_1_full,
	    corner_longitude_point_1_full,
	    corner_latitude_point_2_full,
	    corner_longitude_point_2_full,
	    corner_latitude_point_3_full,
	    corner_longitude_point_3_full,
	    corner_latitude_point_4_full,
	    corner_longitude_point_4_full,
	    platform_pitch_angle_full,
	    platform_roll_angle_full,
	    platform_angle_of_attack_full,
	    platform_sideslip_angle_full,
	    miis_core_identifier,
	    stream_designator,
	    operational_base,
	    broadcast_source,
	    time_airborne,
	    propulsion_unit_speed,
	    number_of_navsats_in_view,
	    positioning_method_source,
	    platform_status,
	    sensor_control_mode,
	    target_id,
	    take_off_time,
	    onboard_mi_storage_capacity,
	    communications_method,
	    leap_seconds,
	    correction_offset,
	    checksum>;
};

}  // namespace klv::misb0601
