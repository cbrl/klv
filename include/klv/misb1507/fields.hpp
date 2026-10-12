/**
 * @file klv/misb1507/fields.hpp
 * @brief MISB ST 1507 Sensor Timing Local Set field descriptors
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/misb1507/detector.hpp"
#include "klv/misb1507/region_ls.hpp"
#include "klv/misb1603/nano_time_transfer.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb1507 {

/// The time stamp that each exposure offset counts from, and its Time Transfer Local Set. The time is at or before
/// the first Exposure Start Time of the Detector Group.
struct reference_time_pack {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Reference Time Pack";
	static constexpr misb1603::nano_time_transfer_codec format{};
};

/// Without this item, the window is the full Detector Group
struct window_corners_pack {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Window Corners Pack";
	static constexpr window_codec format{};
};

/// One item for each Region of the Detector Group
struct region_list {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Region List";
	static constexpr codec::nested<region_ls> format{};
};

/// CRC-16-CCITT of the packet, from the key to the length of this item
struct checksum {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "CRC-16-CCITT";
	static constexpr codec::integer<uint16_t> format{};
};

}  // namespace klv::misb1507
