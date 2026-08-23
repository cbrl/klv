/**
 * @file klv/misb0601/mission.hpp
 * @brief MISB ST 0601 fields: identification, time, security, and communication
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/misb0102/security_ls.hpp"
#include "klv/misb0601/enumerations.hpp"
#include "klv/misb0601/packs/command_record.hpp"
#include "klv/misb0601/packs/countries.hpp"
#include "klv/misb0806/rvt_ls.hpp"
#include "klv/misb1204.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0601 {

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

struct mission_id {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Mission ID";
	static constexpr codec::string format{.max_length = 127};
};

struct platform_tail_number {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Platform Tail Number";
	static constexpr codec::string format{.max_length = 127};
};

struct platform_designation {
	static constexpr uint32_t tag = 10;
	static constexpr std::string_view name = "Platform Designation";
	static constexpr codec::string format{.max_length = 127};
};

struct security_local_set {
	static constexpr uint32_t tag = 48;
	static constexpr std::string_view name = "Security Local Set";
	static constexpr codec::nested<misb0102::security_ls> format{};
};

struct platform_call_sign {
	static constexpr uint32_t tag = 59;
	static constexpr std::string_view name = "Platform Call Sign";
	static constexpr codec::string format{.max_length = 127};
};

struct uas_datalink_ls_version_number {
	static constexpr uint32_t tag = 65;
	static constexpr std::string_view name = "UAS Datalink LS Version Number";
	static constexpr codec::integer<uint8_t> format{};
};

struct event_start_time_utc {
	static constexpr uint32_t tag = 72;
	static constexpr std::string_view name = "Event Start Time - UTC";
	static constexpr codec::timestamp format{};
};

struct rvt_local_set {
	static constexpr uint32_t tag = 73;
	static constexpr std::string_view name = "RVT Local Set";
	static constexpr codec::nested<misb0806::rvt_ls> format{};
};

struct operational_mode {
	static constexpr uint32_t tag = 77;
	static constexpr std::string_view name = "Operational Mode";
	static constexpr codec::enumeration<operation_type> format{.name_of = &to_string};
};

/// MISB ST 1204 Motion Imagery Identification System core identifier
struct miis_core_identifier {
	static constexpr uint32_t tag = 94;
	static constexpr std::string_view name = "MIIS Core Identifier";
	static constexpr misb1204::core_id_codec format{};
};

struct stream_designator {
	static constexpr uint32_t tag = 106;
	static constexpr std::string_view name = "Stream Designator";
	static constexpr codec::string format{.max_length = 127};
};

struct operational_base {
	static constexpr uint32_t tag = 107;
	static constexpr std::string_view name = "Operational Base";
	static constexpr codec::string format{.max_length = 127};
};

struct broadcast_source {
	static constexpr uint32_t tag = 108;
	static constexpr std::string_view name = "Broadcast Source";
	static constexpr codec::string format{.max_length = 127};
};

/// The set can hold the field more than once: one Control Command for each command
struct control_command {
	static constexpr uint32_t tag = 115;
	static constexpr std::string_view name = "Control Command";
	static constexpr codec::repeated<command_record_codec> format{};
};

struct control_command_verification_list {
	static constexpr uint32_t tag = 116;
	static constexpr std::string_view name = "Control Command Verification List";
	static constexpr codec::ber_oid_list format{};
};

struct country_codes {
	static constexpr uint32_t tag = 122;
	static constexpr std::string_view name = "Country Codes";
	static constexpr countries_codec format{};
};

struct take_off_time {
	static constexpr uint32_t tag = 131;
	static constexpr std::string_view name = "Take-off Time";
	static constexpr codec::timestamp format{.min_length = 1};
};

struct transmission_frequency {
	static constexpr uint32_t tag = 132;
	static constexpr std::string_view name = "Transmission Frequency";
	static constexpr std::string_view units = "MHz";
	static constexpr codec::imapb format{1.0, 99'999.0, 3};
};

struct communications_method {
	static constexpr uint32_t tag = 135;
	static constexpr std::string_view name = "Communications Method";
	static constexpr codec::string format{.max_length = 127};
};

struct leap_seconds {
	static constexpr uint32_t tag = 136;
	static constexpr std::string_view name = "Leap Seconds";
	static constexpr std::string_view units = "s";
	static constexpr codec::integer<int32_t> format{};
};

struct correction_offset {
	static constexpr uint32_t tag = 137;
	static constexpr std::string_view name = "Correction Offset";
	static constexpr std::string_view units = "us";
	static constexpr codec::integer<int64_t> format{};
};

}  // namespace klv::misb0601
