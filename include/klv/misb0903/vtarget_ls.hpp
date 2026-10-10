/**
 * @file klv/misb0903/vtarget_ls.hpp
 * @brief MISB ST 0903 VTarget Local Set and VTarget pack (VMTI tag 101): one detected target
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/codecs/pack.hpp"
#include "klv/misb0903/enumerations.hpp"
#include "klv/misb0903/location.hpp"
#include "klv/misb0903/vchip_ls.hpp"
#include "klv/misb0903/vfeature_ls.hpp"
#include "klv/misb0903/vmask_ls.hpp"
#include "klv/misb0903/vobject_ls.hpp"
#include "klv/misb0903/vtracker_ls.hpp"
#include "klv/set.hpp"

#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <string_view>
#include <utility>

namespace klv::misb0903 {

/// Row and column of a focal plane array
struct fpa_index {
	uint8_t row = 0;
	uint8_t column = 0;

	friend auto operator==(fpa_index const&, fpa_index const&) -> bool = default;
};

/// FPA index pack: 1-byte row, then 1-byte column
struct fpa_index_codec {
	using value_type = fpa_index;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<fpa_index> {
		if (in.size() != 2) {
			return std::unexpected(field_error::bad_length);
		}
		return fpa_index{.row = std::to_integer<uint8_t>(in[0]), .column = std::to_integer<uint8_t>(in[1])};
	}

	[[nodiscard]]
	auto encode(fpa_index const& value, byte_writer& out) const -> encode_result {
		out.put_u8(value.row);
		out.put_u8(value.column);
		return {};
	}

	[[nodiscard]]
	auto to_string(fpa_index const& value) const -> std::string {
		return std::format("{{row: {}, column: {}}}", value.row, value.column);
	}
};

/// Pixel index: (row - 1) * frame width + column, with row and column from 1
struct target_centroid {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Target Centroid";
	static constexpr codec::integer<uint64_t> format{.length = codec::variable_length};
};

struct bounding_box_top_left_pixel {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Bounding Box Top Left Pixel";
	static constexpr codec::integer<uint64_t> format{.length = codec::variable_length};
};

struct bounding_box_bottom_right_pixel {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Bounding Box Bottom Right Pixel";
	static constexpr codec::integer<uint64_t> format{.length = codec::variable_length};
};

/// Lower values have a higher priority
struct target_priority {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Target Priority";
	static constexpr codec::integer<uint8_t> format{};
};

struct target_confidence_level {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "Target Confidence Level";
	static constexpr std::string_view units = "%";
	static constexpr codec::integer<uint8_t> format{};
};

/// Number of earlier detections of the target
struct target_history {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "Target History";
	static constexpr codec::integer<uint16_t> format{.length = codec::variable_length};
};

struct percentage_of_target_pixels {
	static constexpr uint32_t tag = 7;
	static constexpr std::string_view name = "Percentage of Target Pixels";
	static constexpr std::string_view units = "%";
	static constexpr codec::integer<uint8_t> format{};
};

/// Dominant color as three bytes: red, green, blue
struct target_color {
	static constexpr uint32_t tag = 8;
	static constexpr std::string_view name = "Target Color";
	static constexpr codec::integer<uint32_t> format{.length = 3};
};

/// Dominant intensity of up to 24 bits
struct target_intensity {
	static constexpr uint32_t tag = 9;
	static constexpr std::string_view name = "Target Intensity";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

/// Offset from the ST 0601 Frame Center Latitude
struct target_location_offset_latitude {
	static constexpr uint32_t tag = 10;
	static constexpr std::string_view name = "Target Location Offset Latitude";
	static constexpr std::string_view units = "deg";
	static constexpr codec::imapb format{-19.2, 19.2, 3};
};

/// Offset from the ST 0601 Frame Center Longitude
struct target_location_offset_longitude {
	static constexpr uint32_t tag = 11;
	static constexpr std::string_view name = "Target Location Offset Longitude";
	static constexpr std::string_view units = "deg";
	static constexpr codec::imapb format{-19.2, 19.2, 3};
};

struct target_height_above_ellipsoid {
	static constexpr uint32_t tag = 12;
	static constexpr std::string_view name = "Target Height Above Ellipsoid";
	static constexpr std::string_view units = "m";
	static constexpr codec::imapb format{-900.0, 19'000.0, 2};
};

struct bounding_box_top_left_latitude_offset {
	static constexpr uint32_t tag = 13;
	static constexpr std::string_view name = "Bounding Box Top Left Latitude Offset";
	static constexpr std::string_view units = "deg";
	static constexpr codec::imapb format{-19.2, 19.2, 3};
};

struct bounding_box_top_left_longitude_offset {
	static constexpr uint32_t tag = 14;
	static constexpr std::string_view name = "Bounding Box Top Left Longitude Offset";
	static constexpr std::string_view units = "deg";
	static constexpr codec::imapb format{-19.2, 19.2, 3};
};

struct bounding_box_bottom_right_latitude_offset {
	static constexpr uint32_t tag = 15;
	static constexpr std::string_view name = "Bounding Box Bottom Right Latitude Offset";
	static constexpr std::string_view units = "deg";
	static constexpr codec::imapb format{-19.2, 19.2, 3};
};

struct bounding_box_bottom_right_longitude_offset {
	static constexpr uint32_t tag = 16;
	static constexpr std::string_view name = "Bounding Box Bottom Right Longitude Offset";
	static constexpr std::string_view units = "deg";
	static constexpr codec::imapb format{-19.2, 19.2, 3};
};

struct target_location {
	static constexpr uint32_t tag = 17;
	static constexpr std::string_view name = "Target Location";
	static constexpr location_codec format{};
};

/// Vertices of the boundary around the target
struct geospatial_contour_series {
	static constexpr uint32_t tag = 18;
	static constexpr std::string_view name = "Geospatial Contour Series";
	static constexpr codec::series<location_codec> format{};
};

/// Row of the centroid pixel, from 1 at the top
struct centroid_pixel_row {
	static constexpr uint32_t tag = 19;
	static constexpr std::string_view name = "Centroid Pixel Row";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

/// Column of the centroid pixel, from 1 at the left
struct centroid_pixel_column {
	static constexpr uint32_t tag = 20;
	static constexpr std::string_view name = "Centroid Pixel Column";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

/// Deprecated
struct fpa_index_pack {
	static constexpr uint32_t tag = 21;
	static constexpr std::string_view name = "FPA Index Pack";
	static constexpr fpa_index_codec format{};
};

/// ID of an algorithm in the VMTI Algorithm Series
struct target_algorithm_id {
	static constexpr uint32_t tag = 22;
	static constexpr std::string_view name = "Algorithm ID";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

struct target_detection_status {
	static constexpr uint32_t tag = 23;
	static constexpr std::string_view name = "Detection Status";
	static constexpr codec::enumeration<detection_status> format{};
};

struct vmask_local_set {
	static constexpr uint32_t tag = 101;
	static constexpr std::string_view name = "VMask Local Set";
	static constexpr codec::nested<vmask_ls> format{};
};

/// Deprecated. Use the VObject Series.
struct vobject_local_set {
	static constexpr uint32_t tag = 102;
	static constexpr std::string_view name = "VObject Local Set";
	static constexpr codec::nested<vobject_ls> format{};
};

/// Deprecated
struct vfeature_local_set {
	static constexpr uint32_t tag = 103;
	static constexpr std::string_view name = "VFeature Local Set";
	static constexpr codec::nested<vfeature_ls> format{};
};

struct vtracker_local_set {
	static constexpr uint32_t tag = 104;
	static constexpr std::string_view name = "VTracker Local Set";
	static constexpr codec::nested<vtracker_ls> format{};
};

struct vchip_local_set {
	static constexpr uint32_t tag = 105;
	static constexpr std::string_view name = "VChip Local Set";
	static constexpr codec::nested<vchip_ls> format{};
};

struct vchip_series {
	static constexpr uint32_t tag = 106;
	static constexpr std::string_view name = "VChip Series";
	static constexpr codec::series<codec::nested<vchip_ls>> format{};
};

struct vobject_series {
	static constexpr uint32_t tag = 107;
	static constexpr std::string_view name = "VObject Series";
	static constexpr codec::series<codec::nested<vobject_ls>> format{};
};

/// MISB ST 0903 VTarget Local Set: the data of one target in a VTarget pack
struct vtarget_ls {
	static constexpr std::string_view name = "VTarget Local Set";

	using fields = field_list<
	    target_centroid,
	    bounding_box_top_left_pixel,
	    bounding_box_bottom_right_pixel,
	    target_priority,
	    target_confidence_level,
	    target_history,
	    percentage_of_target_pixels,
	    target_color,
	    target_intensity,
	    target_location_offset_latitude,
	    target_location_offset_longitude,
	    target_height_above_ellipsoid,
	    bounding_box_top_left_latitude_offset,
	    bounding_box_top_left_longitude_offset,
	    bounding_box_bottom_right_latitude_offset,
	    bounding_box_bottom_right_longitude_offset,
	    target_location,
	    geospatial_contour_series,
	    centroid_pixel_row,
	    centroid_pixel_column,
	    fpa_index_pack,
	    target_algorithm_id,
	    target_detection_status,
	    vmask_local_set,
	    vobject_local_set,
	    vfeature_local_set,
	    vtracker_local_set,
	    vchip_local_set,
	    vchip_series,
	    vobject_series>;
};

/// One target: its ID and its VTarget Local Set
struct vtarget {
	uint32_t id = 0;  ///< Target ID, unique in the frame
	record<vtarget_ls> set;

	friend auto operator==(vtarget const&, vtarget const&) -> bool = default;
};

/// VTarget pack: the target ID as a BER-OID, then the VTarget Local Set
struct vtarget_codec {
	using value_type = vtarget;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<vtarget> {
		auto reader = byte_reader{in};
		auto const id = klv::detail::read_oid(reader);
		if (!id) {
			return std::unexpected(id.error());
		}
		auto set = set_format.decode(reader.rest());
		if (!set) {
			return std::unexpected(set.error());
		}
		return vtarget{.id = *id, .set = std::move(*set)};
	}

	[[nodiscard]]
	auto encode(vtarget const& value, byte_writer& out) const -> encode_result {
		out.put_ber_oid(value.id);
		return set_format.encode(value.set, out);
	}

	[[nodiscard]]
	auto to_string(vtarget const& value) const -> std::string {
		auto const set = set_format.to_string(value.set);
		return set.empty() ? std::format("{{id: {}}}", value.id) : std::format("{{id: {}, {}}}", value.id, set);
	}

private:
	static constexpr codec::nested<vtarget_ls> set_format{};
};

}  // namespace klv::misb0903
