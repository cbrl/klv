/**
 * @file klv/set/definition.hpp
 * @brief Field descriptors and set definitions
 *
 * A field descriptor is a struct with static members:
 * @code
 * struct platform_heading_angle {
 *     static constexpr uint32_t tag = 5;
 *     static constexpr std::string_view name = "Platform Heading Angle";
 *     static constexpr std::string_view units = "deg";                     // optional
 *     static constexpr codec::mapped<uint16_t> format{0.0, 360.0};
 * };
 * @endcode
 *
 * A local set definition lists its fields. The list order is the encode order.
 * @code
 * struct my_set {
 *     static constexpr std::string_view name = "My Set";
 *     using fields = field_list<platform_heading_angle, ...>;
 * };
 * @endcode
 *
 * A universal set definition is a local set definition with a 16-byte `key`. When it also names a `checksum_field`,
 * packet decoding verifies the MISB ST 0601 checksum and packet encoding writes it as the last field.
 */

#pragma once

#include "klv/bytes/universal_key.hpp"
#include "klv/codecs/codec_type.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace klv {

/// Compile-time list of the field descriptors of a set
template<typename... Fields>
struct field_list {
	static constexpr size_t size = sizeof...(Fields);
};

/// A struct with a BER-OID `tag`, a display `name`, and a codec object `format`
template<typename E>
concept field_descriptor = requires {
	{ E::tag } -> std::convertible_to<uint32_t>;
	{ E::name } -> std::convertible_to<std::string_view>;
	E::format;
} && codec_type<std::remove_cvref_t<decltype(E::format)>>;

/// Codec type of a field
template<field_descriptor E>
using codec_of = std::remove_cvref_t<decltype(E::format)>;

/// Decoded value type of a field
template<field_descriptor E>
using value_t = typename codec_of<E>::value_type;

/// Units of a field, or an empty view when the descriptor has no `units` member
template<field_descriptor E>
[[nodiscard]]
constexpr auto units_of() noexcept -> std::string_view {
	if constexpr (requires {
		              { E::units } -> std::convertible_to<std::string_view>;
	              }) {
		return E::units;
	}
	else {
		return {};
	}
}

namespace detail {

template<typename T>
struct is_field_list : std::false_type {};

template<typename... F>
struct is_field_list<field_list<F...>> : std::true_type {};

template<typename E, typename List>
struct list_contains : std::false_type {};

template<typename E, typename... F>
struct list_contains<E, field_list<F...>> : std::bool_constant<(std::same_as<E, F> || ...)> {};

/// Largest tag for which a set uses a dense tag-to-index table
inline constexpr auto max_dense_tag = uint32_t{4'096};

/// Compile-time facts about the tags of a field list
template<typename List>
struct list_info;

template<typename... F>
struct list_info<field_list<F...>> {
	static_assert((field_descriptor<F> && ...), "Each member of a field_list needs a tag, a name, and a codec format");

	static constexpr auto count = sizeof...(F);
	static constexpr auto tags = std::array<uint32_t, count>{static_cast<uint32_t>(F::tag)...};

	static constexpr auto max_tag = [] {
		auto largest = uint32_t{0};
		for (auto const tag : tags) {
			largest = std::max(largest, tag);
		}
		return largest;
	}();

	static constexpr auto unique_tags = [] {
		for (auto i = size_t{0}; i < count; ++i) {
			for (auto j = i + 1; j < count; ++j) {
				if (tags[i] == tags[j]) {
					return false;
				}
			}
		}
		return true;
	}();

	static_assert(unique_tags, "Two fields of a set use the same tag");
	static_assert(max_tag <= max_dense_tag, "A set tag is larger than the dense dispatch table supports");
};

}  // namespace detail

/// A struct with a display `name` and a `fields` list
template<typename S>
concept local_set_definition = requires {
	{ S::name } -> std::convertible_to<std::string_view>;
	typename S::fields;
} && detail::is_field_list<typename S::fields>::value;

/// `E` is a field of the set `S`
template<typename E, typename S>
concept member_of = local_set_definition<S> && detail::list_contains<E, typename S::fields>::value;

/// The set names a `checksum_field`. Packet encoding writes it last, and packet decoding verifies it.
template<typename S>
concept has_checksum = local_set_definition<S> && requires { typename S::checksum_field; };

/// A local set definition with a 16-byte universal `key`
template<typename S>
concept universal_set_definition = local_set_definition<S> && requires {
	{ S::key } -> std::convertible_to<universal_key>;
};

}  // namespace klv
