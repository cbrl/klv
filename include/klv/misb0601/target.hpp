/**
 * @file klv/misb0601/target.hpp
 * @brief MISB ST 0601 fields: target location, tracking, and weapons
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/misb0601/packs/weapons_store.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0601 {

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

struct target_id {
	static constexpr uint32_t tag = 129;
	static constexpr std::string_view name = "Target ID";
	static constexpr codec::string format{.max_length = 32};
};

struct weapons_stores {
	static constexpr uint32_t tag = 140;
	static constexpr std::string_view name = "Weapons Stores";
	static constexpr codec::series<weapons_store_codec> format{};
};

}  // namespace klv::misb0601
