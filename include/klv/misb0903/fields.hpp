/**
 * @file klv/misb0903/fields.hpp
 * @brief MISB ST 0903 VMTI Local Set field descriptors
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/misb0903/algorithm_ls.hpp"
#include "klv/misb0903/ontology_ls.hpp"
#include "klv/misb0903/vtarget_ls.hpp"
#include "klv/misb1204/core_id_codec.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0903 {

struct checksum {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Checksum";
	static constexpr codec::integer<uint16_t> format{};
};

struct precision_time_stamp {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Precision Time Stamp";
	static constexpr codec::timestamp format{};
};

struct vmti_system_name {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "VMTI System Name";
	static constexpr codec::string format{.max_length = 32};
};

struct vmti_ls_version {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "VMTI LS Version";
	static constexpr codec::integer<uint16_t> format{.length = codec::variable_length};
};

struct total_number_of_targets_detected {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "Total Number of Targets Detected";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

/// Number of targets in the VTarget Series, after culling
struct number_of_targets_reported {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "Number of Targets Reported";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

/// Deprecated
struct frame_number {
	static constexpr uint32_t tag = 7;
	static constexpr std::string_view name = "Frame Number";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

struct frame_width {
	static constexpr uint32_t tag = 8;
	static constexpr std::string_view name = "Frame Width";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

struct frame_height {
	static constexpr uint32_t tag = 9;
	static constexpr std::string_view name = "Frame Height";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

struct vmti_source_sensor {
	static constexpr uint32_t tag = 10;
	static constexpr std::string_view name = "VMTI Source Sensor";
	static constexpr codec::string format{.max_length = 128};
};

struct vmti_horizontal_field_of_view {
	static constexpr uint32_t tag = 11;
	static constexpr std::string_view name = "VMTI Horizontal Field of View";
	static constexpr std::string_view units = "deg";
	static constexpr codec::imapb format{0.0, 180.0, 2};
};

struct vmti_vertical_field_of_view {
	static constexpr uint32_t tag = 12;
	static constexpr std::string_view name = "VMTI Vertical Field of View";
	static constexpr std::string_view units = "deg";
	static constexpr codec::imapb format{0.0, 180.0, 2};
};

/// MISB ST 1204 Motion Imagery Identification System core identifier
struct miis_id {
	static constexpr uint32_t tag = 13;
	static constexpr std::string_view name = "MIIS ID";
	static constexpr misb1204::core_id_codec format{};
};

struct vtarget_series {
	static constexpr uint32_t tag = 101;
	static constexpr std::string_view name = "VTarget Series";
	static constexpr codec::series<vtarget_codec> format{};
};

struct algorithm_series {
	static constexpr uint32_t tag = 102;
	static constexpr std::string_view name = "Algorithm Series";
	static constexpr codec::series<codec::nested<algorithm_ls>> format{};
};

struct ontology_series {
	static constexpr uint32_t tag = 103;
	static constexpr std::string_view name = "Ontology Series";
	static constexpr codec::series<codec::nested<ontology_ls>> format{};
};

}  // namespace klv::misb0903
