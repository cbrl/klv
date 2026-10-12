/**
 * @file klv/misb1107/sdcc.hpp
 * @brief MISB ST 1107 SDCC-FLP (tag 32) and the IMAPB range of each standard deviation
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/misb1107/fields.hpp"

#include <cstdint>
#include <optional>
#include <string_view>

namespace klv::misb1107 {

/**
 * @brief IMAPB range of the standard deviation of the item with `tag` (ST 1107 Table 1)
 *
 * Items 22 to 30 have no range. Their standard deviations need IEEE 754.
 */
[[nodiscard]]
constexpr auto sigma_range(uint32_t tag) -> std::optional<imapb_range> {
	switch (tag) {
		case sensor_ecef_position_x::tag:
		case sensor_ecef_position_y::tag:
		case sensor_ecef_position_z::tag:
		case boresight_offset_delta_x::tag:
		case boresight_offset_delta_y::tag:
		case boresight_offset_delta_z::tag:
		case slant_range::tag:
			return imapb_range{.min = 0.0, .max = 650.0};
		case sensor_ecef_velocity_x::tag:
		case sensor_ecef_velocity_y::tag:
		case sensor_ecef_velocity_z::tag:
			return imapb_range{.min = 0.0, .max = 70.0};
		case sensor_absolute_azimuth::tag:
		case sensor_absolute_pitch::tag:
		case sensor_absolute_roll::tag:
			return imapb_range{.min = 0.0, .max = 0.2};
		case sensor_absolute_azimuth_rate::tag:
		case sensor_absolute_pitch_rate::tag:
		case sensor_absolute_roll_rate::tag:
		case focal_plane_principal_point_offset_y::tag:
		case focal_plane_principal_point_offset_x::tag:
			return imapb_range{.min = 0.0, .max = 1.0};
		case boresight_delta_angle_1::tag:
		case boresight_delta_angle_2::tag:
		case boresight_delta_angle_3::tag:
			return imapb_range{.min = 0.0, .max = 2.0};
		case sensor_calibrated_focal_length::tag:
		case sensor_calibrated_focal_length_extended::tag:
			return imapb_range{.min = 0.0, .max = 350.0};
		default:
			return std::nullopt;
	}
}

/// Uncertainty of the items directly before it. The set can hold it more than once.
struct sdcc_flp {
	static constexpr uint32_t tag = 32;
	static constexpr std::string_view name = "SDCC-FLP";
	static constexpr codec::repeated<codec::sdcc_flp> format{.item = {.sigma_ranges = &sigma_range}};
};

}  // namespace klv::misb1107
