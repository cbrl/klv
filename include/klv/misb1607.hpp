/**
 * @file misb1607.hpp
 * @brief MISB ST 1607 - Child sets: the Amend Local Set and the Segment Local Set
 *
 * A child set holds items of its parent set. In MISB ST 0601, tags 100 and 101 hold child sets as records of the
 * parent set (`misb0601::segment_local_set` and `misb0601::amend_local_set`):
 * - An Amend LS corrects, adds, or clears items of the parent.
 * - Each Segment LS holds the items of one segment. The parent items outside the Segment LS apply to all segments.
 * - A child set can hold other child sets.
 *
 * `apply` and `merge` combine a child set with its parent.
 */

#pragma once

#include "klv/field.hpp"
#include "klv/set/definition.hpp"
#include "klv/set/record.hpp"

#include <type_traits>
#include <utility>

namespace klv::misb1607 {

/**
 * @brief Copy each field of `child` that is not absent into `parent`
 *
 * A child value replaces the parent value. An empty child field (length of zero) makes the parent field empty. The
 * function ignores a child field that the parent set does not define, and the unknown fields of the child.
 */
template<local_set_definition Parent, local_set_definition Child>
auto apply(record<Parent>& parent, record<Child> const& child) -> void {
	for_each_field(child, [&]<typename E>(std::type_identity<E>, field<value_t<E>> const& value) {
		if constexpr (member_of<E, Parent>) {
			if (value || value.error() != field_error::absent) {
				parent.template assign<E>(value);
			}
		}
	});
}

/// The parent with the fields of `child` applied: the amended set, or the complete set of one segment
template<local_set_definition Parent, local_set_definition Child>
[[nodiscard]]
auto merge(record<Parent> parent, record<Child> const& child) -> record<Parent> {
	apply(parent, child);
	return parent;
}

}  // namespace klv::misb1607
