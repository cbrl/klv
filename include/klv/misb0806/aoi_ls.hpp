/**
 * @file klv/misb0806/aoi_ls.hpp
 * @brief MISB ST 0806 Area of Interest Local Set (RVT tag 13)
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/misb0806/enumerations.hpp"
#include "klv/set.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0806 {

struct aoi_number {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "AOI Number";
	static constexpr codec::integer<uint16_t> format{};
};

/// Northwest corner
struct aoi_corner_latitude_point_1 {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "AOI Corner Latitude Point 1";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-90.0, 90.0, codec::sentinel::reserve_min};
};

/// Northwest corner
struct aoi_corner_longitude_point_1 {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "AOI Corner Longitude Point 1";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-180.0, 180.0, codec::sentinel::reserve_min};
};

/// Southeast corner
struct aoi_corner_latitude_point_3 {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "AOI Corner Latitude Point 3";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-90.0, 90.0, codec::sentinel::reserve_min};
};

/// Southeast corner
struct aoi_corner_longitude_point_3 {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "AOI Corner Longitude Point 3";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int32_t> format{-180.0, 180.0, codec::sentinel::reserve_min};
};

struct aoi_type {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "AOI Type";
	static constexpr codec::enumeration<interest_type> format{.name_of = &to_string};
};

struct aoi_text {
	static constexpr uint32_t tag = 7;
	static constexpr std::string_view name = "AOI Text";
	static constexpr codec::string format{.max_length = 2048};
};

struct aoi_source_id {
	static constexpr uint32_t tag = 8;
	static constexpr std::string_view name = "AOI Source ID";
	static constexpr codec::string format{.max_length = 255};
};

struct aoi_label {
	static constexpr uint32_t tag = 9;
	static constexpr std::string_view name = "AOI Label";
	static constexpr codec::string format{.max_length = 16};
};

struct aoi_operation_id {
	static constexpr uint32_t tag = 10;
	static constexpr std::string_view name = "Operation ID";
	static constexpr codec::string format{.max_length = 127};
};

/// MISB ST 0806 Area of Interest Local Set
struct aoi_ls {
	static constexpr std::string_view name = "Area of Interest Local Set";

	using fields = field_list<
	    aoi_number,
	    aoi_corner_latitude_point_1,
	    aoi_corner_longitude_point_1,
	    aoi_corner_latitude_point_3,
	    aoi_corner_longitude_point_3,
	    aoi_type,
	    aoi_text,
	    aoi_source_id,
	    aoi_label,
	    aoi_operation_id>;
};

}  // namespace klv::misb0806
