/**
 * @file klv/misb0102/fields.hpp
 * @brief MISB ST 0102 field descriptors
 *
 * A field with a `tag` is a field of the Local Set (ST 0102 Table 2). A field with a `key` is a field of the Universal
 * Set (ST 0102 Table 1). Most fields have both. The Universal Set sends three items as text, and the Local Set sends
 * them as enumerations. These items are the Security Classification and the two country coding methods. Their text
 * items have separate descriptors, for example `security_classification_text`.
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/misb0102/enumerations.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0102 {

struct security_classification {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Security Classification";
	static constexpr codec::enumeration<classification> format{};
};

struct classifying_country_coding_method {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Classifying Country and Releasing Instructions Country Coding Method";
	static constexpr codec::enumeration<classifying_country_coding> format{};
};

struct classifying_country {
	static constexpr uint32_t tag = 3;
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 03 07 01 20 01 02 08 00 00");
	static constexpr std::string_view name = "Classifying Country";
	static constexpr codec::string format{};
};

struct sci_shi_information {
	static constexpr uint32_t tag = 4;
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 0E 01 02 03 02 00 00 00");
	static constexpr std::string_view name = "SCI/SHI Information";
	static constexpr codec::string format{};
};

struct caveats {
	static constexpr uint32_t tag = 5;
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 03 02 08 02 02 00 00 00 00");
	static constexpr std::string_view name = "Caveats";
	static constexpr codec::string format{};
};

struct releasing_instructions {
	static constexpr uint32_t tag = 6;
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 03 07 01 20 01 02 09 00 00");
	static constexpr std::string_view name = "Releasing Instructions";
	static constexpr codec::string format{};
};

struct classified_by {
	static constexpr uint32_t tag = 7;
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 03 02 08 02 03 00 00 00 00");
	static constexpr std::string_view name = "Classified By";
	static constexpr codec::string format{};
};

struct derived_from {
	static constexpr uint32_t tag = 8;
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 03 02 08 02 06 00 00 00 00");
	static constexpr std::string_view name = "Derived From";
	static constexpr codec::string format{};
};

struct classification_reason {
	static constexpr uint32_t tag = 9;
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 03 02 08 02 04 00 00 00 00");
	static constexpr std::string_view name = "Classification Reason";
	static constexpr codec::string format{};
};

struct declassification_date {
	static constexpr uint32_t tag = 10;
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 03 02 08 02 05 00 00 00 00");
	static constexpr std::string_view name = "Declassification Date";
	static constexpr codec::string format{};
};

struct classification_marking_system {
	static constexpr uint32_t tag = 11;
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 03 02 08 02 08 00 00 00 00");
	static constexpr std::string_view name = "Classification and Marking System";
	static constexpr codec::string format{};
};

struct object_country_coding_method {
	static constexpr uint32_t tag = 12;
	static constexpr std::string_view name = "Object Country Coding Method";
	static constexpr codec::enumeration<object_country_coding> format{};
};

/// UTF-16 text. Decoding also accepts the 8-bit text of older producers.
struct object_country_codes {
	static constexpr uint32_t tag = 13;
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 03 07 01 20 01 02 01 01 00");
	static constexpr std::string_view name = "Object Country Codes";
	static constexpr codec::utf16_string format{.legacy_8bit = true};
};

struct classification_comments {
	static constexpr uint32_t tag = 14;
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 03 02 08 02 07 00 00 00 00");
	static constexpr std::string_view name = "Classification Comments";
	static constexpr codec::string format{};
};

struct umid_video {
	static constexpr uint32_t tag = 15;
	static constexpr std::string_view name = "UMID Video";
	static constexpr codec::bytes format{};
};

struct umid_audio {
	static constexpr uint32_t tag = 16;
	static constexpr std::string_view name = "UMID Audio";
	static constexpr codec::bytes format{};
};

struct umid_data {
	static constexpr uint32_t tag = 17;
	static constexpr std::string_view name = "UMID Data";
	static constexpr codec::bytes format{};
};

struct umid_system {
	static constexpr uint32_t tag = 18;
	static constexpr std::string_view name = "UMID System";
	static constexpr codec::bytes format{};
};

struct stream_id {
	static constexpr uint32_t tag = 19;
	static constexpr std::string_view name = "Stream ID";
	static constexpr codec::integer<uint8_t> format{};
};

struct transport_stream_id {
	static constexpr uint32_t tag = 20;
	static constexpr std::string_view name = "Transport Stream ID";
	static constexpr codec::integer<uint16_t> format{};
};

struct item_designator_id {
	static constexpr uint32_t tag = 21;
	static constexpr std::string_view name = "Item Designator ID";
	static constexpr codec::bytes format{};
};

struct version {
	static constexpr uint32_t tag = 22;
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 0E 01 02 05 04 00 00 00");
	static constexpr std::string_view name = "Version";
	static constexpr codec::integer<uint16_t> format{};
};

struct classifying_country_coding_method_version_date {
	static constexpr uint32_t tag = 23;
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 0E 01 04 03 03 00 00 00");
	static constexpr std::string_view name =
	    "Classifying Country and Releasing Instructions Country Coding Method Version Date";
	static constexpr codec::string format{};
};

struct object_country_coding_method_version_date {
	static constexpr uint32_t tag = 24;
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 0E 01 04 03 04 00 00 00");
	static constexpr std::string_view name = "Object Country Coding Method Version Date";
	static constexpr codec::string format{};
};

// Universal Set items that the Local Set sends as an enumeration

/// Security Classification text, for example "UNCLASSIFIED//" (see `parse_classification`)
struct security_classification_text {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 03 02 08 02 01 00 00 00 00");
	static constexpr std::string_view name = "Security Classification";
	static constexpr codec::string format{};
};

/// Classifying Country Coding Method text, for example "GENC Two Letter" (see `parse_classifying_country_coding`)
struct classifying_country_coding_method_text {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 03 07 01 20 01 02 07 00 00");
	static constexpr std::string_view name = "Classifying Country and Releasing Instructions Country Coding Method";
	static constexpr codec::string format{};
};

/// Object Country Coding Method text, for example "GENC Three Letter" (see `parse_object_country_coding`)
struct object_country_coding_method_text {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 03 07 01 20 01 02 06 00 00");
	static constexpr std::string_view name = "Object Country Coding Method";
	static constexpr codec::string format{};
};

}  // namespace klv::misb0102
