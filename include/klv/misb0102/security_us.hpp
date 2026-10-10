/**
 * @file klv/misb0102/security_us.hpp
 * @brief MISB ST 0102 Security Metadata Universal Set definition, and the conversion to and from the Local Set
 *
 * `security_us` lists the 17 items of ST 0102 Table 1 in encode order. Each item has a 16-byte key. The set has no
 * checksum.
 *
 * The Universal Set sends three items as text: the Security Classification and the two country coding methods. The
 * Local Set sends them as enumerations. `to_local` and `to_universal` convert these items (ST 0102 section 6.8), and
 * copy the other items without a change.
 */

#pragma once

#include "klv/field.hpp"
#include "klv/misb0102/enumerations.hpp"
#include "klv/misb0102/fields.hpp"
#include "klv/misb0102/security_ls.hpp"
#include "klv/set.hpp"

#include <expected>
#include <optional>
#include <string>
#include <string_view>

namespace klv::misb0102 {

/// MISB ST 0102 Security Metadata Universal Set
struct security_us {
	static constexpr std::string_view name = "Security Universal Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 01 01 01 02 08 02 00 00 00 00 00");

	using fields = field_list<
	    security_classification_text,
	    classifying_country_coding_method_text,
	    classifying_country,
	    sci_shi_information,
	    caveats,
	    releasing_instructions,
	    classified_by,
	    derived_from,
	    classification_reason,
	    declassification_date,
	    classification_marking_system,
	    object_country_coding_method_text,
	    object_country_codes,
	    classification_comments,
	    version,
	    classifying_country_coding_method_version_date,
	    object_country_coding_method_version_date>;
};

namespace detail {

/// The enumeration value of a text field. A text without a match gives `field_error::invalid_value`.
template<typename Enum>
[[nodiscard]]
auto parse_field(field<std::string> const& text, std::optional<Enum> (*parse)(std::string_view)) -> field<Enum> {
	if (!text) {
		return std::unexpected(text.error());
	}
	if (auto const value = parse(*text)) {
		return *value;
	}
	return std::unexpected(field_error::invalid_value);
}

/// The Universal Set text of an enumeration field. A value without a text gives `field_error::invalid_value`.
template<typename Enum>
[[nodiscard]]
auto text_field(field<Enum> const& value) -> field<std::string> {
	if (!value) {
		return std::unexpected(value.error());
	}
	if (auto const text = universal_text(*value); !text.empty()) {
		return std::string{text};
	}
	return std::unexpected(field_error::invalid_value);
}

}  // namespace detail

/**
 * @brief The Local Set of a Universal Set record (ST 0102 section 6.8)
 *
 * A text without a matching value gives `field_error::invalid_value` in the Local Set field. The function does not
 * copy the unknown items.
 */
[[nodiscard]]
inline auto to_local(record<security_us> const& universal) -> record<security_ls> {
	auto local = record<security_ls>{};
	copy_fields(universal, local);
	local.assign<security_classification>(
	    detail::parse_field(universal.get<security_classification_text>(), &parse_classification)
	);
	local.assign<classifying_country_coding_method>(
	    detail::parse_field(universal.get<classifying_country_coding_method_text>(), &parse_classifying_country_coding)
	);
	local.assign<object_country_coding_method>(
	    detail::parse_field(universal.get<object_country_coding_method_text>(), &parse_object_country_coding)
	);
	return local;
}

/**
 * @brief The Universal Set of a Local Set record (ST 0102 section 6.8)
 *
 * A coding method of "Omitted Value" has no Universal Set text, and gives `field_error::invalid_value`. The Universal
 * Set has no UMID, Stream ID, Transport Stream ID, or Item Designator ID item (tags 15 to 21). The function does not
 * copy these items or the unknown items.
 */
[[nodiscard]]
inline auto to_universal(record<security_ls> const& local) -> record<security_us> {
	auto universal = record<security_us>{};
	copy_fields(local, universal);
	universal.assign<security_classification_text>(detail::text_field(local.get<security_classification>()));
	universal.assign<classifying_country_coding_method_text>(
	    detail::text_field(local.get<classifying_country_coding_method>())
	);
	universal.assign<object_country_coding_method_text>(detail::text_field(local.get<object_country_coding_method>()));
	return universal;
}

}  // namespace klv::misb0102
