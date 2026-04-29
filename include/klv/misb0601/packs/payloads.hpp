/**
 * @file klv/misb0601/packs/payloads.hpp
 * @brief MISB ST 0601 Payload List (tag 138)
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/misb0601/packs/detail.hpp"

#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace klv::misb0601 {

/// Kind of a payload
enum class payload_type : uint32_t {
	electro_optical = 0,
	lidar = 1,
	radar = 2,
	sigint = 3,
};

[[nodiscard]]
constexpr auto to_string(payload_type value) -> std::string_view {
	using enum payload_type;
	switch (value) {
		case electro_optical:
			return "Electro Optical MI Sensor";
		case lidar:
			return "LIDAR";
		case radar:
			return "RADAR";
		case sigint:
			return "SIGINT";
	}
	return {};
}

/// One payload of the platform
struct payload_record {
	uint32_t id = 0;
	payload_type type = payload_type::electro_optical;  ///< Can hold a value that `payload_type` does not name
	std::string name;

	friend auto operator==(payload_record const&, payload_record const&) -> bool = default;
};

/// Payload record: BER-OID ID, BER-OID type, then the length-prefixed name
struct payload_record_codec {
	using value_type = payload_record;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<payload_record> {
		auto reader = byte_reader{in};
		auto const id = detail::read_oid(reader);
		if (!id) {
			return std::unexpected(id.error());
		}
		auto const type = detail::read_oid(reader);
		if (!type) {
			return std::unexpected(type.error());
		}
		auto const name = detail::read_prefixed(reader);
		if (!name) {
			return std::unexpected(name.error());
		}

		return detail::finish(
		    reader,
		    payload_record{.id = *id, .type = static_cast<payload_type>(*type), .name = detail::to_text(*name)}
		);
	}

	[[nodiscard]]
	auto encode(payload_record const& value, byte_writer& out) const -> encode_result {
		out.put_ber_oid(value.id);
		out.put_ber_oid(static_cast<uint32_t>(value.type));
		detail::put_prefixed_text(value.name, out);
		return {};
	}

	[[nodiscard]]
	auto to_string(payload_record const& value) const -> std::string {
		auto const type = misb0601::to_string(value.type);
		return std::format(
		    "{{id: {}, type: {}, name: \"{}\"}}",
		    value.id,
		    type.empty() ? std::format("{}", static_cast<uint32_t>(value.type)) : std::string{type},
		    value.name
		);
	}
};

/// Payloads of the platform
struct payloads {
	uint32_t count = 0;  ///< Number of payloads on the platform
	std::vector<payload_record> records;

	friend auto operator==(payloads const&, payloads const&) -> bool = default;
};

/// Payload List: BER-OID payload count, then a series of payload records
struct payloads_codec {
	using value_type = payloads;

	static constexpr codec::series<payload_record_codec> records_format{};

	[[nodiscard]]
	auto decode(byte_span in) const -> field<payloads> {
		auto reader = byte_reader{in};
		auto const count = detail::read_oid(reader);
		if (!count) {
			return std::unexpected(count.error());
		}
		return records_format.decode(reader.rest()).transform([&](std::vector<payload_record> records) {
			return payloads{.count = *count, .records = std::move(records)};
		});
	}

	[[nodiscard]]
	auto encode(payloads const& value, byte_writer& out) const -> encode_result {
		out.put_ber_oid(value.count);
		return records_format.encode(value.records, out);
	}

	[[nodiscard]]
	auto to_string(payloads const& value) const -> std::string {
		return std::format("{{count: {}, payloads: {}}}", value.count, records_format.to_string(value.records));
	}
};

}  // namespace klv::misb0601
