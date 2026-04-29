/**
 * @file klv/misb0601/packs/frame_rate.hpp
 * @brief MISB ST 0601 Sensor Frame Rate Pack (tag 127)
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/misb0601/packs/detail.hpp"

#include <cstdint>
#include <expected>
#include <format>
#include <string>

namespace klv::misb0601 {

/// Frame rate as a ratio of integers
struct frame_rate {
	uint32_t numerator = 0;
	uint32_t denominator = 1;

	friend auto operator==(frame_rate const&, frame_rate const&) -> bool = default;
};

/// Sensor Frame Rate Pack: BER-OID numerator, then an optional BER-OID denominator (1 when absent)
struct frame_rate_codec {
	using value_type = frame_rate;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<frame_rate> {
		auto reader = byte_reader{in};
		auto const numerator = detail::read_oid(reader);
		if (!numerator) {
			return std::unexpected(numerator.error());
		}

		auto result = frame_rate{.numerator = *numerator};
		if (!reader.empty()) {
			auto const denominator = detail::read_oid(reader);
			if (!denominator) {
				return std::unexpected(denominator.error());
			}
			result.denominator = *denominator;
		}
		if (result.denominator == 0) {
			return std::unexpected(field_error::invalid_value);
		}
		return detail::finish(reader, result);
	}

	[[nodiscard]]
	auto encode(frame_rate const& value, byte_writer& out) const -> encode_result {
		if (value.denominator == 0) {
			return std::unexpected(field_error::invalid_value);
		}
		out.put_ber_oid(value.numerator);
		if (value.denominator != 1) {
			out.put_ber_oid(value.denominator);
		}
		return {};
	}

	[[nodiscard]]
	auto to_string(frame_rate const& value) const -> std::string {
		if (value.denominator == 1) {
			return std::format("{}", value.numerator);
		}
		return std::format(
		    "{}/{} ({:.3f})",
		    value.numerator,
		    value.denominator,
		    static_cast<double>(value.numerator) / static_cast<double>(value.denominator)
		);
	}
};

}  // namespace klv::misb0601
