/**
 * @file klv/set/describe.hpp
 * @brief Display text of a record, for logs and user interfaces
 */

#pragma once

#include "klv/bytes/hex.hpp"
#include "klv/field.hpp"
#include "klv/set/coding.hpp"
#include "klv/set/definition.hpp"
#include "klv/set/record.hpp"

#include <cstddef>
#include <cstdint>
#include <format>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace klv {

/// Display text of one field
struct field_text {
	item_id id;  ///< Tag or key of the item
	std::string name;  ///< Field name. A field of a nested set has the nested field name as a prefix.
	std::string_view units;  ///< Units, or empty
	field<std::string> value;  ///< Display text, or the reason that the field has no value
};

namespace detail {

template<set_definition S>
auto describe_into(
	record<S> const& rec,
	std::string_view prefix,
	bool include_absent,
	std::vector<field_text>& out
) -> void {
	auto const full_name = [&](std::string_view name) {
		return prefix.empty() ? std::string{name} : std::format("{}.{}", prefix, name);
	};

	// One entry without a value, or with the display text of a value
	auto const add_entry = [&](item_id id, std::string_view name, std::string_view units, field<std::string> value) {
		out.push_back(field_text{.id = id, .name = full_name(name), .units = units, .value = std::move(value)});
	};

	// Each field of each set instantiates this body.
	for_each_field(rec, [&]<typename E>(std::type_identity<E>, field<value_t<E>> const& value) {
		auto const id = item_id{id_of<S, E>()};
		constexpr auto units = units_of<E>();

		if (!value) {
			if (value.error() != field_error::absent || include_absent) {
				add_entry(id, E::name, units, std::unexpected(value.error()));
			}
		}
		else if constexpr (is_record<value_t<E>>::value) {
			describe_into(*value, full_name(E::name), include_absent, out);
		}
		else if constexpr (is_record_list<value_t<E>>::value) {
			if (value->empty()) {
				add_entry(id, E::name, units, E::format.to_string(*value));
			}
			for (auto i = size_t{0}; i < value->size(); ++i) {
				describe_into((*value)[i], full_name(std::format("{}[{}]", E::name, i)), include_absent, out);
			}
		}
		else {
			add_entry(id, E::name, units, E::format.to_string(*value));
		}
	});

	for (auto const& unknown : rec.unknown()) {
		out.push_back(
		    field_text{
		        .id = unknown.id,
		        .name = full_name(std::format("Unknown {}", to_string(item_id{unknown.id}))),
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
 * The list contains the fields in definition order, then the unknown items. An unknown item has the name
 * "Unknown tag <tag>" or "Unknown key <key>". A decoded nested set adds its own fields with the name prefix
 * "<nested field name>.". A list of nested sets adds the fields of each set with the name prefix
 * "<nested field name>[<index>].".
 *
 * @param include_absent Also list the fields that are absent
 */
template<set_definition S>
[[nodiscard]]
auto describe(record<S> const& rec, bool include_absent = false) -> std::vector<field_text> {
	auto out = std::vector<field_text>{};
	detail::describe_into(rec, {}, include_absent, out);
	return out;
}

}  // namespace klv
