/**
 * @file klv/misb0602/fields.hpp
 * @brief MISB ST 0602 Annotation Universal Set field descriptors (section 7)
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/misb0602/enumerations.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0602 {

/// Identifier of the annotated object
struct locally_unique_identifier {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 01 03 03 01 00 00 00 00");
	static constexpr std::string_view name = "Locally Unique Identifier";
	static constexpr codec::integer<uint32_t> format{};
};

/// Free text that describes the MIME Data, for example a title
struct description {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 03 02 01 06 03 00 00 00");
	static constexpr std::string_view name = "Description";
	static constexpr codec::string format{};
};

/// MIME media type of the MIME Data, as sent (see `parse_media_type`)
struct mime_media_type {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 07 04 09 02 00 00 00 00 00");
	static constexpr std::string_view name = "MIME Media Type";
	static constexpr codec::string format{};
};

/// The annotation, in the format of the MIME Media Type
struct mime_data {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 0E 01 02 05 01 00 00 00");
	static constexpr std::string_view name = "MIME Data";
	static constexpr codec::bytes format{};
};

/// X position of the reference point of the MIME Data (Table 2). The origin is the upper-left corner of the image.
struct x_viewport_position {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 07 01 02 03 01 00 00 00");
	static constexpr std::string_view name = "X Viewport Position in Pixels";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<int16_t> format{};
};

/// Y position of the reference point of the MIME Data (Table 2). The origin is the upper-left corner of the image.
struct y_viewport_position {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 07 01 02 03 02 00 00 00");
	static constexpr std::string_view name = "Y Viewport Position in Pixels";
	static constexpr std::string_view units = "pixels";
	static constexpr codec::integer<int16_t> format{};
};

struct event_indication {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 05 01 01 02 00 00 00 00");
	static constexpr std::string_view name = "Event Indication";
	static constexpr codec::enumeration<event> format{};
};

/// The author of the most recent event. The contents are user defined.
struct modification_history {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 0E 01 02 05 02 00 00 00");
	static constexpr std::string_view name = "Modification History";
	static constexpr codec::string format{};
};

struct annotation_source {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 0E 01 02 05 03 00 00 00");
	static constexpr std::string_view name = "Annotation Source";
	static constexpr codec::enumeration<source_kind> format{};
};

/// Drawing order of the annotations in a frame, from the lowest value to the highest value
struct z_order {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 0E 01 02 05 06 00 00 00");
	static constexpr std::string_view name = "Z-Order";
	static constexpr codec::ber_oid format{};
};

}  // namespace klv::misb0602
