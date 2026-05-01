/**
 * @file klv/misb0601/packs/view_ranges.hpp
 * @brief MISB ST 0601 View Domain (tag 142)
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/codecs/pack.hpp"

#include <array>
#include <cstddef>
#include <expected>
#include <format>
#include <optional>
#include <string>

namespace klv::misb0601 {

/// Start angle and angular range of one sensor axis
struct view_interval {
	double start = 0.0;  ///< deg
	double range = 0.0;  ///< deg

	friend auto operator==(view_interval const&, view_interval const&) -> bool = default;
};

/// Possible sensor azimuth, elevation, and roll angles. An axis without a limit has no value.
struct view_ranges {
	std::optional<view_interval> azimuth;
	std::optional<view_interval> elevation;
	std::optional<view_interval> roll;

	friend auto operator==(view_ranges const&, view_ranges const&) -> bool = default;
};

/**
 * @brief View Domain: three length-prefixed intervals (azimuth, elevation, roll)
 *
 * Each interval holds two IMAPB values of half its length: the start, then the range from 0 to 360 deg. The start
 * range is 0 to 360 deg for azimuth and roll, and -180 to 180 deg for elevation. A zero length marks an absent
 * interval, and the writer can omit the last intervals. Encoding writes 3 bytes for each value.
 */
struct view_ranges_codec {
	using value_type = view_ranges;

	static constexpr auto value_length = size_t{3};
	static constexpr codec::imapb azimuth_format{0.0, 360.0, value_length};
	static constexpr codec::imapb elevation_format{-180.0, 180.0, value_length};
	static constexpr codec::imapb roll_format{0.0, 360.0, value_length};
	static constexpr codec::imapb range_format{0.0, 360.0, value_length};

	[[nodiscard]]
	auto decode(byte_span in) const -> field<view_ranges> {
		auto reader = byte_reader{in};
		auto result = view_ranges{};

		auto const axes = std::array{
		    axis{&result.azimuth, &azimuth_format},
		    axis{&result.elevation, &elevation_format},
		    axis{&result.roll, &roll_format},
		};
		for (auto const& [interval, start_format] : axes) {
			if (reader.empty()) {
				break;
			}
			auto const bytes = detail::read_prefixed(reader);
			if (!bytes) {
				return std::unexpected(bytes.error());
			}
			if (bytes->empty()) {
				continue;
			}
			if (bytes->size() % 2 != 0) {
				return std::unexpected(field_error::bad_length);
			}

			auto const half = bytes->size() / 2;
			auto const start = start_format->decode(bytes->first(half));
			if (!start) {
				return std::unexpected(start.error());
			}
			auto const range = range_format.decode(bytes->subspan(half));
			if (!range) {
				return std::unexpected(range.error());
			}
			*interval = view_interval{.start = *start, .range = *range};
		}
		return detail::finish(reader, result);
	}

	[[nodiscard]]
	auto encode(view_ranges const& value, byte_writer& out) const -> encode_result {
		auto const axes = std::array{
		    const_axis{&value.azimuth, &azimuth_format},
		    const_axis{&value.elevation, &elevation_format},
		    const_axis{&value.roll, &roll_format},
		};

		// Omit the absent intervals at the end
		auto count = axes.size();
		while (count > 0 && !*axes[count - 1].interval) {
			--count;
		}

		for (auto i = size_t{0}; i < count; ++i) {
			auto const& [interval, start_format] = axes[i];
			if (!*interval) {
				out.put_ber_length(0);
				continue;
			}
			auto status = out.put_length_prefixed([&] {
				return start_format->encode((*interval)->start, out).and_then([&] {
					return range_format.encode((*interval)->range, out);
				});
			});
			if (!status) {
				return status;
			}
		}
		return {};
	}

	[[nodiscard]]
	auto to_string(view_ranges const& value) const -> std::string {
		auto const text = [](std::optional<view_interval> const& interval) {
			return interval ? std::format("{{start: {:.4f}, range: {:.4f}}}", interval->start, interval->range)
			                : std::string{"absent"};
		};
		return std::format(
		    "{{azimuth: {}, elevation: {}, roll: {}}}",
		    text(value.azimuth),
		    text(value.elevation),
		    text(value.roll)
		);
	}

private:
	struct axis {
		std::optional<view_interval>* interval;
		codec::imapb const* start_format;
	};

	struct const_axis {
		std::optional<view_interval> const* interval;
		codec::imapb const* start_format;
	};
};

}  // namespace klv::misb0601
