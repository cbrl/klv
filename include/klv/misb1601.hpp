/**
 * @file misb1601.hpp
 * @brief MISB ST 1601 - Geo-Registration Local Set
 *
 * The set usually arrives nested in MISB ST 0601 tag 98 (see `misb0601::geo_registration_local_set`). It holds the
 * correspondence points between the image and a reference image.
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/set.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace klv::misb1601 {

/// IMAPB range of one row of an SDCC array
struct row_range {
	double min = 0.0;
	double max = 0.0;
};

/**
 * @brief SDCC array: an MDARRAY of unsigned integers, with an IMAPB value of E bytes in each element
 *
 * The array has two dimensions. Each row holds one kind of standard deviation or correlation coefficient. Each column
 * holds one correspondence point. Each row has its own IMAPB range. The value is the array of the real values. Its
 * array algorithm applies to the raw integers, and its IMAPB limits are not used. An IMAPB special value decodes as
 * NaN.
 *
 * The array has `min_rows` or 6 rows. Another shape fails with `field_error::invalid_value`. A row range without
 * min < max, or with an infinite max - min, also fails with `field_error::invalid_value`.
 */
struct sdcc_array_codec {
	using value_type = mdarray<double>;

	std::array<row_range, 6> ranges{};
	uint32_t min_rows = 6;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<mdarray<double>> {
		auto const raw = raw_format.decode(in);
		if (!raw) {
			return std::unexpected(raw.error());
		}
		if (!has_valid_shape(raw->sizes, raw->element_size)) {
			return std::unexpected(field_error::invalid_value);
		}

		auto const formats = row_formats(raw->sizes[0], raw->element_size);
		if (!formats) {
			return std::unexpected(field_error::invalid_value);
		}

		auto result = mdarray<double>{
		    .sizes = raw->sizes,
		    .elements = {},
		    .element_size = raw->element_size,
		    .algorithm = raw->algorithm,
		};
		result.elements.reserve(raw->elements.size());

		// One buffer holds the IMAPB bytes of each element in turn
		auto const columns = size_t{raw->sizes[1]};
		auto bytes = byte_vector{};
		auto writer = byte_writer{bytes};
		for (auto i = size_t{0}; i < raw->elements.size(); ++i) {
			bytes.clear();
			writer.put_big_endian(raw->elements[i], raw->element_size);
			auto const element = (*formats)[i / columns].decode(bytes);
			result.elements.push_back(element.value_or(std::numeric_limits<double>::quiet_NaN()));
		}
		return result;
	}

	[[nodiscard]]
	auto encode(mdarray<double> const& value, byte_writer& out) const -> encode_result {
		if (!has_valid_shape(value.sizes, value.element_size)
		    || value.elements.size() != size_t{value.sizes[0]} * value.sizes[1]) {
			return std::unexpected(field_error::invalid_value);
		}

		auto const formats = row_formats(value.sizes[0], value.element_size);
		if (!formats) {
			return std::unexpected(field_error::invalid_value);
		}

		auto raw = mdarray<uint64_t>{
		    .sizes = value.sizes,
		    .elements = {},
		    .element_size = value.element_size,
		    .algorithm = value.algorithm,
		};
		raw.elements.reserve(value.elements.size());

		// One buffer holds the IMAPB bytes of each element in turn
		auto const columns = size_t{value.sizes[1]};
		auto bytes = byte_vector{};
		auto writer = byte_writer{bytes};
		for (auto i = size_t{0}; i < value.elements.size(); ++i) {
			bytes.clear();
			if (auto status = (*formats)[i / columns].encode(value.elements[i], writer); !status) {
				return status;
			}
			raw.elements.push_back(read_big_endian<uint64_t>(bytes));
		}
		return raw_format.encode(raw, out);
	}

	[[nodiscard]]
	auto to_string(mdarray<double> const& value) const -> std::string {
		return codec::mdarray<double>{}.to_string(value);
	}

private:
	static constexpr codec::mdarray<uint64_t> raw_format{};

