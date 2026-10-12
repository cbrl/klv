/**
 * @file klv/misb1108/enumerations.hpp
 * @brief MISB ST 1108 enumerations
 */

#pragma once

#include <cstdint>
#include <string_view>

namespace klv::misb1108 {

/// Location in the imaging workflow where the metrics are calculated (tag 1, ST 1108 Table 2)
enum class assessment_location : uint8_t {
	sensor = 1,
	sensor_encoder = 2,
	gcs_received = 3,
	gcs_transmit = 4,
	library_archive = 5,
};

[[nodiscard]]
constexpr auto to_string(assessment_location value) -> std::string_view {
	using enum assessment_location;
	switch (value) {
		case sensor:
			return "Sensor";
		case sensor_encoder:
			return "Sensor Encoder";
		case gcs_received:
			return "GCS Received";
		case gcs_transmit:
			return "GCS Transmit";
		case library_archive:
			return "Library/Archive";
	}
	return {};
}

/// Compression of the stream (tag 5)
enum class compression_method : uint8_t {
	uncompressed = 0,
	h262 = 1,
	h264 = 2,
	h265 = 3,
	jpeg2000 = 4,
};

[[nodiscard]]
constexpr auto to_string(compression_method value) -> std::string_view {
	using enum compression_method;
	switch (value) {
		case uncompressed:
			return "Uncompressed";
		case h262:
			return "H.262";
		case h264:
			return "H.264";
		case h265:
			return "H.265";
		case jpeg2000:
			return "JPEG2000";
	}
	return {};
}

/// Profile of the compression (tag 6)
enum class compression_profile_kind : uint8_t {
	not_applicable = 0,
	main = 1,
	main_10 = 2,
	constrained_baseline = 3,
	high = 4,
	main_422_12 = 5,
	main_444_12 = 6,
	high_422 = 7,
	high_444_predictive = 8,
};

[[nodiscard]]
constexpr auto to_string(compression_profile_kind value) -> std::string_view {
	using enum compression_profile_kind;
	switch (value) {
		case not_applicable:
			return "N/A (uncompressed)";
		case main:
			return "Main (H.264)";
		case main_10:
			return "Main 10 (H.265)";
		case constrained_baseline:
			return "Constrained Baseline (H.264)";
		case high:
			return "High (H.264)";
		case main_422_12:
			return "Main 4:2:2 12 (H.265)";
		case main_444_12:
			return "Main 4:4:4 12 (H.265)";
		case high_422:
			return "High 4:2:2 (H.264)";
		case high_444_predictive:
			return "High 4:4:4 Predictive (H.264)";
	}
	return {};
}

}  // namespace klv::misb1108
