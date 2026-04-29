/**
 * @file klv/misb0601/enumerations.hpp
 * @brief MISB ST 0601 enumerations of the fields that are not packs
 */

#pragma once

#include <cstdint>
#include <string_view>

namespace klv::misb0601 {

/// Icing detector state (tag 34)
enum class icing_state : uint8_t {
	detector_off = 0,
	no_icing_detected = 1,
	icing_detected = 2,
};

[[nodiscard]]
constexpr auto to_string(icing_state value) -> std::string_view {
	using enum icing_state;
	switch (value) {
		case detector_off:
			return "Detector Off";
		case no_icing_detected:
			return "No Icing Detected";
		case icing_detected:
			return "Icing Detected";
	}
	return {};
}

/// Sensor field of view name (tag 63)
enum class field_of_view_name : uint8_t {
	ultranarrow = 0,
	narrow = 1,
	medium = 2,
	wide = 3,
	ultrawide = 4,
	narrow_medium = 5,
	ultranarrow_2x = 6,
	ultranarrow_4x = 7,
	continuous_zoom = 8,
};

[[nodiscard]]
constexpr auto to_string(field_of_view_name value) -> std::string_view {
	using enum field_of_view_name;
	switch (value) {
		case ultranarrow:
			return "Ultranarrow";
		case narrow:
			return "Narrow";
		case medium:
			return "Medium";
		case wide:
			return "Wide";
		case ultrawide:
			return "Ultrawide";
		case narrow_medium:
			return "Narrow Medium";
		case ultranarrow_2x:
			return "2x Ultranarrow";
		case ultranarrow_4x:
			return "4x Ultranarrow";
		case continuous_zoom:
			return "Continuous Zoom";
	}
	return {};
}

/// Operational mode of the platform (tag 77)
enum class operation_type : uint8_t {
	other = 0,
	operational = 1,
	training = 2,
	exercise = 3,
	maintenance = 4,
	test = 5,
};

[[nodiscard]]
constexpr auto to_string(operation_type value) -> std::string_view {
	using enum operation_type;
	switch (value) {
		case other:
			return "Other";
		case operational:
			return "Operational";
		case training:
			return "Training";
		case exercise:
			return "Exercise";
		case maintenance:
			return "Maintenance";
		case test:
			return "Test";
	}
	return {};
}

/// Operational state of the platform (tag 125)
enum class platform_state : uint8_t {
	active = 0,
	preflight = 1,
	preflight_taxiing = 2,
	run_up = 3,
	take_off = 4,
	ingress = 5,
	manual_operation = 6,
	automated_orbit = 7,
	transitioning = 8,
	egress = 9,
	landing = 10,
	landed_taxiing = 11,
	landed_parked = 12,
};

[[nodiscard]]
constexpr auto to_string(platform_state value) -> std::string_view {
	using enum platform_state;
	switch (value) {
		case active:
			return "Active";
		case preflight:
			return "Pre-flight";
		case preflight_taxiing:
			return "Pre-flight-taxiing";
		case run_up:
			return "Run-up";
		case take_off:
			return "Take-off";
		case ingress:
			return "Ingress";
		case manual_operation:
			return "Manual operation";
		case automated_orbit:
			return "Automated-orbit";
		case transitioning:
			return "Transitioning";
		case egress:
			return "Egress";
		case landing:
			return "Landing";
		case landed_taxiing:
			return "Landed-taxiing";
		case landed_parked:
			return "Landed-Parked";
	}
	return {};
}

/// Control mode of the sensor (tag 126)
enum class control_mode : uint8_t {
	off = 0,
	home_position = 1,
	uncontrolled = 2,
	manual_control = 3,
	calibrating = 4,
	auto_holding_position = 5,
	auto_tracking = 6,
};

[[nodiscard]]
constexpr auto to_string(control_mode value) -> std::string_view {
	using enum control_mode;
	switch (value) {
		case off:
			return "Off";
		case home_position:
			return "Home Position";
		case uncontrolled:
			return "Uncontrolled";
		case manual_control:
			return "Manual Control";
		case calibrating:
			return "Calibrating";
		case auto_holding_position:
			return "Auto - Holding Position";
		case auto_tracking:
			return "Auto - Tracking";
	}
	return {};
}

}  // namespace klv::misb0601
