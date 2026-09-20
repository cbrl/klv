/**
 * @file misb1507.hpp
 * @brief MISB ST 1507 - Motion Imagery Sensor Timing Metadata
 *
 * The Sensor Timing Local Set gives the exposure time of each detector of a sensor. It supports global shutter,
 * rolling shutter, and multi-Region shutter sensors. The set arrives as a stand-alone packet that ends with a
 * CRC-16-CCITT checksum.
 * - The Reference Time Pack (tag 1) is a MISB ST 1603 Nano Time Transfer Pack. Each exposure time in the set is an
 *   offset in nanoseconds from its time stamp.
 * - The Window Corners Pack (tag 2) gives the detectors that make the image.
 * - The Region List (tag 3) gives the Exposure Pattern of each Region of the Detector Group. Use `add_region` and
 *   `regions` to write and read it.
 *
 * @code
 * auto const& reference = timing.get<klv::misb1507::reference_time_pack>();
 * auto const& list = timing.get<klv::misb1507::region_list>();
 * if (reference && list) {
 *     for (auto const& region : klv::misb1507::regions(*list)) {
 *         auto const start = reference->time + region.start;  // klv::nano_timestamp
 *     }
 * }
 * @endcode
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/codecs/pack.hpp"
#include "klv/misb1603.hpp"
#include "klv/set.hpp"

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace klv::misb1507 {

/// Location of a detector in the Detector Group. The first row and the first column are 0.
struct detector {
	uint32_t row = 0;
	uint32_t column = 0;

	friend auto operator==(detector const&, detector const&) -> bool = default;
};

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

/**
 * @brief Detector window: the detectors that make the image (ST 1507 Table 2)
 *
 * The window can have any orientation in the Detector Group. `c1` becomes the upper left corner of the image, and
 * `c2` becomes the lower right corner.
 */
struct detector_window {
	detector c1{};
	detector c2{};

	friend auto operator==(detector_window const&, detector_window const&) -> bool = default;
};

namespace detail {

[[nodiscard]]
inline auto read_detector(byte_reader& reader) -> field<detector> {
	auto const row = klv::detail::read_oid(reader);
	if (!row) {
		return std::unexpected(row.error());
	}
	auto const column = klv::detail::read_oid(reader);
	if (!column) {
		return std::unexpected(column.error());
	}
	return detector{.row = *row, .column = *column};
}

inline auto put_detector(detector const& value, byte_writer& out) -> void {
	out.put_ber_oid(value.row);
	out.put_ber_oid(value.column);
}

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
inline auto to_text(detector const& value) -> std::string {
	return std::format("({}, {})", value.row, value.column);
}

[[nodiscard]]
inline auto to_text(detector_start const& value) -> std::string {
	return std::format("{} at {} ns", to_text(value.location), value.start.count());
}

}  // namespace detail

/// Window Corners Pack: four BER-OID values in the order C1 row, C1 column, C2 row, C2 column
struct window_codec {
	using value_type = detector_window;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<detector_window> {
		auto reader = byte_reader{in};
		auto const c1 = detail::read_detector(reader);
		if (!c1) {
			return std::unexpected(c1.error());
		}
		auto const c2 = detail::read_detector(reader);
		if (!c2) {
			return std::unexpected(c2.error());
		}
		return klv::detail::finish(reader, detector_window{.c1 = *c1, .c2 = *c2});
	}

	[[nodiscard]]
	auto encode(detector_window const& value, byte_writer& out) const -> encode_result {
		detail::put_detector(value.c1, out);
		detail::put_detector(value.c2, out);
		return {};
	}

	[[nodiscard]]
	auto to_string(detector_window const& value) const -> std::string {
		return std::format("{{c1: {}, c2: {}}}", detail::to_text(value.c1), detail::to_text(value.c2));
	}
};

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

