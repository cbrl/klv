/**
 * @file klv/misb0601/image.hpp
 * @brief MISB ST 0601 fields: image frame geometry (frame center, corners, horizon, and ranges), range images,
 * geo-registration, and composite imaging
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/misb0601/packs/horizon_pixels.hpp"
#include "klv/misb1002/range_image_ls.hpp"
#include "klv/misb1601.hpp"
#include "klv/misb1602.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0601 {

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

struct ground_range {
	static constexpr uint32_t tag = 57;
	static constexpr std::string_view name = "Ground Range";
	static constexpr std::string_view units = "m";
	static constexpr codec::mapped<uint32_t> format{0.0, 5'000'000.0};
};

struct frame_center_height_above_ellipsoid {
	static constexpr uint32_t tag = 78;
	static constexpr std::string_view name = "Frame Center Height Above Ellipsoid";
	static constexpr std::string_view units = "m";
	static constexpr codec::mapped<uint16_t> format{-900.0, 19'000.0};
};

struct image_horizon_pixel_pack {
	static constexpr uint32_t tag = 81;
	static constexpr std::string_view name = "Image Horizon Pixel Pack";
	static constexpr horizon_pixels_codec format{};
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

struct target_width_extended {
	static constexpr uint32_t tag = 96;
	static constexpr std::string_view name = "Target Width Extended";
	static constexpr std::string_view units = "m";
	static constexpr codec::imapb format{0.0, 1'500'000.0, 3};
};

struct range_image_local_set {
	static constexpr uint32_t tag = 97;
	static constexpr std::string_view name = "Range Image Local Set";
	static constexpr codec::nested<misb1002::range_image_ls> format{};
};

struct geo_registration_local_set {
	static constexpr uint32_t tag = 98;
	static constexpr std::string_view name = "Geo-Registration Local Set";
	static constexpr codec::nested<misb1601::geo_registration_ls> format{};
};

struct composite_imaging_local_set {
	static constexpr uint32_t tag = 99;
	static constexpr std::string_view name = "Composite Imaging Local Set";
	static constexpr codec::nested<misb1602::composite_imaging_ls> format{};
};

}  // namespace klv::misb0601
