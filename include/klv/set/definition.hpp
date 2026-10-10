/**
 * @file klv/set/definition.hpp
 * @brief Field descriptors and set definitions
 *
 * A field descriptor is a struct with static members. A field of a local set has a BER-OID `tag`. A field of a
 * universal set has a 16-byte `key`. A descriptor with both can be a field of the two kinds of set.
 * @code
 * struct platform_heading_angle {
 *     static constexpr uint32_t tag = 5;
 *     static constexpr std::string_view name = "Platform Heading Angle";
 *     static constexpr std::string_view units = "deg";                     // optional
 *     static constexpr codec::mapped<uint16_t> format{0.0, 360.0};
 * };
 * @endcode
 *
 * A set definition lists its fields. The list order is the encode order.
 * @code
 * struct my_set {
 *     static constexpr std::string_view name = "My Set";
 *     static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 ...");  // optional
 *     using fields = field_list<platform_heading_angle, ...>;
 * };
 * @endcode
 *
 * The set `key` selects the coding of the items, by the SMPTE 336 rule. When the low 3 bits of key byte 6 are 001,
 * the set is a universal set. Each item of a universal set has a 16-byte key. Any other set is a local set, and each
 * of its items has a BER-OID tag.
 *
 * A packet can hold a set with a `key`. A set can also name a `checksum_field`. Then packet decoding verifies the
 * checksum, and packet encoding writes it as the last field. The checksum covers the packet from the first key byte
 * through the length byte of the checksum field. `checksum_function` selects the algorithm:
 * @code
 * using checksum_field = checksum;                         // codec::integer<uint32_t>
 * static constexpr auto checksum_function = crc32_mpeg2;  // without it: the MISB ST 0601 16-bit sum
 * @endcode
 */

#pragma once

#include "klv/bytes/checksum.hpp"
#include "klv/bytes/types.hpp"
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

/// A struct with a BER-OID `tag`, which identifies it in a local set
template<typename T>
concept has_tag = requires {
	{ T::tag } -> std::convertible_to<uint32_t>;
};

/// A struct with a 16-byte universal `key`, which identifies it in a universal set or in a packet
template<typename T>
concept has_key = requires {
	{ T::key } -> std::convertible_to<universal_key>;
};

/// A struct with a display `name`, a codec object `format`, and a BER-OID `tag` or a 16-byte `key`
template<typename E>
concept field_descriptor = requires {
	{ E::name } -> std::convertible_to<std::string_view>;
	E::format;
} && codec_type<std::remove_cvref_t<decltype(E::format)>> && (has_tag<E> || has_key<E>);

/// Codec type of a field
template<field_descriptor E>
using codec_of = std::remove_cvref_t<decltype(E::format)>;

/// Decoded value type of a field
template<field_descriptor E>
using value_t = typename codec_of<E>::value_type;

/// The field, or each item of the repeated field, covers the items before it in the set (see `covering_codec`)
template<typename E>
concept covering_field =
    field_descriptor<E>
    && (covering_codec<codec_of<E>>
        || (repeated_codec<codec_of<E>> && covering_codec<std::remove_cvref_t<decltype(codec_of<E>::item)>>));

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

template<typename List>
struct list_covers : std::false_type {};

template<typename... F>
struct list_covers<field_list<F...>> : std::bool_constant<(covering_field<F> || ...)> {};

template<typename E, typename List>
struct list_contains : std::false_type {};

template<typename E, typename... F>
struct list_contains<E, field_list<F...>> : std::bool_constant<(std::is_same_v<E, F> || ...)> {};

/// The set has a key, and the low 3 bits of key byte 6 are 001 (SMPTE 336)
template<typename S>
[[nodiscard]]
consteval auto universal_coding() -> bool {
	if constexpr (has_key<S>) {
		constexpr auto registry_designator = std::to_integer<unsigned>(universal_key{S::key}[5]);
		return (registry_designator & 0x07U) == 0x01U;
	}
	else {
		return false;
	}
}

}  // namespace detail

/// A struct with a display `name` and a `fields` list
template<typename S>
concept set_definition = requires {
	{ S::name } -> std::convertible_to<std::string_view>;
	typename S::fields;
} && detail::is_field_list<typename S::fields>::value;

/// A set whose items have 16-byte keys (SMPTE 336 universal set)
template<typename S>
concept universal_set_definition = set_definition<S> && detail::universal_coding<S>();

