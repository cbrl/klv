/**
 * @file klv/codecs/string.hpp
 * @brief Character string codecs: 8-bit text and UTF-16 text
 */

#pragma once

#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/field.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace klv::detail {

/// Append the UTF-8 bytes of the code point `code` to `text`
inline auto append_utf8(std::string& text, char32_t code) -> void {
	auto const put = [&](uint32_t bits) { text += static_cast<char>(static_cast<unsigned char>(bits)); };
	auto const value = static_cast<uint32_t>(code);

	if (value < 0x80U) {
		put(value);
	}
	else if (value < 0x800U) {
		put(0xC0U | (value >> 6U));
		put(0x80U | (value & 0x3FU));
	}
	else if (value < 0x1'0000U) {
		put(0xE0U | (value >> 12U));
		put(0x80U | ((value >> 6U) & 0x3FU));
		put(0x80U | (value & 0x3FU));
	}
	else {
		put(0xF0U | (value >> 18U));
		put(0x80U | ((value >> 12U) & 0x3FU));
		put(0x80U | ((value >> 6U) & 0x3FU));
		put(0x80U | (value & 0x3FU));
	}
}

/**
 * @brief Read one code point of UTF-8 text at `position`, and move `position` after it
 *
 * An overlong form, a surrogate, a value above U+10FFFF, or a truncated sequence gives no value.
 */
[[nodiscard]]
inline auto read_utf8(std::string_view text, size_t& position) -> std::optional<char32_t> {
	auto const lead = static_cast<uint32_t>(static_cast<unsigned char>(text[position]));
	if (lead < 0x80U) {
		++position;
		return static_cast<char32_t>(lead);
	}

	auto count = size_t{0};
	auto value = uint32_t{0};
	auto minimum = uint32_t{0};
	if ((lead & 0xE0U) == 0xC0U) {
		count = 1;
		value = lead & 0x1FU;
		minimum = 0x80U;
	}
	else if ((lead & 0xF0U) == 0xE0U) {
		count = 2;
		value = lead & 0x0FU;
		minimum = 0x800U;
	}
	else if ((lead & 0xF8U) == 0xF0U) {
		count = 3;
		value = lead & 0x07U;
		minimum = 0x1'0000U;
	}
	else {
		return std::nullopt;
	}

	if (count >= text.size() - position) {
		return std::nullopt;
	}
	for (auto i = size_t{1}; i <= count; ++i) {
		auto const next = static_cast<uint32_t>(static_cast<unsigned char>(text[position + i]));
		if ((next & 0xC0U) != 0x80U) {
			return std::nullopt;
		}
		value = (value << 6U) | (next & 0x3FU);
	}

	if (value < minimum || value > 0x10'FFFFU || (value >= 0xD800U && value <= 0xDFFFU)) {
		return std::nullopt;
	}
	position += count + 1;
	return static_cast<char32_t>(value);
}

}  // namespace klv::detail

namespace klv::codec {

/**
 * @brief Character string (ISO 646 or UTF-8)
 *
 * Decoding removes trailing NUL bytes. Encoding fails when the value is longer than `max_length` bytes.
 */
struct string {
	using value_type = std::string;

	size_t max_length = std::numeric_limits<size_t>::max();

	[[nodiscard]]
	auto decode(byte_span in) const -> field<std::string> {
		auto size = in.size();
		while (size > 0 && in[size - 1] == std::byte{0}) {
			--size;
		}
		return std::string(reinterpret_cast<char const*>(in.data()), size);
	}

	[[nodiscard]]
	auto encode(std::string const& value, byte_writer& out) const -> encode_result {
		if (value.size() > max_length) {
			return std::unexpected(field_error::bad_length);
		}
		out.put_bytes(std::as_bytes(std::span{value}));
		return {};
	}

