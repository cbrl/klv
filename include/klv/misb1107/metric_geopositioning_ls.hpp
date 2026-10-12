/**
 * @file klv/misb1107/metric_geopositioning_ls.hpp
 * @brief MISB ST 1107 Metric Geopositioning Local Set definition
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/misb1107/fields.hpp"
#include "klv/misb1107/sdcc.hpp"
#include "klv/set.hpp"

#include <string_view>

namespace klv::misb1107 {

/**
 * @brief MISB ST 1107 Metric Geopositioning Local Set
 *
 * Packet decoding verifies the CRC-16-CCITT checksum (tag 45), and packet encoding computes it. The encoder writes
 * the items that an SDCC-FLP covers directly before it.
 */
struct metric_geopositioning_ls {
	static constexpr std::string_view name = "Metric Geopositioning Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 03 22 00 00 00");

	using checksum_field = checksum;
	static constexpr auto checksum_function = crc16_ccitt;

	using fields = field_list<
	    sensor_ecef_position_x,
	    sensor_ecef_position_y,
	    sensor_ecef_position_z,
	    sensor_ecef_velocity_x,
	    sensor_ecef_velocity_y,
	    sensor_ecef_velocity_z,
	    sensor_absolute_azimuth,
	    sensor_absolute_pitch,
	    sensor_absolute_roll,
	    sensor_absolute_azimuth_rate,
	    sensor_absolute_pitch_rate,
	    sensor_absolute_roll_rate,
	    boresight_offset_delta_x,
	    boresight_offset_delta_y,
	    boresight_offset_delta_z,
	    boresight_delta_angle_1,
	    boresight_delta_angle_2,
	    boresight_delta_angle_3,
	    focal_plane_principal_point_offset_y,
	    focal_plane_principal_point_offset_x,
	    sensor_calibrated_focal_length,
	    radial_distortion_constant,
	    first_radial_distortion_parameter,
	    second_radial_distortion_parameter,
	    third_radial_distortion_parameter,
	    first_tangential_decentering_parameter,
	    second_tangential_decentering_parameter,
	    third_tangential_decentering_parameter,
	    differential_scale_affine_parameter,
	    skewness_affine_parameter,
	    slant_range,
	    sdcc_flp,
	    generalized_transformation_local_set,
	    image_rows,
	    image_columns,
	    pixel_size_x,
	    pixel_size_y,
	    slant_range_pedigree,
	    line_coordinate,
	    sample_coordinate,
	    lrf_divergence,
	    radial_distortion_valid_range,
	    precision_time_stamp,
	    document_version,
	    leap_seconds,
	    sensor_calibrated_focal_length_extended,
	    checksum>;
};

}  // namespace klv::misb1107
