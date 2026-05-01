/**
 * @file klv/misb0601/packs/waypoint_record.hpp
 * @brief MISB ST 0601 Waypoint List (tag 141)
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/codecs/pack.hpp"
#include "klv/misb0601/packs/location.hpp"

#include <cstdint>
#include <expected>
#include <format>
#include <optional>
#include <string>

namespace klv::misb0601 {

/// One navigation waypoint
struct waypoint_record {
	// Bits of `info`
	static constexpr uint8_t manual_mode = 0x01;  ///< Set: manual. Clear: automated.
	static constexpr uint8_t ad_hoc_source = 0x02;  ///< Set: ad hoc. Clear: pre-planned.

	uint32_t id = 0;
	int16_t order = 0;  ///< Prosecution order
	std::optional<uint8_t> info;
	std::optional<location> position;  ///< Needs `info`

	friend auto operator==(waypoint_record const&, waypoint_record const&) -> bool = default;
};

/// Waypoint record: BER-OID ID, 2-byte signed order, then an optional 1-byte info and an optional location pack
struct waypoint_record_codec {
	using value_type = waypoint_record;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<waypoint_record> {
		auto reader = byte_reader{in};
		auto const id = detail::read_oid(reader);
		if (!id) {
			return std::unexpected(id.error());
		}
		auto const order = detail::read_exact(reader, sizeof(int16_t));
		if (!order) {
			return std::unexpected(order.error());
		}

		auto result = waypoint_record{.id = *id, .order = read_big_endian<int16_t>(*order)};
		if (reader.empty()) {
			return result;
		}

		auto const info = detail::read_oid(reader);
		if (!info) {
			return std::unexpected(info.error());
		}
		if (*info > 0x7F) {
			return std::unexpected(field_error::invalid_value);
		}
		result.info = static_cast<uint8_t>(*info);
		if (reader.empty()) {
			return result;
		}

		auto const position = location_codec{}.decode(reader.rest());
		if (!position) {
			return std::unexpected(position.error());
		}
		result.position = *position;
		return result;
	}

	[[nodiscard]]
	auto encode(waypoint_record const& value, byte_writer& out) const -> encode_result {
		if ((value.info && *value.info > 0x7F) || (value.position && !value.info)) {
			return std::unexpected(field_error::invalid_value);
		}

		out.put_ber_oid(value.id);
		out.put_big_endian(value.order);
		if (value.info) {
			out.put_u8(*value.info);
		}
		return value.position ? location_codec{}.encode(*value.position, out) : encode_result{};
	}

	[[nodiscard]]
	auto to_string(waypoint_record const& value) const -> std::string {
		auto text = std::format("{{id: {}, order: {}", value.id, value.order);
		if (value.info) {
			text += std::format(", info: 0x{:X}", *value.info);
		}
		if (value.position) {
			text += std::format(", position: {}", location_codec{}.to_string(*value.position));
		}
		return text + "}";
	}
};

}  // namespace klv::misb0601
