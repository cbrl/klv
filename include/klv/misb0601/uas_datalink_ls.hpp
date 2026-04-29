/**
 * @file klv/misb0601/uas_datalink_ls.hpp
 * @brief MISB ST 0601 UAS Datalink Local Set definition
 *
 * `uas_datalink_ls` lists the fields in encode order: Precision Time Stamp first, Checksum last.
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/misb0601/alternate_platform.hpp"
#include "klv/misb0601/environment.hpp"
#include "klv/misb0601/image.hpp"
#include "klv/misb0601/mission.hpp"
#include "klv/misb0601/platform.hpp"
#include "klv/misb0601/sensor.hpp"
#include "klv/misb0601/target.hpp"
#include "klv/set.hpp"

#include <string_view>

namespace klv::misb0601 {

/// MISB ST 0601 UAS Datalink Local Set
struct uas_datalink_ls {
	static constexpr std::string_view name = "UAS Datalink Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 01 01 00 00 00");

	using checksum_field = checksum;

	using fields = field_list<
	    precision_time_stamp,
	    mission_id,
	    platform_tail_number,
	    platform_heading_angle,
	    platform_pitch_angle,
	    platform_roll_angle,
	    platform_true_airspeed,
	    platform_indicated_airspeed,
	    platform_designation,
	    image_source_sensor,
	    image_coordinate_system,
	    sensor_latitude,
	    sensor_longitude,
	    sensor_true_altitude,
	    sensor_horizontal_field_of_view,
	    sensor_vertical_field_of_view,
	    sensor_relative_azimuth_angle,
	    sensor_relative_elevation_angle,
	    sensor_relative_roll_angle,
	    slant_range,
	    target_width,
	    frame_center_latitude,
	    frame_center_longitude,
	    frame_center_elevation,
	    offset_corner_latitude_point_1,
	    offset_corner_longitude_point_1,
	    offset_corner_latitude_point_2,
	    offset_corner_longitude_point_2,
	    offset_corner_latitude_point_3,
	    offset_corner_longitude_point_3,
	    offset_corner_latitude_point_4,
	    offset_corner_longitude_point_4,
	    icing_detected,
	    wind_direction,
	    wind_speed,
	    static_pressure,
	    density_altitude,
	    outside_air_temperature,
	    target_location_latitude,
	    target_location_longitude,
	    target_location_elevation,
	    target_track_gate_width,
	    target_track_gate_height,
	    target_error_estimate_ce90,
	    target_error_estimate_le90,
	    generic_flag_data,
	    security_local_set,
	    differential_pressure,
	    platform_angle_of_attack,
	    platform_vertical_speed,
	    platform_sideslip_angle,
	    airfield_barometric_pressure,
	    airfield_elevation,
	    relative_humidity,
	    platform_ground_speed,
	    ground_range,
	    platform_fuel_remaining,
	    platform_call_sign,
	    weapon_load,
	    weapon_fired,
	    laser_prf_code,
	    sensor_field_of_view_name,
	    platform_magnetic_heading,
	    uas_datalink_ls_version_number,
	    alternate_platform_latitude,
	    alternate_platform_longitude,
	    alternate_platform_altitude,
	    alternate_platform_name,
	    alternate_platform_heading,
	    event_start_time_utc,
	    sensor_ellipsoid_height,
	    alternate_platform_ellipsoid_height,
	    operational_mode,
	    frame_center_height_above_ellipsoid,
	    sensor_north_velocity,
	    sensor_east_velocity,
	    image_horizon_pixel_pack,
	    corner_latitude_point_1_full,
	    corner_longitude_point_1_full,
	    corner_latitude_point_2_full,
	    corner_longitude_point_2_full,
	    corner_latitude_point_3_full,
	    corner_longitude_point_3_full,
	    corner_latitude_point_4_full,
	    corner_longitude_point_4_full,
	    platform_pitch_angle_full,
	    platform_roll_angle_full,
	    platform_angle_of_attack_full,
	    platform_sideslip_angle_full,
	    miis_core_identifier,
	    target_width_extended,
	    density_altitude_extended,
	    sensor_ellipsoid_height_extended,
	    alternate_platform_ellipsoid_height_extended,
	    stream_designator,
	    operational_base,
	    broadcast_source,
	    range_to_recovery_location,
	    time_airborne,
	    propulsion_unit_speed,
	    platform_course_angle,
	    altitude_agl,
	    radar_altimeter,
	    control_command,
	    control_command_verification_list,
	    sensor_azimuth_rate,
	    sensor_elevation_rate,
	    sensor_roll_rate,
	    onboard_mi_storage_percent_full,
	    active_wavelength_list,
	    country_codes,
	    number_of_navsats_in_view,
	    positioning_method_source,
	    platform_status,
	    sensor_control_mode,
	    sensor_frame_rate_pack,
	    wavelengths_list,
	    target_id,
	    airbase_locations,
	    take_off_time,
	    transmission_frequency,
	    onboard_mi_storage_capacity,
	    zoom_percentage,
	    communications_method,
	    leap_seconds,
	    correction_offset,
	    payload_list,
	    active_payloads,
	    weapons_stores,
	    waypoint_list,
	    view_domain,
	    checksum>;
};

}  // namespace klv::misb0601
