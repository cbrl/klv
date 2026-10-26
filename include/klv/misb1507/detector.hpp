/**
 * @file klv/misb1507/detector.hpp
 * @brief MISB ST 1507 detector locations and the Window Corners Pack (Table 2)
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codecs/pack.hpp"
#include "klv/field.hpp"

#include <cstdint>
#include <expected>
#include <format>
#include <string>

namespace klv::misb1507 {

/// Location of a detector in the Detector Group. The first row and the first column are 0.
struct detector {
	uint32_t row = 0;
	uint32_t column = 0;

	friend auto operator==(detector const&, detector const&) -> bool = default;
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

/**
 * @brief BER-OID value of up to `T` bits
 *
 * A value that the pack cuts off fails with `field_error::bad_length`, as a short `read_exact` does. Another malformed
 * value fails with `field_error::invalid_value`.
 */
template<ber_oid_integer T = uint32_t>
[[nodiscard]]
auto read_number(byte_reader& reader) -> field<T> {
	auto const value = reader.read_ber_oid<T>();
	if (!value) {
		auto const truncated = value.error().code == error_code::unexpected_end_of_data;
		return std::unexpected(truncated ? field_error::bad_length : field_error::invalid_value);
	}
	return *value;
}

[[nodiscard]]
inline auto read_detector(byte_reader& reader) -> field<detector> {
	auto const row = read_number(reader);
	if (!row) {
		return std::unexpected(row.error());
	}
	auto const column = read_number(reader);
	if (!column) {
		return std::unexpected(column.error());
	}
	return detector{.row = *row, .column = *column};
}

inline auto put_detector(detector const& value, byte_writer& out) -> void {
	out.put_ber_oid(value.row);
	out.put_ber_oid(value.column);
}

[[nodiscard]]
inline auto to_text(detector const& value) -> std::string {
	return std::format("({}, {})", value.row, value.column);
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

}  // namespace klv::misb1507
