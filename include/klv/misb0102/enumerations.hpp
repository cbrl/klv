/**
 * @file klv/misb0102/enumerations.hpp
 * @brief MISB ST 0102 enumerations
 */

#pragma once

#include <cstdint>
#include <string_view>

namespace klv::misb0102 {

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

}  // namespace klv::misb0102
