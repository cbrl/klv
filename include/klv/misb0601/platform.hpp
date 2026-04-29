/**
 * @file klv/misb0601/platform.hpp
 * @brief MISB ST 0601 fields: platform attitude, motion, navigation, and flight state
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/misb0601/enumerations.hpp"
#include "klv/misb0601/packs/airbases.hpp"
#include "klv/misb0601/packs/waypoint_record.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0601 {

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

struct platform_ground_speed {
	static constexpr uint32_t tag = 56;
	static constexpr std::string_view name = "Platform Ground Speed";
	static constexpr std::string_view units = "m/s";
	static constexpr codec::integer<uint8_t> format{};
};

struct platform_fuel_remaining {
	static constexpr uint32_t tag = 58;
	static constexpr std::string_view name = "Platform Fuel Remaining";
	static constexpr std::string_view units = "kg";
	static constexpr codec::mapped<uint16_t> format{0.0, 10'000.0};
};

struct platform_magnetic_heading {
	static constexpr uint32_t tag = 64;
	static constexpr std::string_view name = "Platform Magnetic Heading";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<uint16_t> format{0.0, 360.0};
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

struct range_to_recovery_location {
	static constexpr uint32_t tag = 109;
	static constexpr std::string_view name = "Range To Recovery Location";
	static constexpr std::string_view units = "km";
	static constexpr codec::imapb format{0.0, 21'000.0, 4};
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

struct platform_course_angle {
	static constexpr uint32_t tag = 112;
	static constexpr std::string_view name = "Platform Course Angle";
	static constexpr std::string_view units = "deg";
	static constexpr codec::imapb format{0.0, 360.0, 2};
};

struct altitude_agl {
	static constexpr uint32_t tag = 113;
	static constexpr std::string_view name = "Altitude AGL";
	static constexpr std::string_view units = "m";
	static constexpr codec::imapb format{-900.0, 40'000.0, 3};
};

struct radar_altimeter {
	static constexpr uint32_t tag = 114;
	static constexpr std::string_view name = "Radar Altimeter";
	static constexpr std::string_view units = "m";
	static constexpr codec::imapb format{-900.0, 40'000.0, 3};
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

struct airbase_locations {
	static constexpr uint32_t tag = 130;
	static constexpr std::string_view name = "Airbase Locations";
	static constexpr airbases_codec format{};
};

struct waypoint_list {
	static constexpr uint32_t tag = 141;
	static constexpr std::string_view name = "Waypoint List";
	static constexpr codec::series<waypoint_record_codec> format{};
};

}  // namespace klv::misb0601