	[[nodiscard]]
	auto to_string(std::string const& value) const -> std::string {
		return value;
	}
};

/**
 * @brief UTF-16 character string (RFC 2781). The value is UTF-8 text.
 *
 * Decoding reads the byte order from a byte order mark (FE FF or FF FE). Without a mark, the byte order is big-endian.
 * Decoding removes trailing U+0000 characters. An odd length or an unpaired surrogate gives
 * `field_error::invalid_value`. Encoding writes UTF-16BE without a byte order mark. Text that is not valid UTF-8 gives
 * `field_error::invalid_value`.
 *
 * Some producers send 8-bit text in a UTF-16 field. With `legacy_8bit`, a value without a byte order mark can decode
 * as 8-bit text (see `codec::string`). This occurs when the value has an odd length, or when a high byte is not zero.
 * This rule is correct for ASCII text, for example country codes. In UTF-16BE, the high byte of each ASCII character
 * is zero.
 */
struct utf16_string {
	using value_type = std::string;

	bool legacy_8bit = false;  ///< Also decode 8-bit text (see above)

	[[nodiscard]]
	auto decode(byte_span in) const -> field<std::string> {
		auto const mark = in.size() >= 2 ? (std::to_integer<uint32_t>(in[0]) << 8U) | std::to_integer<uint32_t>(in[1])
		                                 : uint32_t{0};
		auto const has_mark = mark == 0xFEFFU || mark == 0xFFFEU;
		auto const little_endian = mark == 0xFFFEU;

		if (legacy_8bit && !has_mark && !zero_high_bytes(in)) {
			return string{}.decode(in);
		}

		auto const units = has_mark ? in.subspan(2) : in;
		if (units.size() % 2 != 0) {
			return std::unexpected(field_error::invalid_value);
		}

		auto const unit_at = [&](size_t index) {
			auto const first = std::to_integer<uint32_t>(units[index]);
			auto const second = std::to_integer<uint32_t>(units[index + 1]);
			return little_endian ? (second << 8U) | first : (first << 8U) | second;
		};

		auto text = std::string{};
		for (auto i = size_t{0}; i < units.size(); i += 2) {
			auto const unit = unit_at(i);
			if (unit >= 0xDC00U && unit <= 0xDFFFU) {
				return std::unexpected(field_error::invalid_value);
			}
			if (unit < 0xD800U || unit > 0xDBFFU) {
				detail::append_utf8(text, static_cast<char32_t>(unit));
				continue;
			}

			// A high surrogate needs a low surrogate after it
			if (i + 2 >= units.size()) {
				return std::unexpected(field_error::invalid_value);
			}
			auto const low = unit_at(i + 2);
			if (low < 0xDC00U || low > 0xDFFFU) {
				return std::unexpected(field_error::invalid_value);
			}
			detail::append_utf8(text, static_cast<char32_t>(0x1'0000U + ((unit - 0xD800U) << 10U) + (low - 0xDC00U)));
			i += 2;
		}

		// In UTF-8, only U+0000 has a zero byte
		while (!text.empty() && text.back() == '\0') {
			text.pop_back();
		}
		return text;
	}

	[[nodiscard]]
	auto encode(std::string const& value, byte_writer& out) const -> encode_result {
		auto const start = out.size();
		for (auto position = size_t{0}; position < value.size();) {
			auto const code = detail::read_utf8(value, position);
			if (!code) {
				out.truncate(start);
				return std::unexpected(field_error::invalid_value);
			}

			auto const bits = static_cast<uint32_t>(*code);
			if (bits < 0x1'0000U) {
				out.put_big_endian(static_cast<uint16_t>(bits));
			}
			else {
				out.put_big_endian(static_cast<uint16_t>(0xD800U + ((bits - 0x1'0000U) >> 10U)));
				out.put_big_endian(static_cast<uint16_t>(0xDC00U + ((bits - 0x1'0000U) & 0x3FFU)));
			}
		}
		return {};
	}

	[[nodiscard]]
	auto to_string(std::string const& value) const -> std::string {
		return value;
	}

private:
	/// True when `in` has an even length and each high byte (big-endian) is zero
	[[nodiscard]]
	static auto zero_high_bytes(byte_span in) noexcept -> bool {
		if (in.size() % 2 != 0) {
			return false;
		}
		for (auto i = size_t{0}; i < in.size(); i += 2) {
			if (in[i] != std::byte{0}) {
				return false;
			}
		}
		return true;
	}
};

}  // namespace klv::codec
