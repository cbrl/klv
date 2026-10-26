/**
 * @file klv/misb0601/packs/weapons_store.hpp
 * @brief MISB ST 0601 Weapons Stores (tag 140)
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/codecs/pack.hpp"

#include <array>
#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <string_view>

namespace klv::misb0601 {

/// General status of a weapons store
enum class weapon_status : uint8_t {
	off = 0,
	initialization = 1,
	ready_degraded = 2,
	ready_all_up_round = 3,
	launch = 4,
	free_flight = 5,
	abort = 6,
	miss_fire = 7,
	hang_fire = 8,
	jettisoned = 9,
	stepped_over = 10,
	no_status_available = 11,
};

[[nodiscard]]
constexpr auto to_string(weapon_status value) -> std::string_view {
	using enum weapon_status;
	switch (value) {
		case off:
			return "Off";
		case initialization:
			return "Initialization";
		case ready_degraded:
			return "Ready/Degraded";
		case ready_all_up_round:
			return "Ready/All Up Round";
		case launch:
			return "Launch";
		case free_flight:
			return "Free Flight";
		case abort:
			return "Abort";
		case miss_fire:
			return "Miss Fire";
		case hang_fire:
			return "Hang Fire";
		case jettisoned:
			return "Jettisoned";
		case stepped_over:
			return "Stepped Over";
		case no_status_available:
			return "No Status Available";
	}
	return {};
}

/// One weapons store and its status
struct weapons_store {
	// Bits of `engagement_status`
	static constexpr uint8_t fuze_enabled = 0x01;
	static constexpr uint8_t laser_enabled = 0x02;
	static constexpr uint8_t target_enabled = 0x04;
	static constexpr uint8_t weapon_armed = 0x08;

	uint32_t station_id = 0;
	uint32_t hardpoint_id = 0;
	uint32_t carriage_id = 0;
	uint32_t store_id = 0;
	weapon_status general_status = weapon_status::off;  ///< Can hold a value that `weapon_status` does not name
	uint8_t engagement_status = 0;
	std::string weapon_type;

	friend auto operator==(weapons_store const&, weapons_store const&) -> bool = default;
};

/**
 * @brief Weapons store record: four BER-OID IDs, a BER-OID status, then the length-prefixed weapon type
 *
 * The status holds the general status in bits 0 to 7 and the engagement status in bits 8 to 11. ST 0601 adds future
 * status bits in more significant bytes and keeps bits 0 to 11. Thus, decoding ignores the bits above bit 11.
 */
struct weapons_store_codec {
	using value_type = weapons_store;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<weapons_store> {
		auto reader = byte_reader{in};
		auto ids = std::array<uint32_t, 4>{};
		for (auto& id : ids) {
			auto const value = detail::read_oid(reader);
			if (!value) {
				return std::unexpected(value.error());
			}
			id = *value;
		}

		auto const status = detail::read_oid<uint64_t>(reader);
		if (!status) {
			return std::unexpected(status.error());
		}
		auto const type = detail::read_prefixed(reader);
		if (!type) {
			return std::unexpected(type.error());
		}

		return detail::finish(
		    reader,
		    weapons_store{
		        .station_id = ids[0],
		        .hardpoint_id = ids[1],
		        .carriage_id = ids[2],
		        .store_id = ids[3],
		        .general_status = static_cast<weapon_status>(*status & 0xFFU),
		        .engagement_status = static_cast<uint8_t>((*status >> 8U) & 0x0FU),
		        .weapon_type = detail::to_text(*type),
		    }
		);
	}

	[[nodiscard]]
	auto encode(weapons_store const& value, byte_writer& out) const -> encode_result {
		if (value.engagement_status > 0x0F) {
			return std::unexpected(field_error::invalid_value);
		}
		out.put_ber_oid(value.station_id);
		out.put_ber_oid(value.hardpoint_id);
		out.put_ber_oid(value.carriage_id);
		out.put_ber_oid(value.store_id);
		out.put_ber_oid(
		    (static_cast<uint32_t>(value.engagement_status) << 8U) | static_cast<uint32_t>(value.general_status)
		);
		detail::put_prefixed_text(value.weapon_type, out);
		return {};
	}

	[[nodiscard]]
	auto to_string(weapons_store const& value) const -> std::string {
		return std::format(
		    "{{station: {}, hardpoint: {}, carriage: {}, store: {}, status: {}, engagement: 0x{:X}, type: \"{}\"}}",
		    value.station_id,
		    value.hardpoint_id,
		    value.carriage_id,
		    value.store_id,
		    klv::detail::enum_text(value.general_status),
		    value.engagement_status,
		    value.weapon_type
		);
	}
};

}  // namespace klv::misb0601
