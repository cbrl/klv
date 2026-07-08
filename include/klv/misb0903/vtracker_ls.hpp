/**
 * @file klv/misb0903/vtracker_ls.hpp
 * @brief MISB ST 0903 VTracker Local Set (VTarget tag 104): the track of a target
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/misb0903/enumerations.hpp"
#include "klv/misb0903/location.hpp"
#include "klv/set.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0903 {

struct track_id {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Track ID";
	static constexpr codec::uuid format{};
};

/// Deprecated. Use the Detection Status of the VTarget LS.
struct tracker_detection_status {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Detection Status";
	static constexpr codec::enumeration<detection_status> format{.name_of = &to_string};
};

struct first_observation_time {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "First Observation Time";
	static constexpr codec::timestamp format{};
};

struct latest_observation_time {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Latest Observation Time";
	static constexpr codec::timestamp format{};
};

/// Vertices of the area or volume around all detections of the track
struct track_boundary_series {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "Track Boundary Series";
	static constexpr codec::series<location_codec> format{};
};

/// Deprecated. Use the Algorithm ID.
struct tracker_algorithm {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "Algorithm";
	static constexpr codec::string format{};
};

/// Larger values are more certain
struct tracker_confidence_level {
	static constexpr uint32_t tag = 7;
	static constexpr std::string_view name = "Confidence Level";
	static constexpr codec::integer<uint8_t> format{};
};

/// Deprecated
struct number_of_track_points {
	static constexpr uint32_t tag = 8;
	static constexpr std::string_view name = "Number of Track Points";
	static constexpr codec::integer<uint16_t> format{.length = codec::variable_length};
};

/// Locations of the detections of the track
struct track_history_series {
	static constexpr uint32_t tag = 9;
	static constexpr std::string_view name = "Track History Series";
	static constexpr codec::series<location_codec> format{};
};

struct tracker_velocity {
	static constexpr uint32_t tag = 10;
	static constexpr std::string_view name = "Velocity";
	static constexpr std::string_view units = "m/s";
	static constexpr motion_codec format{};
};

struct tracker_acceleration {
	static constexpr uint32_t tag = 11;
	static constexpr std::string_view name = "Acceleration";
	static constexpr std::string_view units = "m/s^2";
	static constexpr motion_codec format{};
};

/// ID of an algorithm in the VMTI Algorithm Series
struct tracker_algorithm_id {
	static constexpr uint32_t tag = 12;
	static constexpr std::string_view name = "Algorithm ID";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

/// MISB ST 0903 VTracker Local Set
struct vtracker_ls {
	static constexpr std::string_view name = "VTracker Local Set";

	using fields = field_list<
	    track_id,
	    tracker_detection_status,
	    first_observation_time,
	    latest_observation_time,
	    track_boundary_series,
	    tracker_algorithm,
	    tracker_confidence_level,
	    number_of_track_points,
	    track_history_series,
	    tracker_velocity,
	    tracker_acceleration,
	    tracker_algorithm_id>;
};

}  // namespace klv::misb0903
