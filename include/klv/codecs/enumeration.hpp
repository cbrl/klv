/**
 * @file klv/codecs/enumeration.hpp
 * @brief Enumeration codec
 */

#pragma once

#include "klv/bytes/big_endian.hpp"
#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/field.hpp"

#include <concepts>
#include <expected>
#include <format>
#include <string>
#include <string_view>
#include <type_traits>

namespace klv::detail {

/// The namespace of `Enum` has a `to_string(Enum)` that gives a display name. Argument-dependent lookup finds it.
template<typename Enum>
concept named_enumeration = requires(Enum value) {
	{ to_string(value) } -> std::same_as<std::string_view>;
};

/// Display name of `value`, from the `to_string(Enum)` that argument-dependent lookup finds
template<named_enumeration Enum>
[[nodiscard]]
constexpr auto enumeration_name(Enum value) -> std::string_view {
	return to_string(value);
}

}  // namespace klv::detail

namespace klv::codec {

/**
 * @brief Integer that holds an enumeration value
 *
 * The display name of a value comes from `name_of`. Without `name_of`, it comes from the `to_string(Enum)` function in
 * the namespace of `Enum`. The name function returns an empty view for a value that the standard does not define. The
 * codec treats such a value as `field_error::invalid_value`. Without a name function, each value is valid. Decoding
 * accepts 1 to `sizeof(underlying_type)` bytes.
 *
 * @code
 * static constexpr auto format = codec::enumeration<classification>{};
 * @endcode
 */
template<typename Enum>
requires std::is_enum_v<Enum>
struct enumeration {
	using value_type = Enum;
	using underlying_type = std::underlying_type_t<Enum>;
	using name_function = auto (*)(Enum) -> std::string_view;

	name_function name_of = nullptr;  ///< Name function. Null selects the `to_string(Enum)` of the namespace of `Enum`.

	/// Display name of `value`, or an empty view
	[[nodiscard]]
	constexpr auto name(Enum value) const -> std::string_view {
		if (name_of != nullptr) {
			return name_of(value);
		}
		if constexpr (detail::named_enumeration<Enum>) {
			return detail::enumeration_name(value);
		}
		else {
			return {};
		}
	}

	[[nodiscard]]
	constexpr auto is_defined(Enum value) const -> bool {
		if constexpr (!detail::named_enumeration<Enum>) {
			if (name_of == nullptr) {
				return true;
			}
		}
		return !name(value).empty();
	}

	[[nodiscard]]
	constexpr auto decode(byte_span in) const -> field<Enum> {
		if (in.empty() || in.size() > sizeof(underlying_type)) {
			return std::unexpected(field_error::bad_length);
		}

		auto const value = static_cast<Enum>(read_big_endian<underlying_type>(in));
		if (!is_defined(value)) {
			return std::unexpected(field_error::invalid_value);
		}
		return value;
	}

	[[nodiscard]]
	auto encode(Enum value, byte_writer& out) const -> encode_result {
		if (!is_defined(value)) {
			return std::unexpected(field_error::invalid_value);
		}
		out.put_big_endian(static_cast<underlying_type>(value));
		return {};
	}

	[[nodiscard]]
	auto to_string(Enum value) const -> std::string {
		if (auto const text = name(value); !text.empty()) {
			return std::string{text};
		}
		return std::format("{}", static_cast<underlying_type>(value));
	}
};

}  // namespace klv::codec
