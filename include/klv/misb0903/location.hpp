/**
 * @file klv/misb0903/location.hpp
 * @brief MISB ST 0903 Location, Velocity, and Acceleration packs
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
#include <tuple>

namespace klv::misb0903 {

/// Standard deviations along the east, north, and up axes
struct standard_deviations {
	double east = 0.0;
	double north = 0.0;
	double up = 0.0;

	// Exact comparison. std::tie compares in the standard library, which keeps -Wfloat-equal quiet.
	friend auto operator==(standard_deviations const& lhs, standard_deviations const& rhs) -> bool {
		return std::tie(lhs.east, lhs.north, lhs.up) == std::tie(rhs.east, rhs.north, rhs.up);
	}
};

/// Correlation coefficients of each pair of axes
struct correlations {
	double east_north = 0.0;
	double east_up = 0.0;
	double north_up = 0.0;

	// Exact comparison. std::tie compares in the standard library, which keeps -Wfloat-equal quiet.
	friend auto operator==(correlations const& lhs, correlations const& rhs) -> bool {
		return std::tie(lhs.east_north, lhs.east_up, lhs.north_up)
		    == std::tie(rhs.east_north, rhs.east_up, rhs.north_up);
	}
};

/// Geodetic position with optional precision values
struct location {
	double latitude = 0.0;  ///< deg
	double longitude = 0.0;  ///< deg
	double height = 0.0;  ///< m, above the WGS84 ellipsoid
	std::optional<standard_deviations> sigma;  ///< m
	std::optional<correlations> rho;  ///< Needs `sigma`

	// Exact comparison. std::tie compares in the standard library, which keeps -Wfloat-equal quiet.
	friend auto operator==(location const& lhs, location const& rhs) -> bool {
		return std::tie(lhs.latitude, lhs.longitude, lhs.height, lhs.sigma, lhs.rho)
		    == std::tie(rhs.latitude, rhs.longitude, rhs.height, rhs.sigma, rhs.rho);
	}
};

/// Velocity (m/s) or acceleration (m/s^2) along the east, north, and up axes, with optional precision values
struct motion {
	double east = 0.0;
	double north = 0.0;
	double up = 0.0;
	std::optional<standard_deviations> sigma;
	std::optional<correlations> rho;  ///< Needs `sigma`

	// Exact comparison. std::tie compares in the standard library, which keeps -Wfloat-equal quiet.
	friend auto operator==(motion const& lhs, motion const& rhs) -> bool {
		return std::tie(lhs.east, lhs.north, lhs.up, lhs.sigma, lhs.rho)
		    == std::tie(rhs.east, rhs.north, rhs.up, rhs.sigma, rhs.rho);
	}
};

namespace detail {

inline constexpr codec::imapb sigma_format{0.0, 650.0, 2};
inline constexpr codec::imapb rho_format{-1.0, 1.0, 2};

/// Decode three IMAPB values
[[nodiscard]]
inline auto read_triple(byte_reader& reader, std::array<codec::imapb const*, 3> const& formats)
    -> field<std::array<double, 3>> {
	auto values = std::array<double, 3>{};
	for (auto i = size_t{0}; i < formats.size(); ++i) {
		auto const bytes = klv::detail::read_exact(reader, formats[i]->length());
		if (!bytes) {
			return std::unexpected(bytes.error());
		}
		auto const value = formats[i]->decode(*bytes);
		if (!value) {
			return std::unexpected(value.error());
		}
		values[i] = *value;
	}
	return values;
}

/// Decode the optional sigma and rho parts after the main values. Each part has 6 bytes.
[[nodiscard]]
inline auto read_precision(
	byte_reader& reader,
	std::optional<standard_deviations>& sigma,
	std::optional<correlations>& rho
) -> field<void> {
	constexpr auto part_length = size_t{6};

	if (reader.remaining() >= part_length) {
		auto const values = read_triple(reader, {&sigma_format, &sigma_format, &sigma_format});
		if (!values) {
			return std::unexpected(values.error());
		}
		sigma = standard_deviations{.east = (*values)[0], .north = (*values)[1], .up = (*values)[2]};
	}
	if (reader.remaining() >= part_length) {
		auto const values = read_triple(reader, {&rho_format, &rho_format, &rho_format});
		if (!values) {
			return std::unexpected(values.error());
		}
		rho = correlations{.east_north = (*values)[0], .east_up = (*values)[1], .north_up = (*values)[2]};
	}
	return {};
}

[[nodiscard]]
inline auto put_precision(
	std::optional<standard_deviations> const& sigma,
	std::optional<correlations> const& rho,
	byte_writer& out
) -> encode_result {
	if (rho && !sigma) {
		return std::unexpected(field_error::invalid_value);
	}
	auto status = encode_result{};
	if (sigma) {
		for (auto const value : {sigma->east, sigma->north, sigma->up}) {
			status = status.and_then([&] { return sigma_format.encode(value, out); });
		}
	}
	if (rho) {
		for (auto const value : {rho->east_north, rho->east_up, rho->north_up}) {
			status = status.and_then([&] { return rho_format.encode(value, out); });
		}
	}
	return status;
}

[[nodiscard]]
inline auto precision_text(std::optional<standard_deviations> const& sigma, std::optional<correlations> const& rho)
    -> std::string {
	auto text = std::string{};
	if (sigma) {
		text += std::format(", sigma: {{{:.3f}, {:.3f}, {:.3f}}}", sigma->east, sigma->north, sigma->up);
	}
	if (rho) {
		text += std::format(", rho: {{{:.4f}, {:.4f}, {:.4f}}}", rho->east_north, rho->east_up, rho->north_up);
	}
	return text;
}

}  // namespace detail

/**
 * @brief Location pack: latitude, longitude, and height as IMAPB values of 4, 4, and 2 bytes
 *
 * The optional sigma and rho parts follow.
 */
