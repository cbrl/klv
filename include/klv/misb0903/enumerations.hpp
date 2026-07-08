/**
 * @file klv/misb0903/enumerations.hpp
 * @brief MISB ST 0903 enumerations
 */

#pragma once

#include <cstdint>
#include <string_view>

namespace klv::misb0903 {

/// Status of the detections of one entity (VTarget LS tag 23, VTracker LS tag 2)
enum class detection_status : uint8_t {
	inactive = 0,
	active_moving = 1,
	dropped = 2,
	active_stopped = 3,
	active_coasting = 4,
};

[[nodiscard]]
constexpr auto to_string(detection_status value) -> std::string_view {
	using enum detection_status;
	switch (value) {
		case inactive:
			return "Inactive";
		case active_moving:
			return "Active-Moving";
		case dropped:
			return "Dropped";
		case active_stopped:
			return "Active-Stopped";
		case active_coasting:
			return "Active-Coasting";
	}
	return {};
}

}  // namespace klv::misb0903
