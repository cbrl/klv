/**
 * @file klv/error.hpp
 * @brief Errors and the result type of packet and set operations
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <stdexcept>
#include <string>
#include <utility>

namespace klv {

/// Error codes for packet and set decoding and encoding
enum class error_code : uint8_t {
	unexpected_end_of_data,
	invalid_ber_length,
	invalid_ber_oid,
	unknown_key,
	checksum_mismatch,
	missing_checksum,
	trailing_data,
	invalid_value,
};

/// Detailed error information
struct error {
	error_code code;
	std::string message;
	size_t offset = 0;  ///< Byte offset of the problem in the decoded buffer
};

/// Result type for fallible operations
template<typename T>
using result = std::expected<T, error>;

namespace detail {

[[nodiscard]]
inline auto make_error(error_code code, size_t offset, std::string message) -> std::unexpected<error> {
	return std::unexpected(error{.code = code, .message = std::move(message), .offset = offset});
}

/// Stop constant evaluation with a message. A descriptor with invalid constexpr parameters does not compile.
constexpr auto require(bool condition, char const* message) -> void {
	if (!condition) {
		throw std::invalid_argument(message);
	}
}

}  // namespace detail

}  // namespace klv
