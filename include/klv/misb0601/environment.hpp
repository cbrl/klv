/**
 * @file klv/misb0601/environment.hpp
 * @brief MISB ST 0601 fields: weather and air data
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/misb0601/enumerations.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0601 {

struct icing_detected {
	static constexpr uint32_t tag = 34;
	static constexpr std::string_view name = "Icing Detected";
	static constexpr codec::enumeration<icing_state> format{};
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
	static constexpr codec::mapped<int8_t> format{-127.0, 127.0, codec::sentinel::reserve_min};
};

struct differential_pressure {
	static constexpr uint32_t tag = 49;
	static constexpr std::string_view name = "Differential Pressure";
	static constexpr std::string_view units = "mbar";
	static constexpr codec::mapped<uint16_t> format{0.0, 5'000.0};
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

struct density_altitude_extended {
	static constexpr uint32_t tag = 103;
	static constexpr std::string_view name = "Density Altitude Extended";
	static constexpr std::string_view units = "m";
	static constexpr codec::imapb format{-900.0, 40'000.0, 3};
};

}  // namespace klv::misb0601
