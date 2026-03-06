/**
 * @file klv/packet/split.hpp
 * @brief Splitting a buffer into universal-key packets
 */

#pragma once

#include "klv/bytes/byte_reader.hpp"
#include "klv/bytes/hex.hpp"
#include "klv/bytes/types.hpp"
#include "klv/bytes/universal_key.hpp"
#include "klv/error.hpp"

#include <algorithm>
#include <cstddef>
#include <expected>
#include <format>
#include <generator>
#include <utility>

namespace klv {

/// One framed packet: key, value, and position. The spans point into the split buffer.
struct raw_packet {
	universal_key key{};
	byte_span value;  ///< Value bytes
	byte_span bytes;  ///< Whole packet: key, BER length, and value
	size_t offset = 0;  ///< Offset of the packet in the split buffer
};

namespace detail {

/// Frame one packet at the start of `data`
[[nodiscard]]
inline auto frame_packet(byte_span data, size_t base_offset) -> result<raw_packet> {
	auto reader = byte_reader{data, base_offset};

	auto const key = reader.read_universal_key();
	if (!key) {
		return std::unexpected(key.error());
	}
	if (!std::ranges::equal(byte_span{*key}.first(universal_label_prefix.size()), universal_label_prefix)) {
		return make_error(
		    error_code::unknown_key,
		    base_offset,
		    std::format("No universal label at offset {} (found {})", base_offset, to_hex(*key))
		);
	}

	auto const length = reader.read_ber_length();
	if (!length) {
		return std::unexpected(length.error());
	}

	auto const value = reader.take(*length);
	if (!value) {
		return std::unexpected(value.error());
	}

	return raw_packet{
	    .key = *key,
	    .value = *value,
	    .bytes = data.first(reader.position()),
	    .offset = base_offset,
	};
}

/// Offset of the next universal label prefix at or after `from`, or `data.size()`
[[nodiscard]]
inline auto find_next_label(byte_span data, size_t from) -> size_t {
	if (from >= data.size()) {
		return data.size();
	}

	auto const rest = data.subspan(from);
	auto const found = std::ranges::search(rest, universal_label_prefix);
	if (found.empty()) {
		return data.size();
	}
	return from + static_cast<size_t>(found.begin() - rest.begin());
}

}  // namespace detail

/**
 * @brief Split a buffer into universal-key packets without decoding them
 *
 * A packet that does not start with a universal label, or that does not fit in the buffer, yields one error. The
 * splitter then continues at the next universal label prefix (06 0E 2B 34).
 */
[[nodiscard]]
inline auto split_packets(byte_span buffer) -> std::generator<result<raw_packet>> {
	auto offset = size_t{0};

	while (offset < buffer.size()) {
		auto framed = detail::frame_packet(buffer.subspan(offset), offset);
		if (!framed) {
			co_yield std::unexpected(std::move(framed.error()));
			offset = detail::find_next_label(buffer, offset + 1);
			continue;
		}

		offset += framed->bytes.size();
		co_yield std::move(*framed);
	}
}

}  // namespace klv
