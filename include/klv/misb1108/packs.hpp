/**
 * @file klv/misb1108/packs.hpp
 * @brief MISB ST 1108 Metric Period Pack (tag 2) and Window Corners Pack (tag 3)
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/codecs/pack.hpp"

#include <chrono>
#include <cstdint>
#include <expected>
#include <format>
#include <initializer_list>
#include <limits>
#include <string>

namespace klv::misb1108 {

/// Period of the frames that a metric covers (tag 2)
struct metric_period {
	timestamp start{};  ///< Precision Time Stamp of the first frame
	std::chrono::microseconds offset{0};  ///< Time from the first frame to the last frame, 0 to 2^32 - 1 us

	/// Time of the last frame
	[[nodiscard]]
	auto end() const -> timestamp {
		return start + offset;
	}

	friend auto operator==(metric_period const&, metric_period const&) -> bool = default;
};

/// Metric Period Pack: an 8-byte Precision Time Stamp, then a 4-byte offset in microseconds
struct metric_period_codec {
	using value_type = metric_period;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<metric_period> {
		if (in.size() != 12) {
			return std::unexpected(field_error::bad_length);
		}
		auto const start = codec::timestamp{}.decode(in.first(8));
		if (!start) {
			return std::unexpected(start.error());
		}
		auto const offset = read_big_endian<uint32_t>(in.subspan(8));
		return metric_period{.start = *start, .offset = std::chrono::microseconds{offset}};
	}

	[[nodiscard]]
	auto encode(metric_period const& value, byte_writer& out) const -> encode_result {
		auto const offset = value.offset.count();
		if (offset < 0 || offset > std::numeric_limits<uint32_t>::max()) {
			return std::unexpected(field_error::out_of_range);
		}
		auto const start = codec::timestamp{}.encode(value.start, out);
		if (!start) {
			return start;
		}
		out.put_big_endian(static_cast<uint32_t>(offset));
		return {};
	}

	[[nodiscard]]
	auto to_string(metric_period const& value) const -> std::string {
		return std::format(
		    "{{start: {}, offset: {} us}}", codec::timestamp{}.to_string(value.start), value.offset.count()
		);
	}
};

/**
 * @brief Sub-region of the frame that the metrics cover (tag 3)
 *
 * C1 is the upper left corner and C2 is the lower right corner. Rows and columns start at zero. Without this pack,
 * the metrics cover the whole frame.
 */
struct window_corners {
	uint32_t c1_row = 0;
	uint32_t c1_column = 0;
	uint32_t c2_row = 0;
	uint32_t c2_column = 0;

	friend auto operator==(window_corners const&, window_corners const&) -> bool = default;
};

/// Window Corners Pack: four BER-OID values in the order C1 row, C1 column, C2 row, C2 column
struct window_corners_codec {
	using value_type = window_corners;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<window_corners> {
		auto reader = byte_reader{in};
		auto value = window_corners{};
		for (auto* item : {&value.c1_row, &value.c1_column, &value.c2_row, &value.c2_column}) {
			auto const number = detail::read_oid(reader);
			if (!number) {
				return std::unexpected(number.error());
			}
			*item = *number;
		}
		return detail::finish(reader, value);
	}

	[[nodiscard]]
	auto encode(window_corners const& value, byte_writer& out) const -> encode_result {
		for (auto const item : {value.c1_row, value.c1_column, value.c2_row, value.c2_column}) {
			out.put_ber_oid(item);
		}
		return {};
	}

	[[nodiscard]]
	auto to_string(window_corners const& value) const -> std::string {
		return std::format(
		    "{{c1_row: {}, c1_column: {}, c2_row: {}, c2_column: {}}}",
		    value.c1_row,
		    value.c1_column,
		    value.c2_row,
		    value.c2_column
		);
	}
};

}  // namespace klv::misb1108