/// A set whose items have BER-OID tags (SMPTE 336 local set)
template<typename S>
concept local_set_definition = set_definition<S> && !detail::universal_coding<S>();

/// A set with a 16-byte `key`. A packet can hold it.
template<typename S>
concept keyed_set_definition = set_definition<S> && has_key<S>;

/// Type of the item ids of the set `S`: `uint32_t` (a tag) or `universal_key`
template<set_definition S>
using item_id_t = std::conditional_t<universal_set_definition<S>, universal_key, uint32_t>;

namespace detail {

/// Id of the field `E` in the set `S`: its tag or its key
template<set_definition S, typename E>
[[nodiscard]]
constexpr auto id_of() -> item_id_t<S> {
	if constexpr (universal_set_definition<S> && has_key<E>) {
		return E::key;
	}
	else if constexpr (local_set_definition<S> && has_tag<E>) {
		return static_cast<uint32_t>(E::tag);
	}
	else {
		return {};
	}
}

/// Two item ids are the same. Two keys are the same when only their version bytes are different.
[[nodiscard]]
constexpr auto same_id(uint32_t first, uint32_t second) noexcept -> bool {
	return first == second;
}

[[nodiscard]]
constexpr auto same_id(universal_key const& first, universal_key const& second) noexcept -> bool {
	return key_matches(first, second);
}

/// Largest tag for which a set uses a dense tag-to-index table
inline constexpr auto max_dense_tag = uint32_t{4'096};

/// Compile-time facts about the item ids of a set
template<typename S, typename List = typename S::fields>
struct set_info;

template<typename S, typename... F>
struct set_info<S, field_list<F...>> {
	static_assert(
	    (field_descriptor<F> && ...),
	    "Each member of a field_list needs a name, a codec format, and a tag or a key"
	);
	static_assert(
	    universal_set_definition<S> ? (has_key<F> && ...) : (has_tag<F> && ...),
	    "Each field of a local set needs a tag, and each field of a universal set needs a key"
	);
	static_assert(
	    !(universal_set_definition<S> && list_covers<field_list<F...>>::value),
	    "Only a local set can hold a covering field (for example an SDCC-FLP)"
	);

	static constexpr auto count = sizeof...(F);
	static constexpr auto ids = std::array<item_id_t<S>, count>{id_of<S, F>()...};

	static constexpr auto unique_ids = [] {
		for (auto i = size_t{0}; i < count; ++i) {
			for (auto j = i + 1; j < count; ++j) {
				if (same_id(ids[i], ids[j])) {
					return false;
				}
			}
		}
		return true;
	}();

	/// Largest tag of a local set, or 0
	static constexpr auto max_tag = [] {
		auto largest = uint32_t{0};
		if constexpr (local_set_definition<S>) {
			for (auto const tag : ids) {
				largest = std::max(largest, tag);
			}
		}
		return largest;
	}();

	static_assert(unique_ids, "Two fields of a set use the same tag, or keys that differ only in the version byte");
	static_assert(max_tag <= max_dense_tag, "A set tag is larger than the dense dispatch table supports");
};

}  // namespace detail

/// A field of the set `S` covers the items before it. Set decoding then keeps the tags of the decoded items.
template<typename S>
concept has_covering_field = local_set_definition<S> && detail::list_covers<typename S::fields>::value;

/// `E` is a field of the set `S`
template<typename E, typename S>
concept member_of = set_definition<S> && detail::list_contains<E, typename S::fields>::value;

/// The set names a `checksum_field`. Packet encoding writes it last, and packet decoding verifies it.
template<typename S>
concept has_checksum = set_definition<S> && requires { typename S::checksum_field; };

namespace detail {

/// Checksum of a packet of `S`: `S::checksum_function`, or the MISB ST 0601 16-bit sum
template<typename S>
requires has_checksum<S>
[[nodiscard]]
constexpr auto compute_checksum(byte_span data) noexcept {
	if constexpr (requires { S::checksum_function(data); }) {
		return S::checksum_function(data);
	}
	else {
		return packet_checksum(data);
	}
}

/// Integer type of the checksum of `S`
template<typename S>
using checksum_t = decltype(compute_checksum<S>(byte_span{}));

}  // namespace detail

}  // namespace klv
