/**
 * @file klv/field.hpp
 * @brief The value of one field, the reason that a field has no value, and the result of a codec encode operation
 */

#pragma once

#include <cstdint>
#include <expected>
#include <string_view>

namespace klv {

/// Why a field has no value
enum class field_error : uint8_t {
	absent,  ///< The set does not contain the field
	error_indicator,  ///< The field holds the error ("out of range") value of its format
	bad_length,  ///< The value length does not match the field format
	out_of_range,  ///< The value is outside the range of the field format
	invalid_value,  ///< The value is not valid for the field format
};

[[nodiscard]]
constexpr auto to_string(field_error error) noexcept -> std::string_view {
	switch (error) {
		case field_error::absent:
			return "absent";
		case field_error::error_indicator:
			return "error indicator";
		case field_error::bad_length:
			return "bad length";
		case field_error::out_of_range:
			return "out of range";
		case field_error::invalid_value:
			return "invalid value";
	}
	return "unknown";
}

/// The value of one field, or the reason that it has no value
template<typename T>
using field = std::expected<T, field_error>;

/// Result of a codec encode operation
using encode_result = std::expected<void, field_error>;

}  // namespace klv
