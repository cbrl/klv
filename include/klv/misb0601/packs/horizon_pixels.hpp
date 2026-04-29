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
#include <optional>
#include <string>

namespace klv::misb0601 {

/// Geodetic locations of the two horizon points
struct horizon_locations {
	double latitude0 = 0.0;  ///< deg
	double longitude0 = 0.0;  ///< deg
	double latitude1 = 0.0;  ///< deg
	double longitude1 = 0.0;  ///< deg

	friend auto operator==(horizon_locations const&, horizon_locations const&) -> bool = default;
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

/// Image Horizon Pixel Pack: four 1-byte coordinates, then optionally four 4-byte mapped latitudes and longitudes
struct horizon_pixels_codec {
	using value_type = horizon_pixels;

	static constexpr codec::mapped<uint32_t> latitude_format{-90.0, 90.0};
	static constexpr codec::mapped<uint32_t> longitude_format{-180.0, 180.0};

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
		};
		if (in.size() == 4) {
			return result;
		}

		auto const latitude0 = latitude_format.decode(in.subspan(4, 4));
		auto const longitude0 = longitude_format.decode(in.subspan(8, 4));
		auto const latitude1 = latitude_format.decode(in.subspan(12, 4));
		auto const longitude1 = longitude_format.decode(in.subspan(16, 4));
		for (auto const* part : {&latitude0, &longitude0, &latitude1, &longitude1}) {
			if (!*part) {
				return std::unexpected(part->error());
			}
		}

		result.locations = horizon_locations{
		    .latitude0 = *latitude0,
		    .longitude0 = *longitude0,
		    .latitude1 = *latitude1,
		    .longitude1 = *longitude1,
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
