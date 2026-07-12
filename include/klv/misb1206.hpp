/**
 * @file misb1206.hpp
 * @brief MISB ST 1206 - SAR Motion Imagery Local Set
 *
 * The set usually arrives nested in MISB ST 0601 tag 95 (see `misb0601::sar_motion_imagery_local_set`).
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/set.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb1206 {

/// Side of the platform that the radar looks to (tag 3)
enum class look_side : uint8_t {
	left = 0,
	right = 1,
};

[[nodiscard]]
constexpr auto to_string(look_side value) -> std::string_view {
	switch (value) {
		case look_side::left:
			return "Left";
		case look_side::right:
			return "Right";
	}
	return {};
}

/// Plane of the SAR image (tag 4)
enum class image_plane_type : uint8_t {
	slant = 0,
	ground = 1,
	other = 2,
};

[[nodiscard]]
constexpr auto to_string(image_plane_type value) -> std::string_view {
	switch (value) {
		case image_plane_type::slant:
			return "Slant";
		case image_plane_type::ground:
			return "Ground";
		case image_plane_type::other:
			return "Other";
	}
	return {};
}

struct grazing_angle {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Grazing Angle";
	static constexpr std::string_view units = "deg";
	static constexpr codec::imapb format{0.0, 90.0, 2};
};

struct ground_plane_squint_angle {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Ground Plane Squint Angle";
	static constexpr std::string_view units = "deg";
	static constexpr codec::imapb format{-90.0, 90.0, 2};
};

/// Side of the platform that the radar looks to, relative to the velocity
struct look_direction {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Look Direction";
	static constexpr codec::enumeration<look_side> format{.name_of = &to_string};
};

struct image_plane {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Image Plane";
	static constexpr codec::enumeration<image_plane_type> format{.name_of = &to_string};
};

struct range_resolution {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "Range Resolution";
	static constexpr std::string_view units = "m";
	static constexpr codec::imapb format{0.0, 1.0e6, 4};
};

struct cross_range_resolution {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "Cross-Range Resolution";
	static constexpr std::string_view units = "m";
	static constexpr codec::imapb format{0.0, 1.0e6, 4};
};

struct range_image_plane_pixel_size {
	static constexpr uint32_t tag = 7;
	static constexpr std::string_view name = "Range Image Plane Pixel Size";
	static constexpr std::string_view units = "m";
	static constexpr codec::imapb format{0.0, 1.0e6, 4};
};

struct cross_range_image_plane_pixel_size {
	static constexpr uint32_t tag = 8;
	static constexpr std::string_view name = "Cross-Range Image Plane Pixel Size";
	static constexpr std::string_view units = "m";
	static constexpr codec::imapb format{0.0, 1.0e6, 4};
};

struct image_rows {
	static constexpr uint32_t tag = 9;
	static constexpr std::string_view name = "Image Rows";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<uint16_t> format{};
};

struct image_columns {
	static constexpr uint32_t tag = 10;
	static constexpr std::string_view name = "Image Columns";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<uint16_t> format{};
};

/// Direction of the range vector, clockwise from true north
struct range_direction_angle {
	static constexpr uint32_t tag = 11;
	static constexpr std::string_view name = "Range Direction Angle";
	static constexpr std::string_view units = "deg";
	static constexpr codec::imapb format{0.0, 360.0, 2};
};

/// Direction of true north, clockwise from the top edge of the image
struct true_north {
	static constexpr uint32_t tag = 12;
	static constexpr std::string_view name = "True North";
	static constexpr std::string_view units = "deg";
	static constexpr codec::imapb format{0.0, 360.0, 2};
};

struct range_layover_angle {
	static constexpr uint32_t tag = 13;
	static constexpr std::string_view name = "Range Layover Angle";
	static constexpr std::string_view units = "deg";
	static constexpr codec::imapb format{0.0, 360.0, 2};
};

struct ground_aperture_angular_extent {
	static constexpr uint32_t tag = 14;
	static constexpr std::string_view name = "Ground Aperture Angular Extent";
	static constexpr std::string_view units = "deg";
	static constexpr codec::imapb format{0.0, 90.0, 2};
};

struct aperture_duration {
	static constexpr uint32_t tag = 15;
	static constexpr std::string_view name = "Aperture Duration";
	static constexpr std::string_view units = "us";
	static constexpr codec::integer<uint32_t> format{};
};

struct ground_track_angle {
	static constexpr uint32_t tag = 16;
	static constexpr std::string_view name = "Ground Track Angle";
	static constexpr std::string_view units = "deg";
	static constexpr codec::imapb format{0.0, 360.0, 2};
};

struct minimum_detectable_velocity {
	static constexpr uint32_t tag = 17;
	static constexpr std::string_view name = "Minimum Detectable Velocity";
	static constexpr std::string_view units = "m/s";
	static constexpr codec::imapb format{0.0, 100.0, 2};
};

struct true_pulse_repetition_frequency {
	static constexpr uint32_t tag = 18;
	static constexpr std::string_view name = "True Pulse Repetition Frequency";
	static constexpr std::string_view units = "Hz";
	static constexpr codec::imapb format{0.0, 1.0e6, 4};
};

struct pulse_repetition_frequency_scale_factor {
	static constexpr uint32_t tag = 19;
	static constexpr std::string_view name = "Pulse Repetition Frequency Scale Factor";
	static constexpr codec::imapb format{0.0, 1.0, 2};
};

struct transmit_rf_center_frequency {
	static constexpr uint32_t tag = 20;
	static constexpr std::string_view name = "Transmit RF Center Frequency";
	static constexpr std::string_view units = "Hz";
	static constexpr codec::imapb format{0.0, 1.0e12, 4};
};

struct transmit_rf_bandwidth {
	static constexpr uint32_t tag = 21;
	static constexpr std::string_view name = "Transmit RF Bandwidth";
	static constexpr std::string_view units = "Hz";
	static constexpr codec::imapb format{0.0, 1.0e11, 4};
};

/// Two-dimensional array of polynomial coefficients
struct radar_cross_section_scale_factor_polynomial {
	static constexpr uint32_t tag = 22;
	static constexpr std::string_view name = "Radar Cross Section Scale Factor Polynomial";
	static constexpr codec::mdarray<double> format{};
};

struct reference_frame_precision_time_stamp {
	static constexpr uint32_t tag = 23;
	static constexpr std::string_view name = "Reference Frame Precision Time Stamp";
	static constexpr codec::timestamp format{};
};

struct reference_frame_grazing_angle {
	static constexpr uint32_t tag = 24;
	static constexpr std::string_view name = "Reference Frame Grazing Angle";
	static constexpr std::string_view units = "deg";
	static constexpr codec::imapb format{0.0, 90.0, 2};
};

struct reference_frame_ground_plane_squint_angle {
	static constexpr uint32_t tag = 25;
	static constexpr std::string_view name = "Reference Frame Ground Plane Squint Angle";
	static constexpr std::string_view units = "deg";
	static constexpr codec::imapb format{-90.0, 90.0, 2};
};

struct reference_frame_range_direction_angle {
	static constexpr uint32_t tag = 26;
	static constexpr std::string_view name = "Reference Frame Range Direction Angle";
	static constexpr std::string_view units = "deg";
	static constexpr codec::imapb format{0.0, 360.0, 2};
};

struct reference_frame_range_layover_angle {
	static constexpr uint32_t tag = 27;
	static constexpr std::string_view name = "Reference Frame Range Layover Angle";
	static constexpr std::string_view units = "deg";
	static constexpr codec::imapb format{0.0, 360.0, 2};
};

struct document_version {
	static constexpr uint32_t tag = 28;
	static constexpr std::string_view name = "Document Version";
	static constexpr codec::integer<uint8_t> format{};
};

/// MISB ST 1206 SAR Motion Imagery Local Set
struct sar_motion_imagery_ls {
	static constexpr std::string_view name = "SAR Motion Imagery Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 03 0D 00 00 00");

	using fields = field_list<
	    grazing_angle,
	    ground_plane_squint_angle,
	    look_direction,
	    image_plane,
	    range_resolution,
	    cross_range_resolution,
	    range_image_plane_pixel_size,
	    cross_range_image_plane_pixel_size,
	    image_rows,
	    image_columns,
	    range_direction_angle,
	    true_north,
	    range_layover_angle,
	    ground_aperture_angular_extent,
	    aperture_duration,
	    ground_track_angle,
	    minimum_detectable_velocity,
	    true_pulse_repetition_frequency,
	    pulse_repetition_frequency_scale_factor,
	    transmit_rf_center_frequency,
	    transmit_rf_bandwidth,
	    radar_cross_section_scale_factor_polynomial,
	    reference_frame_precision_time_stamp,
	    reference_frame_grazing_angle,
	    reference_frame_ground_plane_squint_angle,
	    reference_frame_range_direction_angle,
	    reference_frame_range_layover_angle,
	    document_version>;
};

}  // namespace klv::misb1206
