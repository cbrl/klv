/**
 * @file klv/misb0602/check.hpp
 * @brief MISB ST 0602 event check: the items that an event requires
 */

#pragma once

#include "klv/misb0602/annotation_us.hpp"
#include "klv/misb0602/enumerations.hpp"
#include "klv/misb0602/fields.hpp"
#include "klv/set/profile.hpp"
#include "klv/set/record.hpp"

#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <string_view>
#include <vector>

namespace klv::misb0602 {

/// An item that an event requires (ST 0602 sections 6.2 and 6.2.5)
enum class required_item : uint8_t {
	locally_unique_identifier,  ///< Each event
	mime_media_type,  ///< NEW, MODIFY, and STATUS
	mime_data,  ///< NEW, MODIFY, and STATUS
	x_viewport_position,  ///< NEW, MOVE, MODIFY, and STATUS
	y_viewport_position,  ///< NEW, MOVE, MODIFY, and STATUS
	event_indication,  ///< Each event
	modification_history,  ///< NEW, MODIFY, DELETE, and STATUS
	annotation_source,  ///< NEW and STATUS
	z_order,  ///< NEW, MOVE, MODIFY, and STATUS
};

/// Name of the required item, with its requirements
[[nodiscard]]
constexpr auto to_string(required_item value) noexcept -> std::string_view {
	switch (value) {
		case required_item::locally_unique_identifier:
			return "Locally Unique Identifier (ST 0602.4-08)";
		case required_item::mime_media_type:
			return "MIME Media Type (ST 0602.4-12)";
		case required_item::mime_data:
			return "MIME Data (ST 0602.4-12)";
		case required_item::x_viewport_position:
			return "X Viewport Position in Pixels (ST 0602.4-12, -13)";
		case required_item::y_viewport_position:
			return "Y Viewport Position in Pixels (ST 0602.4-12, -13)";
		case required_item::event_indication:
			return "Event Indication (ST 0602.4-09)";
		case required_item::modification_history:
			return "Modification History (ST 0602.4-12, -14)";
		case required_item::annotation_source:
			return "Annotation Source (ST 0602.4-15, -16)";
		case required_item::z_order:
			return "Z-Order (ST 0602.4-12, -13)";
	}
	return {};
}

/// A required item that a record does not hold
using finding = klv::finding<required_item>;

/**
 * @brief The items that the event of `rec` requires, but that `rec` does not hold, in set order
 *
 * An item with a value meets a requirement. An empty item (length of zero) does not. Without a valid Event
 * Indication, the check requires only the Locally Unique Identifier and the Event Indication.
 */
[[nodiscard]]
inline auto check(record<annotation_us> const& rec) -> std::vector<finding> {
	auto const& kind = rec.get<event_indication>();
	auto const one_of = [&](std::initializer_list<event> events) {
		return kind.has_value() && std::ranges::contains(events, *kind);
	};
	auto const full = one_of({event::new_event, event::modify, event::status});
	auto const placed = full || one_of({event::move});

	auto findings = std::vector<finding>{};
	auto const require = [&](required_item which, bool required, bool reported) {
		if (required && !reported) {
			findings.push_back({.item = which, .reason = shortfall::missing});
		}
	};

	require(required_item::locally_unique_identifier, true, klv::reports<locally_unique_identifier>(rec));
	require(required_item::mime_media_type, full, klv::reports<mime_media_type>(rec));
	require(required_item::mime_data, full, klv::reports<mime_data>(rec));
	require(required_item::x_viewport_position, placed, klv::reports<x_viewport_position>(rec));
	require(required_item::y_viewport_position, placed, klv::reports<y_viewport_position>(rec));
	require(required_item::event_indication, true, kind.has_value());
	require(
	    required_item::modification_history,
	    full || one_of({event::delete_event}),
	    klv::reports<modification_history>(rec)
	);
	require(
	    required_item::annotation_source,
	    one_of({event::new_event, event::status}),
	    klv::reports<annotation_source>(rec)
	);
	require(required_item::z_order, placed, klv::reports<z_order>(rec));
	return findings;
}

}  // namespace klv::misb0602