	[[nodiscard]]
	auto has_valid_shape(std::vector<uint32_t> const& sizes, size_t element_size) const -> bool {
		return sizes.size() == 2 && (sizes[0] == min_rows || sizes[0] == ranges.size()) && element_size >= 1
		    && element_size <= codec::imapb::max_length;
	}

	/// The IMAPB format of each of the first `rows` rows, or no value when a row range is not valid
	[[nodiscard]]
	auto row_formats(size_t rows, size_t element_size) const -> std::optional<std::vector<codec::imapb>> {
		auto formats = std::vector<codec::imapb>{};
		formats.reserve(rows);
		for (auto const& range : ranges | std::views::take(static_cast<std::ptrdiff_t>(rows))) {
			if (!(range.min < range.max) || !std::isfinite(range.max - range.min)) {
				return std::nullopt;
			}
			formats.emplace_back(range.min, range.max, element_size);
		}
		return formats;
	}
};

struct document_version {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Document Version";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

struct algorithm_name {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Algorithm Name";
	static constexpr codec::string format{};
};

struct algorithm_version {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Algorithm Version";
	static constexpr codec::string format{};
};

/// Pixel coordinates of the correspondence points
struct correspondence_points_row_column {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Correspondence Points - Row/Column";
	static constexpr codec::mdarray<uint64_t> format{};
};

/// Geographic coordinates of the correspondence points
struct correspondence_points_latitude_longitude {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "Correspondence Points - Latitude/Longitude";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mdarray<double> format{};
};

/// Name of the reference image
struct second_image_name {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "Second Image Name";
	static constexpr codec::string format{};
};

/// Identifier of the algorithm parameters
struct algorithm_configuration_identifier {
	static constexpr uint32_t tag = 7;
	static constexpr std::string_view name = "Algorithm Configuration Identifier";
	static constexpr codec::uuid format{};
};

struct correspondence_points_elevation {
	static constexpr uint32_t tag = 8;
	static constexpr std::string_view name = "Correspondence Points - Elevation";
	static constexpr std::string_view units = "m";
	static constexpr codec::mdarray<double> format{};
};

/// Precision of the pixel coordinates: six rows, each with its own IMAPB range
struct correspondence_points_row_column_sdcc {
	static constexpr uint32_t tag = 9;
	static constexpr std::string_view name = "Correspondence Points - Row/Column SDCC";
	static constexpr sdcc_array_codec format{
	    .ranges = {{{0.0, 100.0}, {0.0, 100.0}, {-1.0, 1.0}, {0.0, 100.0}, {0.0, 100.0}, {-1.0, 1.0}}},
	    .min_rows = 6,
	};
};

/// Precision of the geographic coordinates: three or six rows, each with its own IMAPB range
struct correspondence_points_latitude_longitude_elevation_sdcc {
	static constexpr uint32_t tag = 10;
	static constexpr std::string_view name = "Correspondence Points - Latitude/Longitude/Elevation SDCC";
	static constexpr sdcc_array_codec format{
	    .ranges = {{{0.0, 650.0}, {0.0, 650.0}, {-1.0, 1.0}, {0.0, 1000.0}, {-1.0, 1.0}, {-1.0, 1.0}}},
	    .min_rows = 3,
	};
};

/// MISB ST 1601 Geo-Registration Local Set
struct geo_registration_ls {
	static constexpr std::string_view name = "Geo-Registration Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 03 01 00 00 00");

	using fields = field_list<
	    document_version,
	    algorithm_name,
	    algorithm_version,
	    correspondence_points_row_column,
	    correspondence_points_latitude_longitude,
	    second_image_name,
	    algorithm_configuration_identifier,
	    correspondence_points_elevation,
	    correspondence_points_row_column_sdcc,
	    correspondence_points_latitude_longitude_elevation_sdcc>;
};

}  // namespace klv::misb1601
