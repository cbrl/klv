/**
 * @file klv/codecs/enumeration.hpp
 * @brief Enumeration codec
 */

#pragma once

#include "klv/bytes/big_endian.hpp"
#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/field.hpp"

#include <expected>
#include <format>
#include <string>
#include <string_view>
#include <type_traits>

namespace klv::codec {

/**
 * @brief Integer that holds an enumeration value
 *
 * `name_of` gives the display name of each value. It returns an empty view for a value that the standard does not
 * define, and the codec treats such a value as `field_error::invalid_value`. Decoding accepts 1 to
 * `sizeof(underlying_type)` bytes.
 *
 * @code
 * static constexpr auto format = codec::enumeration<classification>{.name_of = &to_string};
 * @endcode
 */
template<typename Enum>
requires std::is_enum_v<Enum>
struct enumeration {
	using value_type = Enum;
	using underlying_type = std::underlying_type_t<Enum>;
	using name_function = auto (*)(Enum) -> std::string_view;

	name_function name_of = nullptr;

	[[nodiscard]]
	constexpr auto is_defined(Enum value) const -> bool {
		return name_of == nullptr || !name_of(value).empty();
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
		if (name_of != nullptr) {
			if (auto const name = name_of(value); !name.empty()) {
				return std::string{name};
			}
		}
		return std::format("{}", static_cast<underlying_type>(value));
	}
};

}  // namespace klv::codec
