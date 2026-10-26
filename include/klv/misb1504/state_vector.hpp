/**
 * @file klv/misb1504/state_vector.hpp
 * @brief MISB ST 1504 State Vector Pack (tag 4) and the packs of real numbers
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/codecs/pack.hpp"
#include "klv/codecs/text.hpp"
#include "klv/misb1504/enumerations.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <string>

namespace klv::misb1504 {

/**
 * @brief Value of a State Vector Pack: the State Vector Status and the six State Vector Elements
 *
 * `elements[0]` is State Vector Element 1. The kind of the elements gives their meaning (ST 1504 Table 5):
 *
 * | Element | Keplerian                                   | Equinoctial           |
 * |---------|---------------------------------------------|-----------------------|
 * | 1       | Semi-major axis a (m)                       | Semi-major axis a (m) |
 * | 2       | Eccentricity e                              | h                     |
 * | 3       | Inclination i (deg)                         | k                     |
 * | 4       | Argument of perigee (deg)                   | Mean longitude (deg)  |
 * | 5       | Longitude of the ascending node (deg)       | p                     |
 * | 6       | Anomaly that the Anomaly Type selects (deg) | q                     |
 */
struct state_vector {
	reference_frame frame{};
	state_vector_kind kind{};
	std::array<double, 6> elements{};

	friend auto operator==(state_vector const&, state_vector const&) -> bool = default;
};

namespace detail {

/// Read `N` real numbers of one length. The length of `in` must be `N` times 4 or `N` times 8 bytes.
template<size_t N>
[[nodiscard]]
auto read_reals(byte_span in) -> field<std::array<double, N>> {
	if (in.size() != N * sizeof(float) && in.size() != N * sizeof(double)) {
		return std::unexpected(field_error::bad_length);
	}
	auto const length = in.size() / N;
	auto values = std::array<double, N>{};
	for (auto i = size_t{0}; i < N; ++i) {
		auto const value = klv::detail::read_ieee(in.subspan(i * length, length));
		if (!value) {
			return std::unexpected(value.error());
		}
		values[i] = *value;
	}
	return values;
}

/// Write each value with `length` bytes. With `variable_length`, the length is 4 when binary32 holds each value.
template<size_t N>
[[nodiscard]]
auto put_reals(std::array<double, N> const& values, size_t length, byte_writer& out) -> encode_result {
	if (length == codec::variable_length) {
		length = sizeof(float);
		for (auto const value : values) {
			if (!klv::detail::fits_binary32(value)) {
				length = sizeof(double);
			}
		}
	}
	for (auto const value : values) {
		if (auto status = klv::detail::put_ieee(value, length, out); !status) {
			return status;
		}
	}
	return {};
}

}  // namespace detail

/**
 * @brief Pack of `N` real numbers of one length: 4 or 8 bytes each
 *
 * Decoding gets the length of each value from the length of the pack. Another pack length fails with
 * `field_error::bad_length`. Encoding writes `length` bytes for each value. With `variable_length`, it writes 4 bytes
 * when binary32 holds each value exactly, and 8 bytes otherwise.
 */
template<size_t N>
struct real_pack_codec {
	using value_type = std::array<double, N>;

	size_t length = codec::variable_length;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<value_type> {
		return detail::read_reals<N>(in);
	}

	[[nodiscard]]
	auto encode(value_type const& value, byte_writer& out) const -> encode_result {
		return detail::put_reals(value, length, out);
	}

	[[nodiscard]]
	auto to_string(value_type const& value) const -> std::string {
		return klv::detail::list_text(value);
	}
};

/**
 * @brief State Vector Pack: the State Vector Status (BER-OID), then six real numbers of one length
 *
 * Bits 6 to 4 of the first byte of the State Vector Status hold the Reference Frame. Bits 3 to 0 hold the State Vector
 * Type. A later version of ST 1504 can add more bytes, and decoding ignores them. Encoding writes one byte. A reserved
 * value fails with `field_error::invalid_value`.
 */
struct state_vector_codec {
	using value_type = state_vector;

	size_t length = codec::variable_length;  ///< Bytes of each element: 4, 8, or `variable_length`

	[[nodiscard]]
	auto decode(byte_span in) const -> field<state_vector> {
		if (in.empty()) {
			return std::unexpected(field_error::bad_length);
		}

		// Bit 7 of each byte of the BER-OID value, except the last byte, is 1
		auto status_length = size_t{1};
		while ((std::to_integer<uint8_t>(in[status_length - 1]) & 0x80U) != 0) {
			if (status_length == in.size()) {
				return std::unexpected(field_error::invalid_value);
			}
			++status_length;
		}

		auto const bits = std::to_integer<uint8_t>(in.front());
		auto value = state_vector{
		    .frame = static_cast<reference_frame>((bits >> 4U) & 0x07U),
		    .kind = static_cast<state_vector_kind>(bits & 0x0FU),
		    .elements = {},
		};
		if (!is_defined(value)) {
			return std::unexpected(field_error::invalid_value);
		}

		auto const elements = detail::read_reals<6>(in.subspan(status_length));
		if (!elements) {
			return std::unexpected(elements.error());
		}
		value.elements = *elements;
		return value;
	}

	[[nodiscard]]
	auto encode(state_vector const& value, byte_writer& out) const -> encode_result {
		if (!is_defined(value)) {
			return std::unexpected(field_error::invalid_value);
		}
		auto const frame = static_cast<unsigned>(value.frame) << 4U;
		out.put_u8(static_cast<uint8_t>(frame | static_cast<unsigned>(value.kind)));
		return detail::put_reals(value.elements, length, out);
	}

	[[nodiscard]]
	auto to_string(state_vector const& value) const -> std::string {
		return std::format(
		    "{{frame: {}, kind: {}, elements: {}}}",
		    klv::detail::enum_text(value.frame),
		    klv::detail::enum_text(value.kind),
		    klv::detail::list_text(value.elements)
		);
	}

private:
	[[nodiscard]]
	static constexpr auto is_defined(state_vector const& value) noexcept -> bool {
		return !misb1504::to_string(value.frame).empty() && !misb1504::to_string(value.kind).empty();
	}
};

}  // namespace klv::misb1504
