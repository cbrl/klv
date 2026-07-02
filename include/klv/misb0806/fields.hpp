/**
 * @file klv/misb0806/fields.hpp
 * @brief MISB ST 0806 Remote Video Terminal Local Set field descriptors
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/misb0806/aoi_ls.hpp"
#include "klv/misb0806/poi_ls.hpp"
#include "klv/misb0806/user_defined_ls.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0806 {

/// CRC-32 (MPEG-2) of a stand-alone packet. Packet decoding and encoding treat it as a plain field.
struct checksum {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Checksum";
	static constexpr codec::integer<uint32_t> format{};
};

struct precision_time_stamp {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Precision Time Stamp";
	static constexpr codec::timestamp format{};
};

struct platform_true_airspeed {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Platform True Airspeed";
	static constexpr std::string_view units = "m/s";
	static constexpr codec::integer<uint16_t> format{};
};

struct platform_indicated_airspeed {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Platform Indicated Airspeed";
	static constexpr std::string_view units = "m/s";
	static constexpr codec::integer<uint16_t> format{};
};

/// Reserved for future use
struct telemetry_accuracy_indicator {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "Telemetry Accuracy Indicator";
	static constexpr codec::bytes format{};
};

struct frag_circle_radius {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "Frag Circle Radius";
	static constexpr std::string_view units = "m";
	static constexpr codec::integer<uint16_t> format{};
};

/// Counter at 60 Hz
struct frame_code {
	static constexpr uint32_t tag = 7;
	static constexpr std::string_view name = "Frame Code";
	static constexpr codec::integer<uint32_t> format{};
};

struct rvt_ls_version_number {
	static constexpr uint32_t tag = 8;
	static constexpr std::string_view name = "RVT LS Version Number";
	static constexpr codec::integer<uint8_t> format{};
};

/// Bits per second for digital video, or hertz for analog FM
struct video_data_rate {
	static constexpr uint32_t tag = 9;
	static constexpr std::string_view name = "Video Data Rate";
	static constexpr codec::integer<uint32_t> format{};
};

struct digital_video_file_format {
	static constexpr uint32_t tag = 10;
	static constexpr std::string_view name = "Digital Video File Format";
	static constexpr codec::string format{.max_length = 127};
};

struct user_defined_local_set {
	static constexpr uint32_t tag = 11;
	static constexpr std::string_view name = "User Defined Local Set";
	static constexpr codec::repeated<codec::nested<user_defined_ls>> format{};
};

struct poi_local_set {
	static constexpr uint32_t tag = 12;
	static constexpr std::string_view name = "POI Local Set";
	static constexpr codec::repeated<codec::nested<poi_ls>> format{};
};

struct aoi_local_set {
	static constexpr uint32_t tag = 13;
	static constexpr std::string_view name = "AOI Local Set";
	static constexpr codec::repeated<codec::nested<aoi_ls>> format{};
};

/// UTM zone 1 to 60
struct mgrs_zone {
	static constexpr uint32_t tag = 14;
	static constexpr std::string_view name = "MGRS Zone";
	static constexpr codec::integer<uint8_t> format{};
};

/// Latitude band letter, then the two letters of the grid square
struct mgrs_latitude_band_and_grid_square {
	static constexpr uint32_t tag = 15;
	static constexpr std::string_view name = "MGRS Latitude Band and Grid Square";
	static constexpr codec::string format{.max_length = 3};
};

struct mgrs_easting {
	static constexpr uint32_t tag = 16;
	static constexpr std::string_view name = "MGRS Easting";
	static constexpr std::string_view units = "m";
	static constexpr codec::integer<uint32_t> format{.length = 3};
};

struct mgrs_northing {
	static constexpr uint32_t tag = 17;
	static constexpr std::string_view name = "MGRS Northing";
	static constexpr std::string_view units = "m";
	static constexpr codec::integer<uint32_t> format{.length = 3};
};

struct frame_center_mgrs_zone {
	static constexpr uint32_t tag = 18;
	static constexpr std::string_view name = "Frame Center MGRS Zone";
	static constexpr codec::integer<uint8_t> format{};
};

struct frame_center_mgrs_latitude_band_and_grid_square {
	static constexpr uint32_t tag = 19;
	static constexpr std::string_view name = "Frame Center MGRS Latitude Band and Grid Square";
	static constexpr codec::string format{.max_length = 3};
};

struct frame_center_mgrs_easting {
	static constexpr uint32_t tag = 20;
	static constexpr std::string_view name = "Frame Center MGRS Easting";
	static constexpr std::string_view units = "m";
	static constexpr codec::integer<uint32_t> format{.length = 3};
};

struct frame_center_mgrs_northing {
	static constexpr uint32_t tag = 21;
	static constexpr std::string_view name = "Frame Center MGRS Northing";
	static constexpr std::string_view units = "m";
	static constexpr codec::integer<uint32_t> format{.length = 3};
};

}  // namespace klv::misb0806
