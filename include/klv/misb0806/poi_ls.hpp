/**
 * @file klv/misb0806/poi_ls.hpp
 * @brief MISB ST 0806 Point of Interest Local Set (RVT tag 12)
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/misb0806/enumerations.hpp"
#include "klv/set.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0806 {

struct poi_number {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "POI Number";
	static constexpr codec::integer<uint16_t> format{};
};

struct poi_latitude {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "POI Latitude";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-90.0, 90.0, codec::sentinel::reserve_min};
};

struct poi_longitude {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "POI Longitude";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-180.0, 180.0, codec::sentinel::reserve_min};
};

struct poi_altitude {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "POI Altitude";
	static constexpr std::string_view units = "m";
	static constexpr codec::mapped<uint16_t> format{-900.0, 19'000.0};
};

struct poi_type {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "POI Type";
	static constexpr codec::enumeration<interest_type> format{};
};

struct poi_text {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "POI Text";
	static constexpr codec::string format{.max_length = 2048};
};

struct poi_source_icon {
	static constexpr uint32_t tag = 7;
	static constexpr std::string_view name = "POI Source Icon";
	static constexpr codec::string format{.max_length = 127};
};

struct poi_source_id {
	static constexpr uint32_t tag = 8;
	static constexpr std::string_view name = "POI Source ID";
	static constexpr codec::string format{.max_length = 255};
};

struct poi_label {
	static constexpr uint32_t tag = 9;
	static constexpr std::string_view name = "POI Label";
	static constexpr codec::string format{.max_length = 16};
};

struct poi_operation_id {
	static constexpr uint32_t tag = 10;
	static constexpr std::string_view name = "Operation ID";
	static constexpr codec::string format{.max_length = 127};
};

/// MISB ST 0806 Point of Interest Local Set
struct poi_ls {
	static constexpr std::string_view name = "Point of Interest Local Set";

	using fields = field_list<
	    poi_number,
	    poi_latitude,
	    poi_longitude,
	    poi_altitude,
	    poi_type,
	    poi_text,
	    poi_source_icon,
	    poi_source_id,
	    poi_label,
	    poi_operation_id>;
};

}  // namespace klv::misb0806
