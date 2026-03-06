/**
 * @file klv/packet/encode.hpp
 * @brief Universal-key packet encoding
 */

#pragma once

#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/checksum.hpp"
#include "klv/bytes/types.hpp"
#include "klv/error.hpp"
#include "klv/set/definition.hpp"
#include "klv/set/encode.hpp"
#include "klv/set/record.hpp"

#include <cstdint>
#include <utility>

namespace klv {

/**
 * @brief Append one packet of set `S` to `out`
 *
 * The packet has the key of `S`, a BER length, and the fields in definition order. When `S` names a checksum field,
 * the function ignores any value in that field and writes a computed checksum as the last field. On failure, `out`
 * keeps its old size.
 */
template<universal_set_definition S>
[[nodiscard]]
auto encode_packet(record<S> const& rec, byte_vector& out) -> result<void> {
	auto writer = byte_writer{out};
	auto const start = writer.size();

	writer.put_bytes(S::key);
	auto status = writer.put_length_prefixed([&]() -> result<void> {
		if (auto fields = detail::encode_fields(rec, writer); !fields) {
			return fields;
		}

		if constexpr (has_checksum<S>) {
			// Placeholder value. The checksum covers the final BER length, so it is computed after the value is done.
			writer.put_ber_oid(S::checksum_field::tag);
			writer.put_u8(static_cast<uint8_t>(sizeof(uint16_t)));
			writer.put_big_endian(uint16_t{0});
		}
		return {};
	});

	if (!status) {
		writer.truncate(start);
		return status;
	}

	if constexpr (has_checksum<S>) {
		auto const bytes = byte_span{out}.subspan(start);
		auto const checksum = packet_checksum(bytes.first(bytes.size() - sizeof(uint16_t)));
		out[out.size() - 2] = static_cast<std::byte>(checksum >> 8U);
		out[out.size() - 1] = static_cast<std::byte>(checksum & 0xFFU);
	}

	return {};
}

/// Encode one packet of set `S`
template<universal_set_definition S>
[[nodiscard]]
auto encode_packet(record<S> const& rec) -> result<byte_vector> {
	auto out = byte_vector{};
	return encode_packet(rec, out).transform([&] { return std::move(out); });
}

}  // namespace klv
