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
 */

#pragma once

#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/field.hpp"

#include <concepts>
#include <string>

namespace klv {

/// A codec converts one field value between wire bytes and a C++ value
template<typename C>
concept codec_type = requires(C const& codec, byte_span in, typename C::value_type const& value, byte_writer& out) {
	{ codec.decode(in) } -> std::same_as<field<typename C::value_type>>;
	{ codec.encode(value, out) } -> std::same_as<encode_result>;
	{ codec.to_string(value) } -> std::same_as<std::string>;
};

/// A codec that can write the error value of its format
template<typename C>
concept has_error_indicator = codec_type<C> && requires(C const& codec, byte_writer& out) {
	{ codec.encode_error_indicator(out) } -> std::same_as<encode_result>;
};

}  // namespace klv
