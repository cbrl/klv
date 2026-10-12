/**
 * @file klv/misb1107/fields.hpp
 * @brief MISB ST 1107 Metric Geopositioning Local Set field descriptors, except the SDCC-FLP
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/misb1107/enumerations.hpp"
#include "klv/misb1202.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb1107 {

/// Float item of ST 1107: an IEEE 754 value of 2, 4, or 8 bytes
inline constexpr codec::ieee754_variable float_format{.binary16 = true};

// Sensor position, velocity, and absolute orientation (MISB ST 0801 sections 6.4.1 to 6.4.4)

/// Along the axis from the center of the Earth to the intersection of the equator and the prime meridian
struct sensor_ecef_position_x {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Sensor ECEF Position Component X";
	static constexpr std::string_view units = "m";
	static constexpr codec::imapb_value format{-1.0e9, 1.0e9};
};

/// Along the axis that completes a right-handed system with the X and Z axes
struct sensor_ecef_position_y {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Sensor ECEF Position Component Y";
	static constexpr std::string_view units = "m";
	static constexpr codec::imapb_value format{-1.0e9, 1.0e9};
};

/// Along the axis from the center of the Earth to the North Pole
struct sensor_ecef_position_z {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Sensor ECEF Position Component Z";
	static constexpr std::string_view units = "m";
	static constexpr codec::imapb_value format{-1.0e9, 1.0e9};
};

struct sensor_ecef_velocity_x {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Sensor ECEF Velocity Component X";
	static constexpr std::string_view units = "m/s";
	static constexpr codec::imapb_value format{-25.0e3, 25.0e3};
};

struct sensor_ecef_velocity_y {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "Sensor ECEF Velocity Component Y";
	static constexpr std::string_view units = "m/s";
	static constexpr codec::imapb_value format{-25.0e3, 25.0e3};
};

struct sensor_ecef_velocity_z {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "Sensor ECEF Velocity Component Z";
	static constexpr std::string_view units = "m/s";
	static constexpr codec::imapb_value format{-25.0e3, 25.0e3};
};

/// Angle from true north to the boresight in the local horizontal plane. North is 0 and east is 0.5.
struct sensor_absolute_azimuth {
	static constexpr uint32_t tag = 7;
	static constexpr std::string_view name = "Sensor Absolute Azimuth";
	static constexpr std::string_view units = "half-circles";
	static constexpr codec::imapb_value format{0.0, 2.0};
};

struct sensor_absolute_pitch {
	static constexpr uint32_t tag = 8;
	static constexpr std::string_view name = "Sensor Absolute Pitch";
	static constexpr std::string_view units = "half-circles";
	static constexpr codec::imapb_value format{-1.0, 1.0};
};

struct sensor_absolute_roll {
	static constexpr uint32_t tag = 9;
	static constexpr std::string_view name = "Sensor Absolute Roll";
	static constexpr std::string_view units = "half-circles";
	static constexpr codec::imapb_value format{-1.0, 1.0};
};

struct sensor_absolute_azimuth_rate {
	static constexpr uint32_t tag = 10;
	static constexpr std::string_view name = "Sensor Absolute Azimuth Rate";
	static constexpr std::string_view units = "half-circles/s";
	static constexpr codec::imapb_value format{-1.0, 1.0};
};

struct sensor_absolute_pitch_rate {
	static constexpr uint32_t tag = 11;
	static constexpr std::string_view name = "Sensor Absolute Pitch Rate";
	static constexpr std::string_view units = "half-circles/s";
	static constexpr codec::imapb_value format{-1.0, 1.0};
};

struct sensor_absolute_roll_rate {
	static constexpr uint32_t tag = 12;
	static constexpr std::string_view name = "Sensor Absolute Roll Rate";
	static constexpr std::string_view units = "half-circles/s";
	static constexpr codec::imapb_value format{-1.0, 1.0};
};

// Boresight (MISB ST 0801 section 6.5.2)

/// Translation from the sensor reference point to the perspective center
struct boresight_offset_delta_x {
	static constexpr uint32_t tag = 13;
	static constexpr std::string_view name = "Boresight Offset Delta X";
	static constexpr std::string_view units = "m";
	static constexpr codec::imapb_value format{-300.0, 300.0};
};

struct boresight_offset_delta_y {
	static constexpr uint32_t tag = 14;
	static constexpr std::string_view name = "Boresight Offset Delta Y";
	static constexpr std::string_view units = "m";
	static constexpr codec::imapb_value format{-300.0, 300.0};
};

struct boresight_offset_delta_z {
	static constexpr uint32_t tag = 15;
	static constexpr std::string_view name = "Boresight Offset Delta Z";
	static constexpr std::string_view units = "m";
	static constexpr codec::imapb_value format{-300.0, 300.0};
};

/// Rotation that aligns the sensor reference axes with the sensor principal axes
struct boresight_delta_angle_1 {
	static constexpr uint32_t tag = 16;
	static constexpr std::string_view name = "Boresight Delta Angle 1";
	static constexpr std::string_view units = "half-circles";
	static constexpr codec::imapb_value format{-0.25, 0.25};
};

struct boresight_delta_angle_2 {
	static constexpr uint32_t tag = 17;
	static constexpr std::string_view name = "Boresight Delta Angle 2";
	static constexpr std::string_view units = "half-circles";
	static constexpr codec::imapb_value format{-0.25, 0.25};
};

struct boresight_delta_angle_3 {
	static constexpr uint32_t tag = 18;
	static constexpr std::string_view name = "Boresight Delta Angle 3";
	static constexpr std::string_view units = "half-circles";
	static constexpr codec::imapb_value format{-0.25, 0.25};
};

// Interior orientation (MISB ST 0801 sections 6.5.4 to 6.5.7)

struct focal_plane_principal_point_offset_y {
	static constexpr uint32_t tag = 19;
	static constexpr std::string_view name = "Focal Plane Principal Point Offset Y";
	static constexpr std::string_view units = "mm";
	static constexpr codec::imapb_value format{-25.0, 25.0};
};

struct focal_plane_principal_point_offset_x {
	static constexpr uint32_t tag = 20;
	static constexpr std::string_view name = "Focal Plane Principal Point Offset X";
	static constexpr std::string_view units = "mm";
	static constexpr codec::imapb_value format{-25.0, 25.0};
};

/// A stream uses this item or item 47, not both. ST 1107 recommends item 47.
struct sensor_calibrated_focal_length {
	static constexpr uint32_t tag = 21;
	static constexpr std::string_view name = "Sensor Calibrated / Effective Focal Length";
	static constexpr std::string_view units = "mm";
	static constexpr codec::imapb_value format{0.0, 10'000.0};
};

/// k0 of the radial distortion equation
struct radial_distortion_constant {
	static constexpr uint32_t tag = 22;
	static constexpr std::string_view name = "Radial Distortion Constant Parameter";
	static constexpr std::string_view units = "mm/mm";
	static constexpr auto format = float_format;
};

/// k1 of the radial distortion equation
struct first_radial_distortion_parameter {
	static constexpr uint32_t tag = 23;
	static constexpr std::string_view name = "First Radial Distortion Parameter";
	static constexpr std::string_view units = "mm/mm^3";
	static constexpr auto format = float_format;
};

/// k2 of the radial distortion equation
struct second_radial_distortion_parameter {
	static constexpr uint32_t tag = 24;
	static constexpr std::string_view name = "Second Radial Distortion Parameter";
	static constexpr std::string_view units = "mm/mm^5";
	static constexpr auto format = float_format;
};

/// k3 of the radial distortion equation
struct third_radial_distortion_parameter {
	static constexpr uint32_t tag = 25;
	static constexpr std::string_view name = "Third Radial Distortion Parameter";
	static constexpr std::string_view units = "mm/mm^7";
	static constexpr auto format = float_format;
};

/// p1 of the tangential-decentering equation
struct first_tangential_decentering_parameter {
	static constexpr uint32_t tag = 26;
	static constexpr std::string_view name = "First Tangential / Decentering Parameter";
	static constexpr std::string_view units = "mm/mm^2";
	static constexpr auto format = float_format;
};

/// p2 of the tangential-decentering equation
struct second_tangential_decentering_parameter {
	static constexpr uint32_t tag = 27;
	static constexpr std::string_view name = "Second Tangential / Decentering Parameter";
	static constexpr std::string_view units = "mm/mm^2";
	static constexpr auto format = float_format;
};

/// p3 of the tangential-decentering equation
struct third_tangential_decentering_parameter {
	static constexpr uint32_t tag = 28;
	static constexpr std::string_view name = "Third Tangential / Decentering Parameter";
	static constexpr std::string_view units = "1/mm^2";
	static constexpr auto format = float_format;
};

/// b1 of the affine correction equation. It is exactly zero when the pixels are not square.
struct differential_scale_affine_parameter {
	static constexpr uint32_t tag = 29;
	static constexpr std::string_view name = "Differential Scale Affine Parameter";
	static constexpr std::string_view units = "mm/mm";
	static constexpr auto format = float_format;
};

/// b2 of the affine correction equation
struct skewness_affine_parameter {
	static constexpr uint32_t tag = 30;
	static constexpr std::string_view name = "Skewness Affine Parameter";
	static constexpr std::string_view units = "mm/mm";
	static constexpr auto format = float_format;
};

/// Distance from the perspective center to a point on the ground
struct slant_range {
	static constexpr uint32_t tag = 31;
	static constexpr std::string_view name = "Slant Range";
	static constexpr std::string_view units = "m";
	static constexpr auto format = float_format;
};

/// A stream uses this item or item 21, not both
struct sensor_calibrated_focal_length_extended {
	static constexpr uint32_t tag = 47;
	static constexpr std::string_view name = "Sensor Calibrated / Effective Focal Length Extended";
	static constexpr std::string_view units = "mm";
	static constexpr codec::imapb_value format{0.0, 100'000.0};
};

/// Relates the virtual image coordinates to the distorted image coordinates. The set can hold it up to four times.
struct generalized_transformation_local_set {
	static constexpr uint32_t tag = 33;
	static constexpr std::string_view name = "Generalized Transformation Local Set";
	static constexpr codec::repeated<codec::nested<misb1202::generalized_transformation_ls>> format{};
};

// Image size (MISB ST 0801 section 6.5.3)

struct image_rows {
	static constexpr uint32_t tag = 34;
	static constexpr std::string_view name = "Image Rows";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<uint16_t> format{};
};

struct image_columns {
	static constexpr uint32_t tag = 35;
	static constexpr std::string_view name = "Image Columns";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<uint16_t> format{};
};

struct pixel_size_x {
	static constexpr uint32_t tag = 36;
	static constexpr std::string_view name = "Pixel Size X";
	static constexpr std::string_view units = "mm";
	static constexpr codec::imapb_value format{1.0e-4, 0.1};
};

struct pixel_size_y {
	static constexpr uint32_t tag = 37;
	static constexpr std::string_view name = "Pixel Size Y";
	static constexpr std::string_view units = "mm";
	static constexpr codec::imapb_value format{1.0e-4, 0.1};
};

// Slant range (MISB ST 0801 section 6.6.1)

/// Without this item, a slant range is measured
struct slant_range_pedigree {
	static constexpr uint32_t tag = 38;
	static constexpr std::string_view name = "Slant Range Pedigree";
	static constexpr codec::enumeration<range_pedigree> format{};
};

/// Line of the slant range point in the image
struct line_coordinate {
	static constexpr uint32_t tag = 39;
	static constexpr std::string_view name = "Line Coordinate";
	static constexpr std::string_view units = "pixels";
	static constexpr auto format = float_format;
};

/// Sample of the slant range point in the image
struct sample_coordinate {
	static constexpr uint32_t tag = 40;
	static constexpr std::string_view name = "Sample Coordinate";
	static constexpr std::string_view units = "pixels";
	static constexpr auto format = float_format;
};

/// Divergence of the laser range finder that measures the slant range
struct lrf_divergence {
	static constexpr uint32_t tag = 41;
	static constexpr std::string_view name = "LRF Divergence";
	static constexpr std::string_view units = "rad";
	static constexpr auto format = float_format;
};

/// Radial distance from the principal point in which the radial distortion model is valid
struct radial_distortion_valid_range {
	static constexpr uint32_t tag = 42;
	static constexpr std::string_view name = "Valid Range of Radial Distortion";
	static constexpr std::string_view units = "mm";
	static constexpr auto format = float_format;
};

/// Time of the measurement of the items. It does not include leap seconds (see `leap_seconds`).
struct precision_time_stamp {
	static constexpr uint32_t tag = 43;
	static constexpr std::string_view name = "Precision Time Stamp";
	static constexpr codec::timestamp format{};
};

/// Version of ST 1107, for example 4 for ST 1107.4
struct document_version {
	static constexpr uint32_t tag = 44;
	static constexpr std::string_view name = "Document Version";
	static constexpr codec::ber_oid format{};
};

/// CRC-16-CCITT of the packet, from the key to the length of this item
struct checksum {
	static constexpr uint32_t tag = 45;
	static constexpr std::string_view name = "CRC-16-CCITT";
	static constexpr codec::integer<uint16_t> format{};
};

/// Add these seconds to the Precision Time Stamp to get UTC
struct leap_seconds {
	static constexpr uint32_t tag = 46;
	static constexpr std::string_view name = "Leap Seconds";
	static constexpr std::string_view units = "s";
	static constexpr codec::integer<int32_t> format{.length = codec::variable_length};
};

}  // namespace klv::misb1107
