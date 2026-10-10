/**
 * @file klv/packet/encode.hpp
 * @brief Universal-key packet encoding: a set with a key, or a standalone item
 */

#pragma once

#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/checksum.hpp"
#include "klv/bytes/types.hpp"
#include "klv/error.hpp"
#include "klv/field.hpp"
#include "klv/packet/item.hpp"
#include "klv/set/coding.hpp"
#include "klv/set/definition.hpp"
#include "klv/set/encode.hpp"
#include "klv/set/record.hpp"

#include <cstddef>
#include <cstdint>
#include <format>
#include <utility>

namespace klv {

/**
 * @brief Append one packet of set `S` to `out`
 *
 * The packet has the key of `S`, a BER length, and the fields in definition order. When `S` names a checksum field,
 * the function ignores any value in that field. It writes a computed checksum as the last field (see
 * `checksum_function`). On failure, `out` keeps its old size.
 */
template<keyed_set_definition S>
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
			detail::put_id<S>(writer, detail::id_of<S, typename S::checksum_field>());
			writer.put_u8(static_cast<uint8_t>(sizeof(detail::checksum_t<S>)));
			writer.put_big_endian(detail::checksum_t<S>{0});
		}
		return {};
	});

	if (!status) {
		writer.truncate(start);
		return status;
	}

	if constexpr (has_checksum<S>) {
		constexpr auto size = sizeof(detail::checksum_t<S>);
		auto const bytes = byte_span{out}.subspan(start);
		auto checksum = detail::compute_checksum<S>(bytes.first(bytes.size() - size));
		for (auto i = size_t{1}; i <= size; ++i) {
			out[out.size() - i] = static_cast<std::byte>(checksum & 0xFFU);
			checksum = static_cast<detail::checksum_t<S>>(checksum >> 8U);
		}
	}

	return {};
}

/// Encode one packet of set `S`
template<keyed_set_definition S>
[[nodiscard]]
auto encode_packet(record<S> const& rec) -> result<byte_vector> {
	auto out = byte_vector{};
	return encode_packet(rec, out).transform([&] { return std::move(out); });
}

/**
 * @brief Append one standalone item `I` to `out`
 *
 * The packet has the key of `I`, a BER length, and the value. On failure, `out` keeps its old size.
 */
template<item_definition I>
[[nodiscard]]
auto encode_packet(item<I> const& value, byte_vector& out) -> result<void> {
	auto writer = byte_writer{out};
	auto const start = writer.size();

	writer.put_bytes(I::key);
	auto const status = writer.put_length_prefixed([&] { return I::format.encode(value.value, writer); });
	if (!status) {
		writer.truncate(start);
		return detail::make_error(
		    error_code::invalid_value,
		    start,
		    std::format("Cannot encode item '{}': {}", I::name, klv::to_string(status.error()))
		);
	}
	return {};
}

/// Encode one standalone item `I`
template<item_definition I>
[[nodiscard]]
auto encode_packet(item<I> const& value) -> result<byte_vector> {
	auto out = byte_vector{};
	return encode_packet(value, out).transform([&] { return std::move(out); });
}

}  // namespace klv
