/**
 * @file klv/packet/item.hpp
 * @brief Standalone items, and the definitions that a packet can hold
 *
 * A standalone item is one value outside a set: a 16-byte key, a BER length, and the value. Its definition is a field
 * descriptor with a `key`:
 * @code
 * struct precision_time_stamp_item {
 *     static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 03 07 02 01 01 01 05 00 00");
 *     static constexpr std::string_view name = "Precision Time Stamp";
 *     static constexpr codec::timestamp format{};
 * };
 * @endcode
 */

#pragma once

#include "klv/codecs/codec_type.hpp"
#include "klv/set/definition.hpp"
#include "klv/set/record.hpp"

#include <type_traits>

namespace klv {

/// A field descriptor with a 16-byte `key`. A packet can hold it as a standalone item.
template<typename I>
concept item_definition = field_descriptor<I> && has_key<I> && !set_definition<I> && !repeated_codec<codec_of<I>>
                          && !covering_field<I>;

/// The decoded value of the standalone item `I`. Each item has its own type.
template<item_definition I>
struct item {
	using definition = I;

	value_t<I> value{};

	friend auto operator==(item const&, item const&) -> bool = default;
};

/// A set with a key, or a standalone item. A packet can hold it.
template<typename P>
concept packet_definition = keyed_set_definition<P> || item_definition<P>;

namespace detail {

template<typename P>
struct packet_value;

template<keyed_set_definition P>
struct packet_value<P> {
	using type = record<P>;
};

template<item_definition P>
struct packet_value<P> {
	using type = item<P>;
};

}  // namespace detail

/// Decoded type of a packet of `P`: `record<P>` for a set, or `item<P>` for a standalone item
template<packet_definition P>
using packet_value_t = typename detail::packet_value<P>::type;

}  // namespace klv
