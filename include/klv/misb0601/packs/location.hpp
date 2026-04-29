/**
 * @file klv/misb0601/packs/location.hpp
 * @brief MISB ST 0601 Location pack (tags 130 and 141)
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"

#include <expected>
#include <format>
#include <optional>
#include <string>

namespace klv::misb0601 {

/// Geodetic location
struct location {
	double latitude = 0.0;  ///< deg
	double longitude = 0.0;  ///< deg
	std::optional<double> height;  ///< m above the WGS84 ellipsoid

	friend auto operator==(location const&, location const&) -> bool = default;
};

/// Location pack: latitude and longitude (IMAPB, 4 bytes each), then an optional height (IMAPB, 3 bytes)
struct location_codec {
	using value_type = location;

	static constexpr codec::imapb latitude_format{-90.0, 90.0, 4};
	static constexpr codec::imapb longitude_format{-180.0, 180.0, 4};
	static constexpr codec::imapb height_format{-900.0, 9'000.0, 3};

	[[nodiscard]]
	auto decode(byte_span in) const -> field<location> {
		if (in.size() != 8 && in.size() != 11) {
			return std::unexpected(field_error::bad_length);
		}

		auto const latitude = latitude_format.decode(in.first(4));
		if (!latitude) {
			return std::unexpected(latitude.error());
		}
		auto const longitude = longitude_format.decode(in.subspan(4, 4));
		if (!longitude) {
			return std::unexpected(longitude.error());
		}

		auto result = location{.latitude = *latitude, .longitude = *longitude};
		if (in.size() == 11) {
			auto const height = height_format.decode(in.subspan(8));
			if (!height) {
				return std::unexpected(height.error());
			}
			result.height = *height;
		}
		return result;
	}

	[[nodiscard]]
	auto encode(location const& value, byte_writer& out) const -> encode_result {
		return latitude_format.encode(value.latitude, out)
		    .and_then([&] { return longitude_format.encode(value.longitude, out); })
		    .and_then([&] { return value.height ? height_format.encode(*value.height, out) : encode_result{}; });
	}

	[[nodiscard]]
	auto to_string(location const& value) const -> std::string {
		auto text = std::format("{{latitude: {:.6f}, longitude: {:.6f}", value.latitude, value.longitude);
		if (value.height) {
			text += std::format(", height: {:.2f}", *value.height);
		}
		return text + "}";
	}
};

}  // namespace klv::misb0601
