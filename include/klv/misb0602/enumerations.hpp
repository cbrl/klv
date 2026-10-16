/**
 * @file klv/misb0602/enumerations.hpp
 * @brief MISB ST 0602 enumerations: Event Indication, Annotation Source, and MIME media types
 */

#pragma once

#include "klv/codecs/string.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

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
	count_,
};

/// Number of media types
inline constexpr auto media_type_count = static_cast<size_t>(media_type::count_);

/// MIME text of a media type, for example "image/svg+xml"
[[nodiscard]]
constexpr auto to_string(media_type value) noexcept -> std::string_view {
	using enum media_type;
	switch (value) {
		case bmp:
			return "image/x-ms-bmp";
		case cgm:
			return "image/cgm";
		case jpeg:
			return "image/jpeg";
		case png:
			return "image/png";
		case svg:
			return "image/svg+xml";
		case count_:
			break;
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
	if (klv::detail::equal_ignoring_case(text, "cgm")) {
		return media_type::cgm;
	}
	for (auto index = size_t{0}; index < media_type_count; ++index) {
		auto const value = static_cast<media_type>(index);
		if (klv::detail::equal_ignoring_case(text, to_string(value))) {
			return value;
		}
	}
	return std::nullopt;
}

}  // namespace klv::misb0602
