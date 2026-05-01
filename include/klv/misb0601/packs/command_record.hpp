/**
 * @file klv/misb0601/packs/command_record.hpp
 * @brief MISB ST 0601 Control Command (tag 115)
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/codecs/pack.hpp"

#include <cstdint>
#include <expected>
#include <format>
#include <optional>
#include <string>

namespace klv::misb0601 {

/// One command from the ground control station to the aircraft
struct command_record {
	uint32_t id = 0;
	std::string text;
	std::optional<timestamp> time;

	friend auto operator==(command_record const&, command_record const&) -> bool = default;
};

/// Control Command: BER-OID ID, length-prefixed UTF-8 text, then an optional 8-byte time stamp
struct command_record_codec {
	using value_type = command_record;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<command_record> {
		auto reader = byte_reader{in};
		auto const id = detail::read_oid(reader);
		if (!id) {
			return std::unexpected(id.error());
		}
		auto const text = detail::read_prefixed(reader);
		if (!text) {
			return std::unexpected(text.error());
		}

		auto result = command_record{.id = *id, .text = detail::to_text(*text)};
		if (!reader.empty()) {
			auto const time = codec::timestamp{}.decode(reader.rest());
			if (!time) {
				return std::unexpected(time.error());
			}
			result.time = *time;
		}
		return result;
	}

	[[nodiscard]]
	auto encode(command_record const& value, byte_writer& out) const -> encode_result {
		out.put_ber_oid(value.id);
		detail::put_prefixed_text(value.text, out);
		return value.time ? codec::timestamp{}.encode(*value.time, out) : encode_result{};
	}

	[[nodiscard]]
	auto to_string(command_record const& value) const -> std::string {
		auto text = std::format("{{id: {}, text: \"{}\"", value.id, value.text);
		if (value.time) {
			text += std::format(", time: {}", codec::timestamp{}.to_string(*value.time));
		}
		return text + "}";
	}
};

}  // namespace klv::misb0601
