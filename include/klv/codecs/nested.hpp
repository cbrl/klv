/**
 * @file klv/codecs/nested.hpp
 * @brief Codec for a local set inside a field of another set
 */

#pragma once

#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/field.hpp"
#include "klv/set/decode.hpp"
#include "klv/set/definition.hpp"
#include "klv/set/describe.hpp"
#include "klv/set/encode.hpp"
#include "klv/set/record.hpp"

#include <expected>
#include <format>
#include <string>
#include <utility>

namespace klv::codec {

/**
 * @brief A local set inside a field of another set (for example MISB ST 0102 in ST 0601 tag 48)
 */
template<local_set_definition S>
struct nested {
	using value_type = record<S>;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<record<S>> {
		auto rec = decode_set<S>(in);
		if (!rec) {
			return std::unexpected(field_error::invalid_value);
		}
		return std::move(*rec);
	}

	[[nodiscard]]
	auto encode(record<S> const& value, byte_writer& out) const -> encode_result {
		if (!detail::encode_fields(value, out)) {
			return std::unexpected(field_error::invalid_value);
		}
		return {};
	}

	[[nodiscard]]
	auto to_string(record<S> const& value) const -> std::string {
		auto text = std::string{};
		for (auto const& entry : describe(value)) {
			if (!text.empty()) {
				text += "; ";
			}
			text += std::format(
			    "{}: {}",
			    entry.name,
			    entry.value ? *entry.value : std::string{klv::to_string(entry.value.error())}
			);
		}
		return text;
	}
};

}  // namespace klv::codec
