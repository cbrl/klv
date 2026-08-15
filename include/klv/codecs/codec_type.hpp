/**
 * @file klv/codecs/codec_type.hpp
 * @brief The codec interface
 *
 * A codec is a constexpr object that converts one field value between its wire bytes and a plain C++ value. Field
 * descriptors hold a codec as their `format` member, so all format parameters (sizes, ranges, error values) are
 * compile-time constants. A new value kind needs only a new codec type.
 *
 * A codec type has:
 * - `value_type`: the C++ type of a decoded value
 * - `decode(byte_span) -> field<value_type>`
 * - `encode(value_type const&, byte_writer&) -> encode_result`
 * - `to_string(value_type const&) -> std::string`
 * - optionally `encode_error_indicator(byte_writer&) -> encode_result`, for formats with an error value
 *
 * A repeated codec (see `codec::repeated`) also has an `item` codec and an `item_type`. A set can hold its field more
 * than once.
 *
 * A covering codec (see `covering_codec`) also decodes with the tags of the items before the value in its set.
 */

#pragma once

#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/field.hpp"

#include <concepts>
#include <cstdint>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

namespace klv {

/**
 * @brief A codec converts one field value between wire bytes and a C++ value
 *
 * The concept reads the decode result type with `decltype`, which does not need a complete type. This allows a set
 * to hold a list of its own records (MISB ST 1607).
 */
template<typename C>
concept codec_type = requires(C const& codec, byte_span in, typename C::value_type const& value, byte_writer& out) {
	requires std::same_as<decltype(codec.decode(in)), field<typename C::value_type>>;
	{ codec.encode(value, out) } -> std::same_as<encode_result>;
	{ codec.to_string(value) } -> std::same_as<std::string>;
};

/// A codec that can write the error value of its format
template<typename C>
concept has_error_indicator = codec_type<C> && requires(C const& codec, byte_writer& out) {
	{ codec.encode_error_indicator(out) } -> std::same_as<encode_result>;
};

/// A codec of a field that a set can hold more than once. The value is a list with one item for each occurrence.
template<typename C>
concept repeated_codec = codec_type<C> && requires(C const& codec) {
	typename C::item_type;
	requires codec_type<std::remove_cvref_t<decltype(codec.item)>>;
	requires std::same_as<typename C::value_type, std::vector<typename C::item_type>>;
};

/**
 * @brief A codec whose value covers the items before it in its set (for example the MISB ST 1010 SDCC-FLP)
 *
 * Set decoding passes the tags of the items before the value, in decode order. Set encoding writes the items that
 * `covered_tags` names directly before the value.
 */
template<typename C>
concept covering_codec = codec_type<C> && requires(C const& codec, byte_span in, typename C::value_type const& value) {
	requires std::same_as<decltype(codec.decode(in, std::span<uint32_t const>{})), field<typename C::value_type>>;
	{ codec.covered_tags(value) } -> std::same_as<std::span<uint32_t const>>;
};

}  // namespace klv
