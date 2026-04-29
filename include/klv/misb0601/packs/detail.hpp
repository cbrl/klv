/**
 * @file klv/misb0601/packs/detail.hpp
 * @brief Read and write helpers that the MISB ST 0601 pack codecs share
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/field.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>

namespace klv::misb0601 {

namespace detail {

[[nodiscard]]
inline auto read_oid(byte_reader& reader) -> field<uint32_t> {
	auto const value = reader.read_ber_oid();
	if (!value) {
		return std::unexpected(field_error::invalid_value);
	}
	return *value;
}

[[nodiscard]]
inline auto read_exact(byte_reader& reader, size_t count) -> field<byte_span> {
	auto const bytes = reader.take(count);
	if (!bytes) {
		return std::unexpected(field_error::bad_length);
	}
	return *bytes;
}

/// Read a BER length and the bytes that it counts
[[nodiscard]]
inline auto read_prefixed(byte_reader& reader) -> field<byte_span> {
	auto const length = reader.read_ber_length();
	if (!length) {
		return std::unexpected(field_error::invalid_value);
	}
	return read_exact(reader, *length);
}

[[nodiscard]]
inline auto to_text(byte_span bytes) -> std::string {
	return std::string(reinterpret_cast<char const*>(bytes.data()), bytes.size());
}

inline auto put_text(std::string_view text, byte_writer& out) -> void {
	out.put_bytes(std::as_bytes(std::span{text}));
}

/// Write a BER length and the text
inline auto put_prefixed_text(std::string_view text, byte_writer& out) -> void {
	out.put_ber_length(text.size());
	put_text(text, out);
}

/// Fail when bytes remain after the last element of a pack
template<typename T>
[[nodiscard]]
auto finish(byte_reader const& reader, T value) -> field<T> {
	if (!reader.empty()) {
		return std::unexpected(field_error::invalid_value);
	}
	return value;
}

}  // namespace detail

}  // namespace klv::misb0601
