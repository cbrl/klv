/**
 * @file klv/misb0602/preface.hpp
 * @brief MISB ST 0602 preface items (section 6.1)
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0602 {

/// Byte Order value of big-endian metadata ("MM"). ST 0602 allows only this value.
inline constexpr auto big_endian_byte_order = uint16_t{0x4D4D};

/// Byte order of the metadata (see `big_endian_byte_order`)
struct byte_order_item {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 03 01 02 01 02 00 00 00");
	static constexpr std::string_view name = "Byte Order";
	static constexpr codec::integer<uint16_t> format{};
};

/// Number of active lines (rows) in a frame of the Motion Imagery, before compression
struct active_lines_per_frame_item {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 04 01 03 02 02 00 00 00");
	static constexpr std::string_view name = "Active Lines per Frame";
	static constexpr codec::integer<uint16_t> format{};
};

/// Number of active samples (columns) in a line of the Motion Imagery, before compression
struct active_samples_per_line_item {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 04 01 05 01 02 00 00 00");
	static constexpr std::string_view name = "Active Samples per Line";
	static constexpr codec::integer<uint16_t> format{};
};

}  // namespace klv::misb0602
