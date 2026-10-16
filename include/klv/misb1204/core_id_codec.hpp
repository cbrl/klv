/**
 * @file klv/misb1204/core_id_codec.hpp
 * @brief MISB ST 1204 Core Identifier binary and text formats, and the standalone KLV item
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codecs/pack.hpp"
#include "klv/codecs/uuid.hpp"
#include "klv/field.hpp"
#include "klv/misb1204/check_value.hpp"
#include "klv/misb1204/core_id.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace klv::misb1204 {

namespace detail {

/// Version of the Core Identifier structure. ST 1204.3 defines only version 1.
inline constexpr uint32_t version = 1;

/// The usage value: the ID types and the IDs that the Core Identifier holds (ST 1204 Table 3)
[[nodiscard]]
constexpr auto usage_value(core_id const& value) -> uint8_t {
	auto bits = 0U;
	if (value.sensor) {
		bits |= (static_cast<unsigned>(value.sensor->type) & 3U) << 5U;
	}
	if (value.platform) {
		bits |= (static_cast<unsigned>(value.platform->type) & 3U) << 3U;
	}
	if (value.window) {
		bits |= 0x04U;
	}
	if (value.minor) {
		bits |= 0x02U;
	}
	return static_cast<uint8_t>(bits);
}

/// Call `f` for each ID of `value`, in the order sensor, platform, window, minor
template<typename F>
constexpr auto for_each_id(core_id const& value, F&& f) -> void {
	if (value.sensor) {
		f(value.sensor->id);
	}
	if (value.platform) {
		f(value.platform->id);
	}
	if (value.window) {
		f(*value.window);
	}
	if (value.minor) {
		f(*value.minor);
	}
}

/// True when `value` follows the EBNF of ST 1204 section 7.1
[[nodiscard]]
constexpr auto is_valid(core_id const& value) -> bool {
	auto const defined = [](std::optional<device_id> const& id) { return !id || !to_string(id->type).empty(); };
	if (!defined(value.sensor) || !defined(value.platform)) {
		return false;
	}
	auto const foundational = value.sensor || value.platform || value.window;
	return foundational != value.minor.has_value();
}

/// Append the hex digits of `bytes` in groups of four, with a dash between two groups
inline auto append_groups(std::string& text, byte_span bytes) -> void {
	constexpr auto digits = std::string_view{"0123456789ABCDEF"};
	for (auto i = size_t{0}; i < bytes.size(); ++i) {
		if (i > 0 && i % 2 == 0) {
			text += '-';
		}
		auto const byte = std::to_integer<size_t>(bytes[i]);
		text += digits[byte >> 4U];
		text += digits[byte & 0x0FU];
	}
}

/// Read hex digits in groups of four, with a dash between two groups. Return false for other text.
[[nodiscard]]
inline auto read_groups(std::string_view text, byte_vector& out) -> bool {
	if (text.size() % 5 != 4) {
		return false;
	}
	for (auto i = size_t{0}; i < text.size(); i += 5) {
		if (i > 0 && text[i - 1] != '-') {
			return false;
		}
		for (auto k = i; k < i + 4; k += 2) {
			auto const byte = klv::detail::hex_byte(text[k], text[k + 1]);
			if (byte < 0) {
				return false;
			}
			out.push_back(static_cast<std::byte>(byte));
		}
	}
	return true;
}

}  // namespace detail

/**
 * @brief Core Identifier binary format (ST 1204 section 7.4.1)
 *
 * The value is a BER-OID version, a usage byte, and a 16-byte UUID for each ID. The display text has the text format
 * of section 7.4.2, for example "0170:F592-F023-...-B2DA/16B7-4341-...-3645:D3".
 *
 * Decoding fails with `field_error::invalid_value` in these cases:
 * - The version is not 1.
 * - The usage byte has a reserved bit set, or it is zero.
 * - A Minor ID comes with another ID.
 *
 * Decoding fails with `field_error::bad_length` when the length does not match the usage byte. Encoding fails with
 * `field_error::invalid_value` for a value that does not follow these rules.
 */
