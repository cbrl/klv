/**
 * @file klv/misb0601/packs/horizon_pixels.hpp
 * @brief MISB ST 0601 Image Horizon Pixel Pack (tag 81)
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <limits>
#include <optional>
#include <string>
#include <tuple>

namespace klv::misb0601 {

/// Geodetic locations of the two horizon points. NaN stands for the error value of ST 0601 (-2^31).
struct horizon_locations {
	double latitude0 = 0.0;  ///< deg
	double longitude0 = 0.0;  ///< deg
	double latitude1 = 0.0;  ///< deg
	double longitude1 = 0.0;  ///< deg

	// Exact comparison. std::tie compares in the standard library, which keeps -Wfloat-equal quiet.
	friend auto operator==(horizon_locations const& lhs, horizon_locations const& rhs) -> bool {
		return std::tie(lhs.latitude0, lhs.longitude0, lhs.latitude1, lhs.longitude1)
		    == std::tie(rhs.latitude0, rhs.longitude0, rhs.latitude1, rhs.longitude1);
	}
};

/// Image coordinates of the two points where the horizon meets the image frame
struct horizon_pixels {
	uint8_t x0 = 0;
	uint8_t y0 = 0;
	uint8_t x1 = 0;
	uint8_t y1 = 0;
	std::optional<horizon_locations> locations;

	friend auto operator==(horizon_pixels const&, horizon_pixels const&) -> bool = default;
};

/**
 * @brief Image Horizon Pixel Pack: four 1-byte coordinates, then optionally four 4-byte mapped latitudes and longitudes
 *
 * Each latitude and longitude is an int32 that maps -(2^31-1)..(2^31-1) to +/-90 or +/-180 degrees (ST 0601 Table 6),
 * as in tags 82 to 89. The raw value -2^31 is the error value. It decodes as NaN. Encoding writes NaN, and a value
 * outside the range, as the error value.
 */
struct horizon_pixels_codec {
	using value_type = horizon_pixels;

	static constexpr codec::mapped<int32_t> latitude_format{-90.0, 90.0, codec::sentinel::reserve_min};
	static constexpr codec::mapped<int32_t> longitude_format{-180.0, 180.0, codec::sentinel::reserve_min};

	[[nodiscard]]
	auto decode(byte_span in) const -> field<horizon_pixels> {
		if (in.size() != 4 && in.size() != 20) {
			return std::unexpected(field_error::bad_length);
		}

		auto result = horizon_pixels{
		    .x0 = std::to_integer<uint8_t>(in[0]),
		    .y0 = std::to_integer<uint8_t>(in[1]),
		    .x1 = std::to_integer<uint8_t>(in[2]),
		    .y1 = std::to_integer<uint8_t>(in[3]),
		    .locations = std::nullopt,
		};
		if (in.size() == 4) {
			return result;
		}

		// Each raw value is in the domain or is the error value. Thus, only the error value fails.
		auto const coordinate = [&](codec::mapped<int32_t> const& format, size_t offset) {
			return format.decode(in.subspan(offset, 4)).value_or(std::numeric_limits<double>::quiet_NaN());
		};
		result.locations = horizon_locations{
		    .latitude0 = coordinate(latitude_format, 4),
		    .longitude0 = coordinate(longitude_format, 8),
		    .latitude1 = coordinate(latitude_format, 12),
		    .longitude1 = coordinate(longitude_format, 16),
		};
		return result;
	}

	[[nodiscard]]
	auto encode(horizon_pixels const& value, byte_writer& out) const -> encode_result {
		out.put_u8(value.x0);
		out.put_u8(value.y0);
		out.put_u8(value.x1);
		out.put_u8(value.y1);
		if (!value.locations) {
			return {};
		}

		auto const& points = *value.locations;
		return latitude_format.encode(points.latitude0, out)
		    .and_then([&] { return longitude_format.encode(points.longitude0, out); })
		    .and_then([&] { return latitude_format.encode(points.latitude1, out); })
		    .and_then([&] { return longitude_format.encode(points.longitude1, out); });
	}

	[[nodiscard]]
	auto to_string(horizon_pixels const& value) const -> std::string {
		auto text = std::format("{{x0: {}, y0: {}, x1: {}, y1: {}", value.x0, value.y0, value.x1, value.y1);
		if (value.locations) {
			auto const& points = *value.locations;
			text += std::format(
			    ", latitude0: {:.6f}, longitude0: {:.6f}, latitude1: {:.6f}, longitude1: {:.6f}",
			    points.latitude0,
			    points.longitude0,
			    points.latitude1,
			    points.longitude1
			);
		}
		return text + "}";
	}
};

}  // namespace klv::misb0601
