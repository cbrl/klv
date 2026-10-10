/**
 * @file klv/packet/decode.hpp
 * @brief Universal-key packet decoding and checksum verification
 *
 * A packet holds a set with a key, or a standalone item.
 * @code
 * using namespace klv;
 * for (auto const& packet : decode_packets<misb0601::uas_datalink_ls, misb0603::precision_time_stamp_item>(bytes)) {
 *     if (!packet) { report(packet.error()); continue; }
 *     if (auto const* uas = std::get_if<record<misb0601::uas_datalink_ls>>(&*packet)) {
 *         if (auto const& latitude = uas->get<misb0601::sensor_latitude>()) { use(*latitude); }
 *     }
 *     else if (auto const* time = std::get_if<item<misb0603::precision_time_stamp_item>>(&*packet)) {
 *         use(time->value);
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
#include "klv/field.hpp"
#include "klv/packet/item.hpp"
#include "klv/packet/split.hpp"
#include "klv/set/coding.hpp"
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

/// A packet whose key matches no requested definition
struct unknown_packet {
	universal_key key{};
	byte_vector value;

	friend auto operator==(unknown_packet const&, unknown_packet const&) -> bool = default;
};

/// A decoded packet of one of the requested sets or items, or an unknown packet
template<packet_definition... Ps>
using packet = std::variant<packet_value_t<Ps>..., unknown_packet>;

namespace detail {

template<keyed_set_definition S>
[[nodiscard]]
auto verify_checksum(raw_packet const& packet, last_field_info<item_id_t<S>> const& last) -> result<void> {
	using checksum_field = typename S::checksum_field;
	using value_type = checksum_t<S>;
	static_assert(member_of<checksum_field, S>, "The checksum field must be a field of the set");
	static_assert(
	    std::same_as<codec_of<checksum_field>, codec::integer<value_type>>,
	    "The checksum field must use codec::integer of the checksum function result type"
	);

	if (!last.present || !same_id(last.id, id_of<S, checksum_field>())) {
		return make_error(
		    error_code::missing_checksum,
		    packet.offset,
		    std::format("{} at offset {} does not end with a checksum field", S::name, packet.offset)
		);
	}
	if (last.value.size() != sizeof(value_type)) {
		return make_error(
		    error_code::checksum_mismatch,
		    packet.offset,
		    std::format("{} at offset {} has a checksum of {} bytes", S::name, packet.offset, last.value.size())
		);
	}

	// The checksum covers the packet from the first key byte through the length byte of the checksum field
	auto const covered = static_cast<size_t>(last.value.data() - packet.bytes.data());
	auto const computed = compute_checksum<S>(packet.bytes.first(covered));
	auto const stored = read_big_endian<value_type>(last.value);

	if (computed != stored) {
		return make_error(
		    error_code::checksum_mismatch,
		    packet.offset,
		    std::format(
		        "{} at offset {}: checksum is 0x{:0{}X}, computed 0x{:0{}X}",
		        S::name,
		        packet.offset,
		        stored,
		        2 * sizeof(value_type),
		        computed,
		        2 * sizeof(value_type)
		    )
		);
	}
	return {};
}

/// Decode the value of a framed packet of `P`. The key of the packet is the key of `P`.
template<packet_definition P>
[[nodiscard]]
auto decode_framed(raw_packet const& packet, decode_options const& options) -> result<packet_value_t<P>> {
	if constexpr (item_definition<P>) {
		static_cast<void>(options);
		auto value = packet.value.empty() ? field<value_t<P>>{std::unexpect, field_error::empty}
		                                  : P::format.decode(packet.value);
		if (!value) {
			return make_error(
			    error_code::invalid_value,
			    packet.offset,
			    std::format("{} at offset {}: {}", P::name, packet.offset, klv::to_string(value.error()))
			);
		}
		return item<P>{std::move(*value)};
	}
	else {
		auto const value_offset = packet.offset + static_cast<size_t>(packet.value.data() - packet.bytes.data());
		auto last = last_field_info<item_id_t<P>>{};
		auto rec = decode_fields<P>(packet.value, value_offset, options, &last);
		if (!rec) {
			return rec;
		}

		if constexpr (has_checksum<P>) {
			if (options.verify_checksum) {
				if (auto status = verify_checksum<P>(packet, last); !status) {
					return std::unexpected(std::move(status.error()));
				}
			}
		}

		return rec;
	}
}

template<packet_definition... Ps>
[[nodiscard]]
auto dispatch_packet(raw_packet const& framed, decode_options const& options) -> result<packet<Ps...>> {
	auto decoded = std::optional<result<packet<Ps...>>>{};

	auto const try_one = [&]<typename P>(std::type_identity<P>) -> bool {
		if (!key_matches(P::key, framed.key)) {
			return false;
		}

		auto value = decode_framed<P>(framed, options);
		if (value) {
			decoded.emplace(packet<Ps...>{std::in_place_type<packet_value_t<P>>, std::move(*value)});
		}
		else {
			decoded.emplace(std::unexpect, std::move(value.error()));
		}
		return true;
	};

	if ((try_one(std::type_identity<Ps>{}) || ...)) {
		return std::move(*decoded);
	}

	return packet<Ps...>{unknown_packet{
	    .key = framed.key,
	    .value = byte_vector(framed.value.begin(), framed.value.end()),
	}};
}

}  // namespace detail

/**
 * @brief Decode exactly one packet of the set or the standalone item `P`
 *
 * The key must match `P::key` (the version byte is ignored), and the buffer must hold no bytes after the packet. A
 * set gives a `record<P>`, and an item gives an `item<P>`. An item value that its codec rejects gives an error with
 * `error_code::invalid_value`.
 */
template<packet_definition P>
[[nodiscard]]
auto decode_packet(byte_span bytes, decode_options const& options = {}) -> result<packet_value_t<P>> {
	auto const framed = detail::frame_packet(bytes, 0);
	if (!framed) {
		return std::unexpected(framed.error());
	}

	if (!key_matches(P::key, framed->key)) {
		return detail::make_error(
		    error_code::unknown_key,
		    0,
		    std::format("Packet key {} is not the {} key", to_hex(framed->key), P::name)
		);
	}
	if (framed->bytes.size() != bytes.size()) {
		return detail::make_error(
		    error_code::trailing_data,
		    framed->bytes.size(),
		    std::format("{} bytes follow the {} packet", bytes.size() - framed->bytes.size(), P::name)
		);
	}

	return detail::decode_framed<P>(*framed, options);
}

/**
 * @brief Decode all packets in a buffer
 *
 * Each packet becomes a `record` or an `item` of the first definition in `Ps` whose key matches, or an
 * `unknown_packet`. A malformed packet yields one error, and decoding continues at the next universal label. An item
 * value that its codec rejects also yields one error. The buffer must outlive the generator.
 */
template<packet_definition... Ps>
[[nodiscard]]
auto decode_packets(byte_span buffer, decode_options options = {}) -> std::generator<result<packet<Ps...>>> {
	for (auto&& framed : split_packets(buffer)) {
		if (!framed) {
			co_yield std::unexpected(std::move(framed.error()));
			continue;
		}

		co_yield detail::dispatch_packet<Ps...>(*framed, options);
	}
}

}  // namespace klv