struct core_id_codec {
	using value_type = core_id;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<core_id> {
		auto reader = byte_reader{in};
		auto const version = klv::detail::read_oid(reader);
		if (!version) {
			return std::unexpected(version.error());
		}
		auto const usage = klv::detail::read_exact(reader, 1);
		if (!usage) {
			return std::unexpected(usage.error());
		}
		auto const bits = std::to_integer<unsigned>(usage->front());
		auto const sensor_type = (bits >> 5U) & 3U;
		auto const platform_type = (bits >> 3U) & 3U;
		auto const has_window = (bits & 0x04U) != 0;
		auto const has_minor = (bits & 0x02U) != 0;
		auto const reserved = (bits & 0x81U) != 0;
		auto const foundational = sensor_type != 0 || platform_type != 0 || has_window;
		if (*version != detail::version || reserved || foundational == has_minor) {
			return std::unexpected(field_error::invalid_value);
		}

		// Each ID is one UUID, in the order sensor, platform, window, minor
		auto ids = reader.rest();
		auto const count = (sensor_type != 0 ? 1U : 0U) + (platform_type != 0 ? 1U : 0U) + (has_window ? 1U : 0U)
		                   + (has_minor ? 1U : 0U);
		if (ids.size() != count * std::tuple_size_v<klv::uuid>) {
			return std::unexpected(field_error::bad_length);
		}
		auto const next_id = [&ids] {
			auto id = klv::uuid{};
			std::ranges::copy(ids.first(id.size()), id.begin());
			ids = ids.subspan(id.size());
			return id;
		};

		auto value = core_id{};
		if (sensor_type != 0) {
			value.sensor = device_id{.type = static_cast<id_type>(sensor_type), .id = next_id()};
		}
		if (platform_type != 0) {
			value.platform = device_id{.type = static_cast<id_type>(platform_type), .id = next_id()};
		}
		if (has_window) {
			value.window = next_id();
		}
		if (has_minor) {
			value.minor = next_id();
		}
		return value;
	}

	[[nodiscard]]
	auto encode(core_id const& value, byte_writer& out) const -> encode_result {
		if (!detail::is_valid(value)) {
			return std::unexpected(field_error::invalid_value);
		}
		out.put_ber_oid(detail::version);
		out.put_u8(detail::usage_value(value));
		detail::for_each_id(value, [&out](klv::uuid const& id) { out.put_bytes(id); });
		return {};
	}

	[[nodiscard]]
	auto to_string(core_id const& value) const -> std::string {
		auto const head = std::array{static_cast<std::byte>(detail::version), std::byte{detail::usage_value(value)}};
		auto text = std::string{};
		detail::append_groups(text, head);
		auto separator = ':';
		detail::for_each_id(value, [&](klv::uuid const& id) {
			text += separator;
			detail::append_groups(text, id);
			separator = '/';
		});

		auto const check = std::array{std::byte{check_value(text)}};
		text += ':';
		detail::append_groups(text, check);
		return text;
	}
};

/// Standalone Core Identifier KLV item
struct core_id_item {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 0E 01 04 05 03 00 00 00");
	static constexpr std::string_view name = "Core Identifier";
	static constexpr core_id_codec format{};
};

/**
 * @brief Read a Core Identifier from its text format (ST 1204 section 7.4.2)
 *
 * The text is "VVUU:ID:CC", "VVUU:ID/ID:CC", or "VVUU:ID/ID/ID:CC":
 * - VV is the version, and UU is the usage byte.
 * - Each ID is 8 groups of 4 hex digits, with a dash between two groups.
 * - CC is the check value.
 *
 * The hex digits can be upper or lower case. The result is empty in these cases:
 * - The text has another form.
 * - The check value is wrong.
 * - The binary value is not valid.
 */
[[nodiscard]]
inline auto parse_core_id(std::string_view text) -> std::optional<core_id> {
	// 32 hex digits and 7 dashes
	constexpr auto uuid_text_size = size_t{39};

	auto const first = text.find(':');
	auto const last = text.rfind(':');
	if (first != 4 || last == first || last + 3 != text.size()) {
		return std::nullopt;
	}

	auto bytes = byte_vector{};
	if (!detail::read_groups(text.substr(0, first), bytes)) {
		return std::nullopt;
	}
	auto ids = text.substr(first + 1, last - first - 1);
	for (auto more = true; more;) {
		auto const slash = ids.find('/');
		auto const id = ids.substr(0, slash);
		more = slash != std::string_view::npos;
		if (id.size() != uuid_text_size || !detail::read_groups(id, bytes)) {
			return std::nullopt;
		}
		ids.remove_prefix(more ? slash + 1 : ids.size());
	}

	auto const check = klv::detail::hex_byte(text[last + 1], text[last + 2]);
	if (check < 0 || static_cast<unsigned>(check) != check_value(text.substr(0, last))) {
		return std::nullopt;
	}

	auto value = core_id_codec{}.decode(bytes);
	if (!value) {
		return std::nullopt;
	}
	return *std::move(value);
}

}  // namespace klv::misb1204
