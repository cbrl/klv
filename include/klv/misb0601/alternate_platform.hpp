/**
 * @file klv/misb0601/alternate_platform.hpp
 * @brief MISB ST 0601 fields: the alternate platform, which connects to the UAS over a direct datalink
 */

#pragma once

#include "klv/codec.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0601 {

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

struct alternate_platform_ellipsoid_height {
	static constexpr uint32_t tag = 76;
	static constexpr std::string_view name = "Alternate Platform Ellipsoid Height";
	static constexpr std::string_view units = "m";
	static constexpr codec::mapped<uint16_t> format{-900.0, 19'000.0};
};

struct alternate_platform_ellipsoid_height_extended {
	static constexpr uint32_t tag = 105;
	static constexpr std::string_view name = "Alternate Platform Ellipsoid Height Extended";
	static constexpr std::string_view units = "m";
	static constexpr codec::imapb format{-900.0, 40'000.0, 3};
};

}  // namespace klv::misb0601
