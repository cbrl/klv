/**
 * @file misb0102.hpp
 * @brief MISB ST 0102 - Security Metadata Local Set
 *
 * The set usually arrives nested in MISB ST 0601 tag 48 (see `misb0601::security_local_set`). Each struct below is a
 * static field descriptor. `security_ls` lists the fields.
 */

#pragma once

#include "codec.hpp"
#include "set.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0102 {

// =====================================================================================================================
// Enumerations
// =====================================================================================================================

/// Security classification (tag 1)
enum class classification : uint8_t {
	unclassified = 0x01,
	restricted = 0x02,
	confidential = 0x03,
	secret = 0x04,
	top_secret = 0x05,
};

[[nodiscard]]
constexpr auto to_string(classification value) -> std::string_view {
	switch (value) {
		case classification::unclassified:
			return "UNCLASSIFIED";
		case classification::restricted:
			return "RESTRICTED";
		case classification::confidential:
			return "CONFIDENTIAL";
		case classification::secret:
			return "SECRET";
		case classification::top_secret:
			return "TOP SECRET";
	}
	return {};
}

/// Country coding method of the classifying and releasing countries (tag 2)
enum class classifying_country_coding : uint8_t {
	iso_3166_two_letter = 0x01,
	iso_3166_three_letter = 0x02,
	fips_10_4_two_letter = 0x03,
	fips_10_4_four_letter = 0x04,
	iso_3166_numeric = 0x05,
	stanag_1059_two_letter = 0x06,
	stanag_1059_three_letter = 0x07,
	omitted_value_08 = 0x08,
	omitted_value_09 = 0x09,
	fips_10_4_mixed = 0x0A,
	iso_3166_mixed = 0x0B,
	stanag_1059_mixed = 0x0C,
	genc_two_letter = 0x0D,
	genc_three_letter = 0x0E,
	genc_numeric = 0x0F,
	genc_mixed = 0x10,
};

[[nodiscard]]
constexpr auto to_string(classifying_country_coding value) -> std::string_view {
	using enum classifying_country_coding;
	switch (value) {
		case iso_3166_two_letter:
			return "ISO-3166 Two Letter";
		case iso_3166_three_letter:
			return "ISO-3166 Three Letter";
		case fips_10_4_two_letter:
			return "FIPS 10-4 Two Letter";
		case fips_10_4_four_letter:
			return "FIPS 10-4 Four Letter";
		case iso_3166_numeric:
			return "ISO-3166 Numeric";
		case stanag_1059_two_letter:
			return "1059 Two Letter";
		case stanag_1059_three_letter:
			return "1059 Three Letter";
		case omitted_value_08:
		case omitted_value_09:
			return "Omitted Value";
		case fips_10_4_mixed:
			return "FIPS 10-4 Mixed";
		case iso_3166_mixed:
			return "ISO 3166 Mixed";
		case stanag_1059_mixed:
			return "STANAG 1059 Mixed";
		case genc_two_letter:
			return "GENC Two Letter";
		case genc_three_letter:
			return "GENC Three Letter";
		case genc_numeric:
			return "GENC Numeric";
		case genc_mixed:
			return "GENC Mixed";
	}
	return {};
}

/// Country coding method of the object country codes (tag 12). The values differ from tag 2.
enum class object_country_coding : uint8_t {
	iso_3166_two_letter = 0x01,
	iso_3166_three_letter = 0x02,
	iso_3166_numeric = 0x03,
	fips_10_4_two_letter = 0x04,
	fips_10_4_four_letter = 0x05,
	stanag_1059_two_letter = 0x06,
	stanag_1059_three_letter = 0x07,
	omitted_value_08 = 0x08,
	omitted_value_09 = 0x09,
	omitted_value_0a = 0x0A,
	omitted_value_0b = 0x0B,
	omitted_value_0c = 0x0C,
	genc_two_letter = 0x0D,
	genc_three_letter = 0x0E,
	genc_numeric = 0x0F,
	genc_admin_sub = 0x40,
};

