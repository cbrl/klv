/**
 * @file misb1602.hpp
 * @brief MISB ST 1602 - Composite Imaging Local Set
 *
 * The set usually arrives nested in MISB ST 0601 tag 99 (see `misb0601::composite_imaging_local_set`). It places one
 * source image in a composite image.
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/set.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb1602 {

struct precision_time_stamp {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Precision Time Stamp";
	static constexpr codec::timestamp format{};
};

struct document_version {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Document Version";
	static constexpr codec::ber_oid format{};
};

struct source_image_rows {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Source Image Rows";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

struct source_image_columns {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Source Image Columns";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

struct source_image_aoi_rows {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "Source Image AOI Rows";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

struct source_image_aoi_columns {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "Source Image AOI Columns";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

/// Position of the area of interest. The origin is the top left corner.
struct source_image_aoi_position_x {
	static constexpr uint32_t tag = 7;
	static constexpr std::string_view name = "Source Image AOI Position X";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<int32_t> format{.length = codec::variable_length};
};

struct source_image_aoi_position_y {
	static constexpr uint32_t tag = 8;
	static constexpr std::string_view name = "Source Image AOI Position Y";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<int32_t> format{.length = codec::variable_length};
};

struct sub_image_rows {
	static constexpr uint32_t tag = 9;
	static constexpr std::string_view name = "Sub-Image Rows";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

struct sub_image_columns {
	static constexpr uint32_t tag = 10;
	static constexpr std::string_view name = "Sub-Image Columns";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

/// Position of the sub-image. The origin is the top left corner.
struct sub_image_position_x {
	static constexpr uint32_t tag = 11;
	static constexpr std::string_view name = "Sub-Image Position X";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<int32_t> format{.length = codec::variable_length};
};

struct sub_image_position_y {
	static constexpr uint32_t tag = 12;
	static constexpr std::string_view name = "Sub-Image Position Y";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<int32_t> format{.length = codec::variable_length};
};

struct active_sub_image_rows {
	static constexpr uint32_t tag = 13;
	static constexpr std::string_view name = "Active Sub-Image Rows";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

struct active_sub_image_columns {
	static constexpr uint32_t tag = 14;
	static constexpr std::string_view name = "Active Sub-Image Columns";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

/// Offset of the active sub-image, from the top left corner of the sub-image
struct active_sub_image_offset_x {
	static constexpr uint32_t tag = 15;
	static constexpr std::string_view name = "Active Sub-Image Offset X";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<int32_t> format{.length = codec::variable_length};
};

struct active_sub_image_offset_y {
	static constexpr uint32_t tag = 16;
	static constexpr std::string_view name = "Active Sub-Image Offset Y";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<int32_t> format{.length = codec::variable_length};
};

/// Transparency: 0 is fully opaque, and 255 is fully transparent.
struct transparency {
	static constexpr uint32_t tag = 17;
	static constexpr std::string_view name = "Transparency";
	static constexpr codec::integer<uint8_t> format{};
};

/// Unique position of the image along the z axis. 0 is the bottom image.
struct z_order {
	static constexpr uint32_t tag = 18;
	static constexpr std::string_view name = "Z-Order";
	static constexpr codec::integer<uint8_t> format{};
};

/// MISB ST 1602 Composite Imaging Local Set
struct composite_imaging_ls {
	static constexpr std::string_view name = "Composite Imaging Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 03 02 00 00 00");

	using fields = field_list<
	    precision_time_stamp,
	    document_version,
	    source_image_rows,
	    source_image_columns,
	    source_image_aoi_rows,
	    source_image_aoi_columns,
	    source_image_aoi_position_x,
	    source_image_aoi_position_y,
	    sub_image_rows,
	    sub_image_columns,
	    sub_image_position_x,
	    sub_image_position_y,
	    active_sub_image_rows,
	    active_sub_image_columns,
	    active_sub_image_offset_x,
	    active_sub_image_offset_y,
	    transparency,
	    z_order>;
};

}  // namespace klv::misb1602
