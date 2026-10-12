/**
 * @file klv/misb0602/annotation_us.hpp
 * @brief MISB ST 0602 Annotation Universal Set definition
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/misb0602/fields.hpp"
#include "klv/set.hpp"

#include <string_view>

namespace klv::misb0602 {

/// MISB ST 0602 Annotation Universal Set
struct annotation_us {
	static constexpr std::string_view name = "Annotation Universal Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 01 01 01 0E 01 03 03 01 00 00 00");

	using fields = field_list<
	    locally_unique_identifier,
	    description,
	    mime_media_type,
	    mime_data,
	    x_viewport_position,
	    y_viewport_position,
	    event_indication,
	    modification_history,
	    annotation_source,
	    z_order>;
};

}  // namespace klv::misb0602
