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

#include <concepts>
#include <type_traits>
#include <vector>

namespace klv::misb1607 {

namespace detail {

/// Field `E` of `S` holds child sets of `S`: records of `S` (for example ST 0601 tags 100 and 101)
template<typename E, typename S>
concept child_set_field = std::same_as<value_t<E>, std::vector<record<S>>>;

}  // namespace detail

/**
 * @brief Apply `child` to `parent`. Afterwards, `parent` is the complete set of the child.
 *
 * A child value replaces the parent value. An empty child field (length of zero) makes the parent field empty. A child
 * field that did not decode (for example `field_error::bad_length`) does not change the parent field. The function
 * ignores a child field that the parent set does not define, and the unknown fields of the child.
 *
 * The child sets of the parent are `child` and its siblings. They do not apply to the result. Thus, the child sets of
 * the result are the child sets of `child`, which a later `apply` can apply in turn.
 */
template<set_definition Parent, set_definition Child>
auto apply(record<Parent>& parent, record<Child> const& child) -> void {
	[&]<typename... E>(field_list<E...>) {
		(
		    [&] {
			    if constexpr (detail::child_set_field<E, Parent>) {
				    parent.template erase<E>();
			    }
		    }(),
		    ...
		);
	}(typename Parent::fields{});

	for_each_field(child, [&]<typename E>(std::type_identity<E>, field<value_t<E>> const& value) {
		if constexpr (member_of<E, Parent>) {
			if (detail::child_set_field<E, Parent> || value || value.error() == field_error::empty) {
				parent.template assign<E>(value);
			}
		}
	});
}

/// The parent with `child` applied (see `apply`): the amended set, or the complete set of one segment
template<set_definition Parent, set_definition Child>
[[nodiscard]]
auto merge(record<Parent> parent, record<Child> const& child) -> record<Parent> {
	apply(parent, child);
	return parent;
}

}  // namespace klv::misb1607
