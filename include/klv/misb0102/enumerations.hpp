/**
 * @file klv/misb0102/enumerations.hpp
 * @brief MISB ST 0102 enumerations
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
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

// Universal Set text (ST 0102 Table 1 and section 6.8)

/// Universal Set text of a classification, for example "UNCLASSIFIED//", or an empty view
[[nodiscard]]
constexpr auto universal_text(classification value) -> std::string_view {
	switch (value) {
		case classification::unclassified:
			return "UNCLASSIFIED//";
		case classification::restricted:
			return "RESTRICTED//";
		case classification::confidential:
			return "CONFIDENTIAL//";
		case classification::secret:
			return "SECRET//";
		case classification::top_secret:
			return "TOP SECRET//";
	}
	return {};
}

/// Universal Set text of a coding method, or an empty view. An "Omitted Value" has no Universal Set text.
[[nodiscard]]
constexpr auto universal_text(classifying_country_coding value) -> std::string_view {
	auto const text = to_string(value);
	return text == "Omitted Value" ? std::string_view{} : text;
}

/// Universal Set text of a coding method, or an empty view. An "Omitted Value" has no Universal Set text.
[[nodiscard]]
constexpr auto universal_text(object_country_coding value) -> std::string_view {
	auto const text = to_string(value);
	return text == "Omitted Value" ? std::string_view{} : text;
}

namespace detail {

/// True when the two texts are equal. The comparison ignores the case of ASCII letters.
[[nodiscard]]
constexpr auto equal_ignoring_case(std::string_view first, std::string_view second) noexcept -> bool {
	auto const lower = [](char c) { return c >= 'A' && c <= 'Z' ? static_cast<char>(c - 'A' + 'a') : c; };
	if (first.size() != second.size()) {
		return false;
	}
	for (auto i = size_t{0}; i < first.size(); ++i) {
		if (lower(first[i]) != lower(second[i])) {
			return false;
		}
	}
	return true;
}

/// The value of `Enum` whose Universal Set text is `text`
template<typename Enum>
[[nodiscard]]
constexpr auto parse_universal_text(std::string_view text) -> std::optional<Enum> {
	for (auto i = 0U; i <= 0xFFU; ++i) {
		auto const value = static_cast<Enum>(i);
		if (auto const name = universal_text(value); !name.empty() && equal_ignoring_case(name, text)) {
			return value;
		}
	}
	return std::nullopt;
}

}  // namespace detail

/**
 * @brief The classification of a Universal Set text, for example "SECRET//"
 *
 * The comparison ignores the case of ASCII letters. ST 0102 section 6.8 gives upper-case text, but Table 1 gives the
 * coding methods in mixed case.
 */
[[nodiscard]]
constexpr auto parse_classification(std::string_view text) -> std::optional<classification> {
	return detail::parse_universal_text<classification>(text);
}

/// The coding method of a Universal Set text, for example "GENC Two Letter". The comparison ignores the case.
[[nodiscard]]
constexpr auto parse_classifying_country_coding(std::string_view text) -> std::optional<classifying_country_coding> {
	return detail::parse_universal_text<classifying_country_coding>(text);
}

/// The coding method of a Universal Set text, for example "GENC AdminSub". The comparison ignores the case.
[[nodiscard]]
constexpr auto parse_object_country_coding(std::string_view text) -> std::optional<object_country_coding> {
	return detail::parse_universal_text<object_country_coding>(text);
}

}  // namespace klv::misb0102
