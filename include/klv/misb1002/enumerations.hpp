/**
 * @file klv/misb1002/enumerations.hpp
 * @brief MISB ST 1002 enumerations and the Range Image Enumerations pack (tag 12)
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/field.hpp"

#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <string_view>

namespace klv::misb1002 {

/// Compression method of the range image
enum class compression_method : uint8_t {
	none = 0,
	planar_fit = 1,
};

[[nodiscard]]
constexpr auto to_string(compression_method value) -> std::string_view {
	switch (value) {
		case compression_method::none:
			return "None";
		case compression_method::planar_fit:
			return "Planar Fit";
	}
	return {};
}

/// Kind of range values in the image
enum class range_data_type : uint8_t {
	perspective_range_image = 0,  ///< Distance from the perspective center
	depth_range_image = 1,  ///< Distance from the image plane
};

[[nodiscard]]
constexpr auto to_string(range_data_type value) -> std::string_view {
	switch (value) {
		case range_data_type::perspective_range_image:
			return "Perspective Range Image";
		case range_data_type::depth_range_image:
			return "Depth Range Image";
	}
	return {};
}

/// Source of the range image
enum class range_image_source : uint8_t {
	computationally_extracted = 0,
	range_sensor = 1,
};

[[nodiscard]]
constexpr auto to_string(range_image_source value) -> std::string_view {
	switch (value) {
		case range_image_source::computationally_extracted:
			return "Computationally Extracted";
		case range_image_source::range_sensor:
			return "Range Sensor";
	}
	return {};
}

/// Characteristics of the range image
struct range_image_enumerations {
	compression_method compression = compression_method::none;
	range_data_type data_type = range_data_type::perspective_range_image;
	range_image_source source = range_image_source::computationally_extracted;

	friend auto operator==(range_image_enumerations const&, range_image_enumerations const&) -> bool = default;
};

/**
 * @brief Range Image Enumerations pack: one BER-OID byte
 *
 * Bits 2 to 0 hold the compression method, bits 5 to 3 the data type, and bit 6 the source. An undefined value fails
 * with `field_error::invalid_value`.
 */
struct range_image_enumerations_codec {
	using value_type = range_image_enumerations;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<range_image_enumerations> {
		if (in.size() != 1) {
			return std::unexpected(field_error::bad_length);
		}
		auto const bits = std::to_integer<uint8_t>(in.front());
		if ((bits & 0x80U) != 0) {
			return std::unexpected(field_error::invalid_value);
		}
		auto const value = range_image_enumerations{
		    .compression = static_cast<compression_method>(bits & 0x07U),
		    .data_type = static_cast<range_data_type>((bits >> 3U) & 0x07U),
		    .source = static_cast<range_image_source>((bits >> 6U) & 0x01U),
		};
		if (!is_defined(value)) {
			return std::unexpected(field_error::invalid_value);
		}
		return value;
	}

	[[nodiscard]]
	auto encode(range_image_enumerations const& value, byte_writer& out) const -> encode_result {
		if (!is_defined(value)) {
			return std::unexpected(field_error::invalid_value);
		}
		out.put_u8(static_cast<uint8_t>(
		    (static_cast<uint8_t>(value.source) << 6U) | (static_cast<uint8_t>(value.data_type) << 3U)
		    | static_cast<uint8_t>(value.compression)
		));
		return {};
	}

	[[nodiscard]]
	auto to_string(range_image_enumerations const& value) const -> std::string {
		return std::format(
		    "{{compression: {}, data type: {}, source: {}}}",
		    misb1002::to_string(value.compression),
		    misb1002::to_string(value.data_type),
		    misb1002::to_string(value.source)
		);
	}

private:
	[[nodiscard]]
	static constexpr auto is_defined(range_image_enumerations const& value) -> bool {
		return !misb1002::to_string(value.compression).empty() && !misb1002::to_string(value.data_type).empty()
		    && !misb1002::to_string(value.source).empty();
	}
};

}  // namespace klv::misb1002
