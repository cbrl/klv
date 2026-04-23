/**
 * @file klv/codecs/series.hpp
 * @brief Codecs for repeated values: a series of length-prefixed elements, and a list of BER-OID integers
 */

#pragma once

#include "klv/bytes/byte_reader.hpp"
#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/codecs/codec_type.hpp"
#include "klv/field.hpp"

#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <utility>
#include <vector>

namespace klv::codec {

/**
 * @brief Series: zero or more elements, each with a BER length before it
 *
 * Decoding fails with the element error when one element does not decode, and with `field_error::invalid_value`
 * when a length is malformed or overruns the value.
 *
 * @code
 * static constexpr codec::series<waypoint_record_codec> format{};
 * @endcode
 */
template<codec_type Element>
struct series {
	using element_type = typename Element::value_type;
	using value_type = std::vector<element_type>;

	Element element{};

	[[nodiscard]]
	auto decode(byte_span in) const -> field<value_type> {
		auto reader = byte_reader{in};
		auto result = value_type{};

		while (!reader.empty()) {
			auto const length = reader.read_ber_length();
			if (!length) {
				return std::unexpected(field_error::invalid_value);
			}
			auto const bytes = reader.take(*length);
			if (!bytes) {
				return std::unexpected(field_error::invalid_value);
			}

			auto item = element.decode(*bytes);
			if (!item) {
				return std::unexpected(item.error());
			}
			result.push_back(std::move(*item));
		}
		return result;
	}

	[[nodiscard]]
	auto encode(value_type const& value, byte_writer& out) const -> encode_result {
		for (auto const& item : value) {
			auto status = out.put_length_prefixed([&] { return element.encode(item, out); });
			if (!status) {
				return status;
			}
		}
		return {};
	}

	[[nodiscard]]
	auto to_string(value_type const& value) const -> std::string {
		auto text = std::string{"["};
		for (auto const& item : value) {
			if (text.size() > 1) {
				text += ", ";
			}
			text += element.to_string(item);
		}
		return text + "]";
	}
};

/**
 * @brief List of BER-OID integers without separators
 */
struct ber_oid_list {
	using value_type = std::vector<uint32_t>;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<value_type> {
		auto reader = byte_reader{in};
		auto result = value_type{};

		while (!reader.empty()) {
			auto const item = reader.read_ber_oid();
			if (!item) {
				return std::unexpected(field_error::invalid_value);
			}
			result.push_back(*item);
		}
		return result;
	}

	[[nodiscard]]
	auto encode(value_type const& value, byte_writer& out) const -> encode_result {
		for (auto const item : value) {
			out.put_ber_oid(item);
		}
		return {};
	}

	[[nodiscard]]
	auto to_string(value_type const& value) const -> std::string {
		auto text = std::string{"["};
		for (auto const item : value) {
			text += std::format("{}{}", text.size() > 1 ? ", " : "", item);
		}
		return text + "]";
	}
};

}  // namespace klv::codec
