/**
 * @file klv/misb1002/section_data.hpp
 * @brief MISB ST 1002 Section Data pack (tag 20): the range values of one section of the image
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/codecs/pack.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <optional>
#include <string>
#include <utility>

namespace klv::misb1002 {

/// Range values of one section of the range image
struct section_data {
	uint32_t section_x = 0;  ///< Section number along the x axis
	uint32_t section_y = 0;  ///< Section number along the y axis
	mdarray<double> measurements;  ///< Range values (m), or the planar fit residuals
	std::optional<mdarray<double>> uncertainty;  ///< Uncertainty of each range value (m)
	std::optional<double> plane_x_scale;  ///< Planar fit: x scale factor
	std::optional<double> plane_y_scale;  ///< Planar fit: y scale factor
	std::optional<double> plane_constant;  ///< Planar fit: constant

	friend auto operator==(section_data const&, section_data const&) -> bool = default;
};

/**
 * @brief Section Data pack: a series of values, each with a BER length before it
 *
 * The values are, in this order:
 * - the section numbers along x and y (BER-OID)
 * - the range MDARRAY
 * - the uncertainty MDARRAY
 * - the three planar fit values (IEEE 754)
 *
 * The writer can omit the optional values at the end. An omitted value before a present value has a zero length.
 * Encoding writes a planar fit value with 4 bytes when binary32 holds it exactly, and 8 bytes otherwise.
 */
struct section_data_codec {
	using value_type = section_data;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<section_data> {
		auto reader = byte_reader{in};
		auto result = section_data{};

		for (auto* number : {&result.section_x, &result.section_y}) {
			auto const bytes = klv::detail::read_prefixed(reader);
			if (!bytes) {
				return std::unexpected(bytes.error());
			}
			auto const value = codec::ber_oid{}.decode(*bytes);
			if (!value) {
				return std::unexpected(value.error());
			}
			*number = *value;
		}

		auto const measurements = klv::detail::read_prefixed(reader);
		if (!measurements) {
			return std::unexpected(measurements.error());
		}
		auto array = array_format.decode(*measurements);
		if (!array) {
			return std::unexpected(array.error());
		}
		result.measurements = std::move(*array);

		if (!reader.empty()) {
			auto const bytes = klv::detail::read_prefixed(reader);
			if (!bytes) {
				return std::unexpected(bytes.error());
			}
			if (!bytes->empty()) {
				auto uncertainty = array_format.decode(*bytes);
				if (!uncertainty) {
					return std::unexpected(uncertainty.error());
				}
				result.uncertainty = std::move(*uncertainty);
			}
		}

		for (auto* plane : {&result.plane_x_scale, &result.plane_y_scale, &result.plane_constant}) {
			if (reader.empty()) {
				break;
			}
			auto const bytes = klv::detail::read_prefixed(reader);
			if (!bytes) {
				return std::unexpected(bytes.error());
			}
			if (!bytes->empty()) {
				auto const value = plane_format.decode(*bytes);
				if (!value) {
					return std::unexpected(value.error());
				}
				*plane = *value;
			}
		}
		return klv::detail::finish(reader, std::move(result));
	}

	[[nodiscard]]
	auto encode(section_data const& value, byte_writer& out) const -> encode_result {
		auto status = encode_result{};
		for (auto const number : {value.section_x, value.section_y}) {
			status = status.and_then([&] {
				return out.put_length_prefixed([&] { return codec::ber_oid{}.encode(number, out); });
			});
		}
		status = status.and_then([&] {
			return out.put_length_prefixed([&] { return array_format.encode(value.measurements, out); });
		});

		// Omit the absent values at the end
		auto const planes = std::array{&value.plane_x_scale, &value.plane_y_scale, &value.plane_constant};
		auto plane_count = planes.size();
		while (plane_count > 0 && !*planes[plane_count - 1]) {
			--plane_count;
		}

		if (value.uncertainty || plane_count > 0) {
			status = status.and_then([&] {
				return out.put_length_prefixed([&] {
					return value.uncertainty ? array_format.encode(*value.uncertainty, out) : encode_result{};
				});
			});
		}
		for (auto i = size_t{0}; i < plane_count; ++i) {
			status = status.and_then([&] {
				return out.put_length_prefixed([&] {
					return *planes[i] ? plane_format.encode(**planes[i], out) : encode_result{};
				});
			});
		}
		return status;
	}

	[[nodiscard]]
	auto to_string(section_data const& value) const -> std::string {
		auto text = std::format(
		    "{{section: ({}, {}), measurements: {}",
		    value.section_x,
		    value.section_y,
		    array_format.to_string(value.measurements)
		);
		if (value.uncertainty) {
			text += std::format(", uncertainty: {}", array_format.to_string(*value.uncertainty));
		}
		if (value.plane_x_scale || value.plane_y_scale || value.plane_constant) {
			text += std::format(
			    ", plane: ({}, {}, {})",
			    value.plane_x_scale.value_or(0.0),
			    value.plane_y_scale.value_or(0.0),
			    value.plane_constant.value_or(0.0)
			);
		}
		return text + "}";
	}

private:
	static constexpr codec::mdarray<double> array_format{};
	static constexpr codec::ieee754_variable plane_format{};
};

}  // namespace klv::misb1002