// Region Local Set items. Each tag is one style of Exposure Pattern (ST 1507 Table 5). The set holds one item for each
// Region, so it can hold a tag more than once.

/// All detectors of the Region have the same exposure time (Tuple 0 and Tuple 3)
struct global_shutter {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Global Shutter Region";
	static constexpr codec::repeated<region_codec> format{};
};

/// Each detector of a Sub-Region has its own Exposure Start Time (Tuple 0, Tuple 1, and Tuple 3)
struct detector_delay {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Detector Delay Region";
	static constexpr codec::repeated<region_codec> format{.item = {.sub_region_end = true}};
};

/// Each Sub-Region starts after a delay (Tuple 0, Tuple 2, and Tuple 3)
struct sub_region_delay {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Sub-Region Delay Region";
	static constexpr codec::repeated<region_codec> format{.item = {.second_sub_region = true}};
};

/// Each detector of a Sub-Region has its own Exposure Start Time, and each Sub-Region starts after a delay (all
/// tuples)
struct detector_and_sub_region_delay {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Detector and Sub-Region Delay Region";
	static constexpr codec::repeated<region_codec> format{.item = {.sub_region_end = true, .second_sub_region = true}};
};

/// MISB ST 1507 Region Local Set: the Region List of a Sensor Timing Local Set
struct region_ls {
	static constexpr std::string_view name = "Region Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 02 01 02 00 00");

	using fields = field_list<global_shutter, detector_delay, sub_region_delay, detector_and_sub_region_delay>;
};

/// Add `value` to the Region List. The tuples of `value` select the tag.
inline auto add_region(record<region_ls>& list, region value) -> void {
	if (value.sub_region_end && value.second_sub_region) {
		list.add<detector_and_sub_region_delay>(std::move(value));
	}
	else if (value.sub_region_end) {
		list.add<detector_delay>(std::move(value));
	}
	else if (value.second_sub_region) {
		list.add<sub_region_delay>(std::move(value));
	}
	else {
		list.add<global_shutter>(std::move(value));
	}
}

/// The regions of the Region List, in tag order. An item that did not decode adds no region.
[[nodiscard]]
inline auto regions(record<region_ls> const& list) -> std::vector<region> {
	auto all = std::vector<region>{};
	auto const append = [&](field<std::vector<region>> const& items) {
		if (items) {
			all.insert(all.end(), items->begin(), items->end());
		}
	};
	append(list.get<global_shutter>());
	append(list.get<detector_delay>());
	append(list.get<sub_region_delay>());
	append(list.get<detector_and_sub_region_delay>());
	return all;
}

// Sensor Timing Local Set items

/// The time stamp that each exposure offset counts from, and its Time Transfer Local Set. The time is at or before
/// the first Exposure Start Time of the Detector Group.
struct reference_time_pack {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Reference Time Pack";
	static constexpr misb1603::nano_time_transfer_codec format{};
};

/// Without this item, the window is the full Detector Group
struct window_corners_pack {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Window Corners Pack";
	static constexpr window_codec format{};
};

/// One item for each Region of the Detector Group
struct region_list {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Region List";
	static constexpr codec::nested<region_ls> format{};
};

/// CRC-16-CCITT of the packet, from the key to the length of this item
struct checksum {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "CRC-16-CCITT";
	static constexpr codec::integer<uint16_t> format{};
};

/**
 * @brief MISB ST 1507 Sensor Timing Local Set
 *
 * The set holds the Window Corners Pack, the Region List, or both. Packet decoding verifies the CRC-16-CCITT checksum
 * (tag 4), and packet encoding computes it.
 */
struct sensor_timing_ls {
	static constexpr std::string_view name = "Sensor Timing Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 02 01 00 00 00");

	using checksum_field = checksum;
	static constexpr auto checksum_function = crc16_ccitt;

	using fields = field_list<reference_time_pack, window_corners_pack, region_list, checksum>;
};

}  // namespace klv::misb1507