struct location_codec {
	using value_type = location;

	static constexpr codec::imapb latitude_format{-90.0, 90.0, 4};
	static constexpr codec::imapb longitude_format{-180.0, 180.0, 4};
	static constexpr codec::imapb height_format{-900.0, 19'000.0, 2};

	[[nodiscard]]
	auto decode(byte_span in) const -> field<location> {
		auto reader = byte_reader{in};
		auto const values = detail::read_triple(reader, {&latitude_format, &longitude_format, &height_format});
		if (!values) {
			return std::unexpected(values.error());
		}

		auto result = location{.latitude = (*values)[0], .longitude = (*values)[1], .height = (*values)[2]};
		if (auto status = detail::read_precision(reader, result.sigma, result.rho); !status) {
			return std::unexpected(status.error());
		}
		return klv::detail::finish(reader, result);
	}

	[[nodiscard]]
	auto encode(location const& value, byte_writer& out) const -> encode_result {
		return latitude_format.encode(value.latitude, out)
		    .and_then([&] { return longitude_format.encode(value.longitude, out); })
		    .and_then([&] { return height_format.encode(value.height, out); })
		    .and_then([&] { return detail::put_precision(value.sigma, value.rho, out); });
	}

	[[nodiscard]]
	auto to_string(location const& value) const -> std::string {
		return std::format(
		    "{{latitude: {:.6f}, longitude: {:.6f}, height: {:.1f}{}}}",
		    value.latitude,
		    value.longitude,
		    value.height,
		    detail::precision_text(value.sigma, value.rho)
		);
	}
};

/**
 * @brief Velocity or Acceleration pack: east, north, and up as IMAPB values of 2 bytes
 *
 * The optional sigma and rho parts follow.
 */
struct motion_codec {
	using value_type = motion;

	static constexpr codec::imapb axis_format{-900.0, 900.0, 2};

	[[nodiscard]]
	auto decode(byte_span in) const -> field<motion> {
		auto reader = byte_reader{in};
		auto const values = detail::read_triple(reader, {&axis_format, &axis_format, &axis_format});
		if (!values) {
			return std::unexpected(values.error());
		}

		auto result = motion{.east = (*values)[0], .north = (*values)[1], .up = (*values)[2]};
		if (auto status = detail::read_precision(reader, result.sigma, result.rho); !status) {
			return std::unexpected(status.error());
		}
		return klv::detail::finish(reader, result);
	}

	[[nodiscard]]
	auto encode(motion const& value, byte_writer& out) const -> encode_result {
		return axis_format.encode(value.east, out)
		    .and_then([&] { return axis_format.encode(value.north, out); })
		    .and_then([&] { return axis_format.encode(value.up, out); })
		    .and_then([&] { return detail::put_precision(value.sigma, value.rho, out); });
	}

	[[nodiscard]]
	auto to_string(motion const& value) const -> std::string {
		return std::format(
		    "{{east: {:.3f}, north: {:.3f}, up: {:.3f}{}}}",
		    value.east,
		    value.north,
		    value.up,
		    detail::precision_text(value.sigma, value.rho)
		);
	}
};

}  // namespace klv::misb0903
