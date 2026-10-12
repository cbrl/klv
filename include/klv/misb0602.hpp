/**
 * @file misb0602.hpp
 * @brief MISB ST 0602 - Annotation Metadata Set
 *
 * An annotation data stream is a sequence of KLV packets. Three standalone preface items help a receiver read the
 * Annotation Universal Sets that follow them. These items are Byte Order, Active Lines per Frame, and Active Samples
 * per Line. Each Annotation Universal Set holds one event of one annotated object, for example NEW or MOVE. The set
 * has no checksum.
 * @code
 * for (auto const& packet : klv::misb0602::decode_stream(bytes)) {
 *     if (!packet) { report(packet.error()); continue; }
 *     if (auto const* annotation = std::get_if<klv::record<klv::misb0602::annotation_us>>(&*packet)) {
 *         for (auto const& finding : klv::misb0602::check(*annotation)) {
 *             // to_string(finding.item), to_string(finding.reason)
 *         }
 *     }
 * }
 * @endcode
 *
 * `check` gives the items that an event requires, but that a record does not hold. It does not check these rules:
 * - The preface items come before an Annotation Universal Set at least each quarter second (ST 0602.4-04 to -06).
 * - Each annotated object gets a STATUS or MODIFY event at least once every five seconds (ST 0602.4-17).
 * - An annotation without a Z-Order has a Z-Order of zero for rendering (ST 0602.4-18).
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/packet.hpp"
#include "klv/set.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <generator>
#include <initializer_list>
#include <optional>
#include <string_view>
#include <vector>

namespace klv::misb0602 {

/// Event Indication (Table 3). Each value is a string of one digit, for example "1" (0x31) for NEW.
enum class event : uint8_t {
	new_event = 0x31,  ///< Creates an annotated object
	move = 0x32,  ///< Changes the position of an annotated object in the viewport
	modify = 0x33,  ///< Changes the data of an annotated object
	delete_event = 0x34,  ///< Ends an annotated object
	status = 0x35,  ///< Gives the current data and position of an annotated object, at regular intervals
};

[[nodiscard]]
constexpr auto to_string(event value) noexcept -> std::string_view {
	switch (value) {
		case event::new_event:
			return "NEW";
		case event::move:
			return "MOVE";
		case event::modify:
			return "MODIFY";
		case event::delete_event:
			return "DELETE";
		case event::status:
			return "STATUS";
	}
	return {};
}

/// Source of an annotation (Table 4)
enum class source_kind : uint32_t {
	manual = 0x00,
	encyclopedia = 0x01,
	center_point = 0x02,
	area_of_interest = 0x04,
	pixel_analysis = 0x08,
};

[[nodiscard]]
constexpr auto to_string(source_kind value) noexcept -> std::string_view {
	switch (value) {
		case source_kind::manual:
			return "Manually Annotated";
		case source_kind::encyclopedia:
			return "Automated from the Basic Encyclopedia or other sources";
		case source_kind::center_point:
			return "Automated from user defined latitude / longitude center point";
		case source_kind::area_of_interest:
			return "Automated from user defined AOI (Area of Interest)";
		case source_kind::pixel_analysis:
			return "Automated from pixel analysis";
	}
	return {};
}

/// MIME media type of the MIME Data (Table 1)
enum class media_type : uint8_t {
	bmp,
	cgm,
	jpeg,
	png,
	svg,
};

/// MIME text of a media type, for example "image/svg+xml"
[[nodiscard]]
constexpr auto to_string(media_type value) noexcept -> std::string_view {
	switch (value) {
		case media_type::bmp:
			return "image/x-ms-bmp";
		case media_type::cgm:
			return "image/cgm";
		case media_type::jpeg:
			return "image/jpeg";
		case media_type::png:
			return "image/png";
		case media_type::svg:
			return "image/svg+xml";
	}
	return {};
}

/**
 * @brief The media type of a MIME Media Type text, for example "image/png"
 *
 * The comparison ignores the case of ASCII letters (RFC 2045). ST 0602 versions before version 2 used the text "cgm".
 * This text also gives `media_type::cgm`.
 */
[[nodiscard]]
constexpr auto parse_media_type(std::string_view text) -> std::optional<media_type> {
	using enum media_type;
	if (klv::detail::equal_ignoring_case(text, "cgm")) {
		return cgm;
	}
	for (auto const value : std::array{bmp, cgm, jpeg, png, svg}) {
		if (klv::detail::equal_ignoring_case(text, to_string(value))) {
			return value;
		}
	}
	return std::nullopt;
}

// Preface items (section 6.1)

/// Byte Order value of big-endian metadata ("MM"). ST 0602 allows only this value.
inline constexpr auto big_endian_byte_order = uint16_t{0x4D4D};

/// Byte order of the metadata (see `big_endian_byte_order`)
struct byte_order_item {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 03 01 02 01 02 00 00 00");
	static constexpr std::string_view name = "Byte Order";
	static constexpr codec::integer<uint16_t> format{};
};

/// Number of active lines (rows) in a frame of the Motion Imagery, before compression
struct active_lines_per_frame_item {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 04 01 03 02 02 00 00 00");
	static constexpr std::string_view name = "Active Lines per Frame";
	static constexpr codec::integer<uint16_t> format{};
};

/// Number of active samples (columns) in a line of the Motion Imagery, before compression
struct active_samples_per_line_item {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 04 01 05 01 02 00 00 00");
	static constexpr std::string_view name = "Active Samples per Line";
	static constexpr codec::integer<uint16_t> format{};
};

// Annotation Universal Set items (section 7)

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

// Stream decoding

/// A decoded packet of an annotation data stream
using stream_packet = packet<byte_order_item, active_lines_per_frame_item, active_samples_per_line_item, annotation_us>;

/// Decode an annotation data stream: the preface items, the Annotation Universal Sets, and unknown packets. The
/// buffer must outlive the generator.
[[nodiscard]]
inline auto decode_stream(byte_span bytes, decode_options options = {}) -> std::generator<result<stream_packet>> {
	return decode_packets<byte_order_item, active_lines_per_frame_item, active_samples_per_line_item, annotation_us>(
	    bytes,
	    options
	);
}

// Event check

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
