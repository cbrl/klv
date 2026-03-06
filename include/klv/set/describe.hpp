/**
 * @file klv/set/describe.hpp
 * @brief Display text of a record, for logs and user interfaces
 */

#pragma once

#include "klv/bytes/hex.hpp"
#include "klv/field.hpp"
#include "klv/set/definition.hpp"
#include "klv/set/record.hpp"

#include <cstdint>
#include <format>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace klv {

/// Display text of one field
struct field_text {
	uint32_t tag = 0;
	std::string name;  ///< Field name. A field of a nested set has the nested field name as a prefix.
	std::string_view units;  ///< Units, or empty
	field<std::string> value;  ///< Display text, or the reason that the field has no value
};

namespace detail {

template<local_set_definition S>
auto describe_into(
	record<S> const& rec,
	std::string_view prefix,
	bool include_absent,
	std::vector<field_text>& out
) -> void {
	auto const full_name = [&](std::string_view name) {
		return prefix.empty() ? std::string{name} : std::format("{}.{}", prefix, name);
	};

	for_each_field(rec, [&]<typename E>(std::type_identity<E>, field<value_t<E>> const& value) {
		if (!value && value.error() == field_error::absent && !include_absent) {
			return;
		}

		if constexpr (is_record<value_t<E>>::value) {
			if (value) {
				describe_into(*value, full_name(E::name), include_absent, out);
				return;
			}
		}

		out.push_back(
		    field_text{
		        .tag = static_cast<uint32_t>(E::tag),
		        .name = full_name(E::name),
		        .units = units_of<E>(),
		        .value = value.transform([](value_t<E> const& v) { return E::format.to_string(v); }),
		    }
		);
	});

	for (auto const& unknown : rec.unknown()) {
		out.push_back(
		    field_text{
		        .tag = unknown.tag,
		        .name = full_name(std::format("Unknown tag {}", unknown.tag)),
		        .units = {},
		        .value = to_hex(unknown.value),
		    }
		);
	}
}

}  // namespace detail

/**
 * @brief Flat display list of a record, for logs and user interfaces
 *
 * The list contains the fields in definition order, then the unknown fields. A decoded nested set adds its own fields
 * with the name prefix "<nested field name>.".
 *
 * @param include_absent Also list the fields that are absent
 */
template<local_set_definition S>
[[nodiscard]]
auto describe(record<S> const& rec, bool include_absent = false) -> std::vector<field_text> {
	auto out = std::vector<field_text>{};
	detail::describe_into(rec, {}, include_absent, out);
	return out;
}

}  // namespace klv
