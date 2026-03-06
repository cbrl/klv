/**
 * @file klv/packet/decode.hpp
 * @brief Universal-key packet decoding and checksum verification
 *
 * @code
 * for (auto const& packet : klv::decode_packets<misb0601::uas_datalink_ls>(bytes)) {
 *     if (!packet) { report(packet.error()); continue; }
 *     if (auto const* uas = std::get_if<klv::record<misb0601::uas_datalink_ls>>(&*packet)) {
 *         if (auto const& latitude = uas->get<misb0601::sensor_latitude>()) { use(*latitude); }
 *     }
 * }
 * @endcode
 */

#pragma once

#include "klv/bytes/big_endian.hpp"
#include "klv/bytes/checksum.hpp"
#include "klv/bytes/hex.hpp"
#include "klv/bytes/types.hpp"
#include "klv/bytes/universal_key.hpp"
#include "klv/codecs/integer.hpp"
#include "klv/error.hpp"
#include "klv/packet/split.hpp"
#include "klv/set/decode.hpp"
#include "klv/set/definition.hpp"
#include "klv/set/record.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <generator>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

namespace klv {

/// A packet whose key matches no requested set
struct unknown_packet {
	universal_key key{};
	byte_vector value;

	friend auto operator==(unknown_packet const&, unknown_packet const&) -> bool = default;
};

/// A decoded packet of one of the requested sets, or an unknown packet
template<universal_set_definition... Sets>
using packet = std::variant<record<Sets>..., unknown_packet>;

namespace detail {

template<universal_set_definition S>
[[nodiscard]]
auto verify_checksum(raw_packet const& packet, last_field_info const& last) -> result<void> {
	using checksum_field = typename S::checksum_field;
	static_assert(member_of<checksum_field, S>, "The checksum field must be a field of the set");
	static_assert(
	    std::same_as<codec_of<checksum_field>, codec::integer<uint16_t>>,
	    "The checksum field must use codec::integer<uint16_t>"
	);

	if (!last.present || last.tag != checksum_field::tag) {
		return make_error(
		    error_code::missing_checksum,
		    packet.offset,
		    std::format("{} at offset {} does not end with a checksum field", S::name, packet.offset)
		);
	}
	if (last.value.size() != sizeof(uint16_t)) {
		return make_error(
		    error_code::checksum_mismatch,
		    packet.offset,
		    std::format("{} at offset {} has a checksum of {} bytes", S::name, packet.offset, last.value.size())
		);
	}

	// The checksum covers the packet from the first key byte through the length byte of the checksum field
	auto const covered = static_cast<size_t>(last.value.data() - packet.bytes.data());
	auto const computed = packet_checksum(packet.bytes.first(covered));
	auto const stored = read_big_endian<uint16_t>(last.value);

	if (computed != stored) {
		return make_error(
		    error_code::checksum_mismatch,
		    packet.offset,
		    std::format(
		        "{} at offset {}: checksum is 0x{:04X}, computed 0x{:04X}",
		        S::name,
		        packet.offset,
		        stored,
		        computed
		    )
		);
	}
	return {};
}

template<universal_set_definition S>
[[nodiscard]]
auto decode_framed(raw_packet const& packet, decode_options const& options) -> result<record<S>> {
	auto const value_offset = packet.offset + static_cast<size_t>(packet.value.data() - packet.bytes.data());

	auto last = last_field_info{};
	auto rec = decode_fields<S>(packet.value, value_offset, options, &last);
	if (!rec) {
		return rec;
	}

	if constexpr (has_checksum<S>) {
		if (options.verify_checksum) {
			if (auto status = verify_checksum<S>(packet, last); !status) {
				return std::unexpected(std::move(status.error()));
			}
		}
	}

	return rec;
}

template<universal_set_definition... Sets>
[[nodiscard]]
auto dispatch_packet(raw_packet const& framed, decode_options const& options) -> result<packet<Sets...>> {
	auto decoded = std::optional<result<packet<Sets...>>>{};

	auto const try_set = [&]<typename S>(std::type_identity<S>) -> bool {
		if (!key_matches(S::key, framed.key)) {
			return false;
		}

		auto rec = decode_framed<S>(framed, options);
		if (rec) {
			decoded.emplace(packet<Sets...>{std::in_place_type<record<S>>, std::move(*rec)});
		}
		else {
			decoded.emplace(std::unexpect, std::move(rec.error()));
		}
		return true;
	};

	if ((try_set(std::type_identity<Sets>{}) || ...)) {
		return std::move(*decoded);
	}

	return packet<Sets...>{unknown_packet{
	    .key = framed.key,
	    .value = byte_vector(framed.value.begin(), framed.value.end()),
	}};
}

}  // namespace detail

/**
 * @brief Decode exactly one packet of set `S`
 *
 * The key must match `S::key` (the version byte is ignored), and the buffer must hold no bytes after the packet.
 */
template<universal_set_definition S>
[[nodiscard]]
auto decode_packet(byte_span bytes, decode_options const& options = {}) -> result<record<S>> {
	auto const framed = detail::frame_packet(bytes, 0);
	if (!framed) {
		return std::unexpected(framed.error());
	}

	if (!key_matches(S::key, framed->key)) {
		return detail::make_error(
		    error_code::unknown_key,
		    0,
		    std::format("Packet key {} is not the {} key", to_hex(framed->key), S::name)
		);
	}
	if (framed->bytes.size() != bytes.size()) {
		return detail::make_error(
		    error_code::trailing_data,
		    framed->bytes.size(),
		    std::format("{} bytes follow the {} packet", bytes.size() - framed->bytes.size(), S::name)
		);
	}

	return detail::decode_framed<S>(*framed, options);
}

/**
 * @brief Decode all packets in a buffer
 *
 * Each packet becomes a `record` of the first set in `Sets` whose key matches, or an `unknown_packet`. A malformed
 * packet yields one error, and decoding continues at the next universal label. The buffer must outlive the generator.
 */
template<universal_set_definition... Sets>
[[nodiscard]]
auto decode_packets(byte_span buffer, decode_options options = {}) -> std::generator<result<packet<Sets...>>> {
	for (auto&& framed : split_packets(buffer)) {
		if (!framed) {
			co_yield std::unexpected(std::move(framed.error()));
			continue;
		}

		co_yield detail::dispatch_packet<Sets...>(*framed, options);
	}
}

}  // namespace klv
