/**
 * @file klv/misb1507/region.hpp
 * @brief MISB ST 1507 Exposure Pattern of a Region (Table 1)
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codecs/pack.hpp"
#include "klv/field.hpp"
#include "klv/misb1507/detector.hpp"

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <initializer_list>
#include <limits>
#include <optional>
#include <string>
#include <utility>

namespace klv::misb1507 {

/// A detector and the offset of its Exposure Start Time from the Reference Time (Tuple 1 or Tuple 2)
struct detector_start {
	detector location{};
	std::chrono::nanoseconds start{0};

	friend auto operator==(detector_start const&, detector_start const&) -> bool = default;
};

/**
 * @brief Exposure Pattern of one Region (ST 1507 Table 1)
 *
 * Each time is an offset from the Reference Time. Without Tuple 1, the detectors of a Sub-Region start at the same
 * time. Without Tuple 2, all Sub-Regions start at the same time.
 */
struct region {
	detector first{};  ///< Tuple 0: the first detector of the first Sub-Region
	std::chrono::nanoseconds start{0};  ///< Tuple 0: Exposure Start Time of `first`
	std::chrono::nanoseconds end{0};  ///< Tuple 0: Exposure End Time of `first`
	std::optional<detector_start> sub_region_end;  ///< Tuple 1: the last detector of the first Sub-Region
	std::optional<detector_start> second_sub_region;  ///< Tuple 2: the first detector of the second Sub-Region
	detector last{};  ///< Tuple 3: the last detector of the last Sub-Region

	friend auto operator==(region const&, region const&) -> bool = default;
};

namespace detail {

/// Time offset as a BER-OID of up to 64 bits. A value above the `int64_t` range fails with `out_of_range`.
[[nodiscard]]
inline auto read_offset(byte_reader& reader) -> field<std::chrono::nanoseconds> {
	auto value = uint64_t{0};
	for (auto first = true;; first = false) {
		auto const byte = reader.read_u8();
		if (!byte || (first && *byte == 0x80U) || value > (std::numeric_limits<uint64_t>::max() >> 7U)) {
			return std::unexpected(field_error::invalid_value);
		}
		value = (value << 7U) | (*byte & 0x7FU);
		if ((*byte & 0x80U) == 0) {
			break;
		}
	}
	if (value > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
		return std::unexpected(field_error::out_of_range);
	}
	return std::chrono::nanoseconds{static_cast<int64_t>(value)};
}

[[nodiscard]]
inline auto put_offset(std::chrono::nanoseconds value, byte_writer& out) -> encode_result {
	if (value.count() < 0) {
		return std::unexpected(field_error::out_of_range);
	}
	auto bits = static_cast<uint64_t>(value.count());
	auto groups = std::array<uint8_t, 10>{};
	auto count = size_t{0};
	do {
		groups[count++] = static_cast<uint8_t>(bits & 0x7FU);
		bits >>= 7U;
	} while (bits != 0);
	for (auto i = count; i > 0; --i) {
		out.put_u8(static_cast<uint8_t>(groups[i - 1] | (i > 1 ? 0x80U : 0x00U)));
	}
	return {};
}

[[nodiscard]]
inline auto read_detector_start(byte_reader& reader) -> field<detector_start> {
	auto const location = read_detector(reader);
	if (!location) {
		return std::unexpected(location.error());
	}
	auto const start = read_offset(reader);
	if (!start) {
		return std::unexpected(start.error());
	}
	return detector_start{.location = *location, .start = *start};
}

[[nodiscard]]
inline auto put_detector_start(detector_start const& value, byte_writer& out) -> encode_result {
	put_detector(value.location, out);
	return put_offset(value.start, out);
}

[[nodiscard]]
inline auto to_text(detector_start const& value) -> std::string {
	return std::format("{} at {} ns", to_text(value.location), value.start.count());
}

}  // namespace detail

/**
 * @brief Value of a Region Local Set item: the tuples of one Region in order, as BER-OID values
 *
 * Tuple 0 and Tuple 3 are always present. `sub_region_end` selects Tuple 1, and `second_sub_region` selects Tuple 2.
 * Encoding fails with `field_error::invalid_value` when the tuples of the region do not match the codec.
 */
struct region_codec {
	using value_type = region;

	bool sub_region_end = false;  ///< The value holds Tuple 1
	bool second_sub_region = false;  ///< The value holds Tuple 2

	[[nodiscard]]
	auto decode(byte_span in) const -> field<region> {
		auto reader = byte_reader{in};
		auto value = region{};

		auto const first = detail::read_detector_start(reader);
		if (!first) {
			return std::unexpected(first.error());
		}
		auto const end = detail::read_offset(reader);
		if (!end) {
			return std::unexpected(end.error());
		}
		value.first = first->location;
		value.start = first->start;
		value.end = *end;

		if (sub_region_end) {
			auto tuple = detail::read_detector_start(reader);
			if (!tuple) {
				return std::unexpected(tuple.error());
			}
			value.sub_region_end = *tuple;
		}
		if (second_sub_region) {
			auto tuple = detail::read_detector_start(reader);
			if (!tuple) {
				return std::unexpected(tuple.error());
			}
			value.second_sub_region = *tuple;
		}

		auto const last = detail::read_detector(reader);
		if (!last) {
			return std::unexpected(last.error());
		}
		value.last = *last;
		return klv::detail::finish(reader, std::move(value));
	}

	[[nodiscard]]
	auto encode(region const& value, byte_writer& out) const -> encode_result {
		if (value.sub_region_end.has_value() != sub_region_end
		    || value.second_sub_region.has_value() != second_sub_region) {
			return std::unexpected(field_error::invalid_value);
		}
		if (auto const first = detail::put_detector_start({.location = value.first, .start = value.start}, out);
		    !first) {
			return first;
		}
		if (auto const end = detail::put_offset(value.end, out); !end) {
			return end;
		}
		for (auto const& tuple : {value.sub_region_end, value.second_sub_region}) {
			if (tuple) {
				if (auto const status = detail::put_detector_start(*tuple, out); !status) {
					return status;
				}
			}
		}
		detail::put_detector(value.last, out);
		return {};
	}

	[[nodiscard]]
	auto to_string(region const& value) const -> std::string {
		auto text = std::format(
		    "{{first: {}, start: {} ns, end: {} ns",
		    detail::to_text(value.first),
		    value.start.count(),
		    value.end.count()
		);
		if (value.sub_region_end) {
			text += std::format(", sub_region_end: {}", detail::to_text(*value.sub_region_end));
		}
		if (value.second_sub_region) {
			text += std::format(", second_sub_region: {}", detail::to_text(*value.second_sub_region));
		}
		return text + std::format(", last: {}}}", detail::to_text(value.last));
	}
};

}  // namespace klv::misb1507
