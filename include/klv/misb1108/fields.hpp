/**
 * @file klv/misb1108/fields.hpp
 * @brief MISB ST 1108 Interpretability and Quality Local Set field descriptors
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/misb1108/enumerations.hpp"
#include "klv/misb1108/metric_ls.hpp"
#include "klv/misb1108/packs.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb1108 {

struct assessment_point {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Assessment Point";
	static constexpr codec::enumeration<assessment_location> format{};
};

struct metric_period_pack {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Metric Period Pack";
	static constexpr metric_period_codec format{};
};

struct window_corners_pack {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Window Corners Pack";
	static constexpr window_corners_codec format{};
};

/// One item for each metric
struct metric_local_set {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Metric Local Set";
	static constexpr codec::repeated<codec::nested<metric_ls>> format{};
};

struct compression_type {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "Compression Type";
	static constexpr codec::enumeration<compression_method> format{};
};

struct compression_profile {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "Compression Profile";
	static constexpr codec::enumeration<compression_profile_kind> format{};
};

/// Level of the compression, for example "4.1" or "5" (H.264 and H.265) or "HL" (H.262). "N/A" when uncompressed.
struct compression_level {
	static constexpr uint32_t tag = 7;
	static constexpr std::string_view name = "Compression Level";
	static constexpr codec::string format{.max_length = 3};
};

/// Source size divided by compressed size. 1.0 when uncompressed.
struct compression_ratio {
	static constexpr uint32_t tag = 8;
	static constexpr std::string_view name = "Compression Ratio";
	static constexpr codec::ieee754_variable format{};
};

struct stream_bitrate {
	static constexpr uint32_t tag = 9;
	static constexpr std::string_view name = "Stream Bitrate";
	static constexpr std::string_view units = "kbit/s";
	static constexpr codec::integer<uint16_t> format{.length = codec::variable_length};
};

/// Version of ST 1108, for example 4 for ST 1108.4
struct document_version {
	static constexpr uint32_t tag = 10;
	static constexpr std::string_view name = "Document Version";
	static constexpr codec::integer<uint8_t> format{.length = codec::variable_length};
};

/// CRC-16-CCITT of the packet, from the key to the length of this item
struct checksum {
	static constexpr uint32_t tag = 11;
	static constexpr std::string_view name = "CRC-16-CCITT";
	static constexpr codec::integer<uint16_t> format{};
};

}  // namespace klv::misb1108