[[nodiscard]]
constexpr auto to_string(object_country_coding value) -> std::string_view {
	using enum object_country_coding;
	switch (value) {
		case iso_3166_two_letter:
			return "ISO-3166 Two Letter";
		case iso_3166_three_letter:
			return "ISO-3166 Three Letter";
		case iso_3166_numeric:
			return "ISO-3166 Numeric";
		case fips_10_4_two_letter:
			return "FIPS 10-4 Two Letter";
		case fips_10_4_four_letter:
			return "FIPS 10-4 Four Letter";
		case stanag_1059_two_letter:
			return "1059 Two Letter";
		case stanag_1059_three_letter:
			return "1059 Three Letter";
		case omitted_value_08:
		case omitted_value_09:
		case omitted_value_0a:
		case omitted_value_0b:
		case omitted_value_0c:
			return "Omitted Value";
		case genc_two_letter:
			return "GENC Two Letter";
		case genc_three_letter:
			return "GENC Three Letter";
		case genc_numeric:
			return "GENC Numeric";
		case genc_admin_sub:
			return "GENC AdminSub";
	}
	return {};
}

// =====================================================================================================================
// Fields
// =====================================================================================================================

struct security_classification {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Security Classification";
	static constexpr codec::enumeration<classification> format{.name_of = &to_string};
};

struct classifying_country_coding_method {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Classifying Country and Releasing Instructions Country Coding Method";
	static constexpr codec::enumeration<classifying_country_coding> format{.name_of = &to_string};
};

struct classifying_country {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Classifying Country";
	static constexpr codec::string format{};
};

struct sci_shi_information {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "SCI/SHI Information";
	static constexpr codec::string format{};
};

struct caveats {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "Caveats";
	static constexpr codec::string format{};
};

struct releasing_instructions {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "Releasing Instructions";
	static constexpr codec::string format{};
};

struct classified_by {
	static constexpr uint32_t tag = 7;
	static constexpr std::string_view name = "Classified By";
	static constexpr codec::string format{};
};

struct derived_from {
	static constexpr uint32_t tag = 8;
	static constexpr std::string_view name = "Derived From";
	static constexpr codec::string format{};
};

struct classification_reason {
	static constexpr uint32_t tag = 9;
	static constexpr std::string_view name = "Classification Reason";
	static constexpr codec::string format{};
};

struct declassification_date {
	static constexpr uint32_t tag = 10;
	static constexpr std::string_view name = "Declassification Date";
	static constexpr codec::string format{};
};

struct classification_marking_system {
	static constexpr uint32_t tag = 11;
	static constexpr std::string_view name = "Classification and Marking System";
	static constexpr codec::string format{};
};

struct object_country_coding_method {
	static constexpr uint32_t tag = 12;
	static constexpr std::string_view name = "Object Country Coding Method";
	static constexpr codec::enumeration<object_country_coding> format{.name_of = &to_string};
};

struct object_country_codes {
	static constexpr uint32_t tag = 13;
	static constexpr std::string_view name = "Object Country Codes";
	static constexpr codec::string format{};
};

struct classification_comments {
	static constexpr uint32_t tag = 14;
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
	static constexpr std::string_view name = "Version";
	static constexpr codec::integer<uint16_t> format{};
};

struct classifying_country_coding_method_version_date {
	static constexpr uint32_t tag = 23;
	static constexpr std::string_view name =
	    "Classifying Country and Releasing Instructions Country Coding Method Version Date";
	static constexpr codec::string format{};
};

struct object_country_coding_method_version_date {
	static constexpr uint32_t tag = 24;
	static constexpr std::string_view name = "Object Country Coding Method Version Date";
	static constexpr codec::string format{};
};

// =====================================================================================================================
// Set
// =====================================================================================================================

/// MISB ST 0102 Security Metadata Local Set
struct security_ls {
	static constexpr std::string_view name = "Security Local Set";

	using fields = field_list<
	    security_classification,
	    classifying_country_coding_method,
	    classifying_country,
	    sci_shi_information,
	    caveats,
	    releasing_instructions,
	    classified_by,
	    derived_from,
	    classification_reason,
	    declassification_date,
	    classification_marking_system,
	    object_country_coding_method,
	    object_country_codes,
	    classification_comments,
	    umid_video,
	    umid_audio,
	    umid_data,
	    umid_system,
	    stream_id,
	    transport_stream_id,
	    item_designator_id,
	    version,
	    classifying_country_coding_method_version_date,
	    object_country_coding_method_version_date>;
};

}  // namespace klv::misb0102
