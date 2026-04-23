/**
 * @file misb0601_packs.hpp
 * @brief MISB ST 0601 packs: the value types and codecs of the fields with an inner structure
 *
 * `misb0601.hpp` uses these codecs in its field descriptors. Each codec decodes the complete value bytes of one field.
 * Bytes after the last element of a pack are an error (`field_error::invalid_value`).
 */

#pragma once

#include "bytes.hpp"
#include "codec.hpp"
#include "misb0102.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace klv::misb0601 {

namespace detail {

[[nodiscard]]
inline auto read_oid(byte_reader& reader) -> field<uint32_t> {
	auto const value = reader.read_ber_oid();
	if (!value) {
		return std::unexpected(field_error::invalid_value);
	}
	return *value;
}

[[nodiscard]]
inline auto read_exact(byte_reader& reader, size_t count) -> field<byte_span> {
	auto const bytes = reader.take(count);
	if (!bytes) {
		return std::unexpected(field_error::bad_length);
	}
	return *bytes;
}

/// Read a BER length and the bytes that it counts
[[nodiscard]]
inline auto read_prefixed(byte_reader& reader) -> field<byte_span> {
	auto const length = reader.read_ber_length();
	if (!length) {
		return std::unexpected(field_error::invalid_value);
	}
	return read_exact(reader, *length);
}

[[nodiscard]]
inline auto to_text(byte_span bytes) -> std::string {
	return std::string(reinterpret_cast<char const*>(bytes.data()), bytes.size());
}

inline auto put_text(std::string_view text, byte_writer& out) -> void {
	out.put_bytes(std::as_bytes(std::span{text}));
}

/// Write a BER length and the text
inline auto put_prefixed_text(std::string_view text, byte_writer& out) -> void {
	out.put_ber_length(text.size());
	put_text(text, out);
}

/// Fail when bytes remain after the last element of a pack
template<typename T>
[[nodiscard]]
auto finish(byte_reader const& reader, T value) -> field<T> {
	if (!reader.empty()) {
		return std::unexpected(field_error::invalid_value);
	}
	return value;
}

}  // namespace detail

// =====================================================================================================================
// Location pack (tags 130 and 141)
// =====================================================================================================================

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

// =====================================================================================================================
// Image Horizon Pixel Pack (tag 81)
// =====================================================================================================================

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

// =====================================================================================================================
// Control Command (tag 115)
// =====================================================================================================================

/// One command from the ground control station to the aircraft
struct command_record {
	uint32_t id = 0;
	std::string text;
	std::optional<timestamp> time;

	friend auto operator==(command_record const&, command_record const&) -> bool = default;
};

/// Control Command: BER-OID ID, length-prefixed UTF-8 text, then an optional 8-byte time stamp
struct command_record_codec {
	using value_type = command_record;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<command_record> {
		auto reader = byte_reader{in};
		auto const id = detail::read_oid(reader);
		if (!id) {
			return std::unexpected(id.error());
		}
		auto const text = detail::read_prefixed(reader);
		if (!text) {
			return std::unexpected(text.error());
		}

		auto result = command_record{.id = *id, .text = detail::to_text(*text)};
		if (!reader.empty()) {
			auto const time = codec::timestamp{}.decode(reader.rest());
			if (!time) {
				return std::unexpected(time.error());
			}
			result.time = *time;
		}
		return result;
	}

	[[nodiscard]]
	auto encode(command_record const& value, byte_writer& out) const -> encode_result {
		out.put_ber_oid(value.id);
		detail::put_prefixed_text(value.text, out);
		return value.time ? codec::timestamp{}.encode(*value.time, out) : encode_result{};
	}

	[[nodiscard]]
	auto to_string(command_record const& value) const -> std::string {
		auto text = std::format("{{id: {}, text: \"{}\"", value.id, value.text);
		if (value.time) {
			text += std::format(", time: {}", codec::timestamp{}.to_string(*value.time));
		}
		return text + "}";
	}
};

// =====================================================================================================================
// Country Codes (tag 122)
// =====================================================================================================================

/// Countries of the platform and its operation. An absent country has no value.
struct countries {
	misb0102::classifying_country_coding coding_method = misb0102::classifying_country_coding::iso_3166_two_letter;
	std::optional<std::string> overflight_country;
	std::optional<std::string> operator_country;
	std::optional<std::string> country_of_manufacture;

	friend auto operator==(countries const&, countries const&) -> bool = default;
};

/**
 * @brief Country Codes: four length-prefixed elements (coding method, then three country codes)
 *
 * A zero length marks an absent country. The writer can omit the last two elements, or the last element.
 */
struct countries_codec {
	using value_type = countries;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<countries> {
		auto reader = byte_reader{in};
		auto const method = detail::read_prefixed(reader);
		if (!method) {
			return std::unexpected(method.error());
		}
		if (method->empty() || method->size() > sizeof(uint64_t)) {
			return std::unexpected(field_error::bad_length);
		}
		auto const raw_method = read_big_endian<uint64_t>(*method);
		if (raw_method > 0xFF) {
			return std::unexpected(field_error::invalid_value);
		}

		auto result = countries{.coding_method = static_cast<misb0102::classifying_country_coding>(raw_method)};
		for (auto* country : {&result.overflight_country, &result.operator_country, &result.country_of_manufacture}) {
			if (reader.empty()) {
				break;
			}
			auto const code = detail::read_prefixed(reader);
			if (!code) {
				return std::unexpected(code.error());
			}
			if (!code->empty()) {
				*country = detail::to_text(*code);
			}
		}
		return detail::finish(reader, std::move(result));
	}

	[[nodiscard]]
	auto encode(countries const& value, byte_writer& out) const -> encode_result {
		out.put_ber_length(1);
		out.put_u8(static_cast<uint8_t>(value.coding_method));

		auto const put_country = [&](std::optional<std::string> const& country) {
			detail::put_prefixed_text(country ? std::string_view{*country} : std::string_view{}, out);
		};

		put_country(value.overflight_country);
		if (value.operator_country || value.country_of_manufacture) {
			put_country(value.operator_country);
		}
		if (value.country_of_manufacture) {
			put_country(value.country_of_manufacture);
		}
		return {};
	}

	[[nodiscard]]
	auto to_string(countries const& value) const -> std::string {
		auto const method = misb0102::to_string(value.coding_method);
		auto const country = [](std::optional<std::string> const& code) {
			return code ? *code : std::string{"absent"};
		};
		return std::format(
		    "{{coding method: {}, overflight: {}, operator: {}, manufacture: {}}}",
		    method.empty() ? std::format("{}", static_cast<unsigned>(value.coding_method)) : std::string{method},
		    country(value.overflight_country),
		    country(value.operator_country),
		    country(value.country_of_manufacture)
		);
	}
};

// =====================================================================================================================
// Sensor Frame Rate Pack (tag 127)
// =====================================================================================================================

/// Frame rate as a ratio of integers
struct frame_rate {
	uint32_t numerator = 0;
	uint32_t denominator = 1;

	friend auto operator==(frame_rate const&, frame_rate const&) -> bool = default;
};

/// Sensor Frame Rate Pack: BER-OID numerator, then an optional BER-OID denominator (1 when absent)
struct frame_rate_codec {
	using value_type = frame_rate;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<frame_rate> {
		auto reader = byte_reader{in};
		auto const numerator = detail::read_oid(reader);
		if (!numerator) {
			return std::unexpected(numerator.error());
		}

		auto result = frame_rate{.numerator = *numerator};
		if (!reader.empty()) {
			auto const denominator = detail::read_oid(reader);
			if (!denominator) {
				return std::unexpected(denominator.error());
			}
			result.denominator = *denominator;
		}
		if (result.denominator == 0) {
			return std::unexpected(field_error::invalid_value);
		}
		return detail::finish(reader, result);
	}

	[[nodiscard]]
	auto encode(frame_rate const& value, byte_writer& out) const -> encode_result {
		if (value.denominator == 0) {
			return std::unexpected(field_error::invalid_value);
		}
		out.put_ber_oid(value.numerator);
		if (value.denominator != 1) {
			out.put_ber_oid(value.denominator);
		}
		return {};
	}

	[[nodiscard]]
	auto to_string(frame_rate const& value) const -> std::string {
		if (value.denominator == 1) {
			return std::format("{}", value.numerator);
		}
		return std::format(
		    "{}/{} ({:.3f})",
		    value.numerator,
		    value.denominator,
		    static_cast<double>(value.numerator) / static_cast<double>(value.denominator)
		);
	}
};

// =====================================================================================================================
// Wavelengths List (tag 128)
// =====================================================================================================================

/// One wavelength band of the sensors
struct wavelength_record {
	uint32_t id = 0;
	double minimum = 0.0;  ///< nm
	double maximum = 0.0;  ///< nm
	std::string name;

	friend auto operator==(wavelength_record const&, wavelength_record const&) -> bool = default;
};

/// Wavelength record: BER-OID ID, minimum and maximum (IMAPB, 4 bytes each), then the name in the remaining bytes
struct wavelength_record_codec {
	using value_type = wavelength_record;

	static constexpr codec::imapb wavelength_format{0.0, 1.0e9, 4};

	[[nodiscard]]
	auto decode(byte_span in) const -> field<wavelength_record> {
		auto reader = byte_reader{in};
		auto const id = detail::read_oid(reader);
		if (!id) {
			return std::unexpected(id.error());
		}

		auto bounds = std::array<double, 2>{};
		for (auto& bound : bounds) {
			auto const bytes = detail::read_exact(reader, wavelength_format.length());
			if (!bytes) {
				return std::unexpected(bytes.error());
			}
			auto const value = wavelength_format.decode(*bytes);
			if (!value) {
				return std::unexpected(value.error());
			}
			bound = *value;
		}

		return wavelength_record{
		    .id = *id,
		    .minimum = bounds[0],
		    .maximum = bounds[1],
		    .name = detail::to_text(reader.rest()),
		};
	}

	[[nodiscard]]
	auto encode(wavelength_record const& value, byte_writer& out) const -> encode_result {
		out.put_ber_oid(value.id);
		return wavelength_format.encode(value.minimum, out)
		    .and_then([&] { return wavelength_format.encode(value.maximum, out); })
		    .and_then([&] {
			    detail::put_text(value.name, out);
			    return encode_result{};
		    });
	}

	[[nodiscard]]
	auto to_string(wavelength_record const& value) const -> std::string {
		return std::format(
		    "{{id: {}, minimum: {:.3f}, maximum: {:.3f}, name: \"{}\"}}",
		    value.id,
		    value.minimum,
		    value.maximum,
		    value.name
		);
	}
};

// =====================================================================================================================
// Airbase Locations (tag 130)
// =====================================================================================================================

/// Take-off and recovery locations. An unknown location has no value.
struct airbases {
	std::optional<location> take_off;
	std::optional<location> recovery;

	friend auto operator==(airbases const&, airbases const&) -> bool = default;
};

/**
 * @brief Airbase Locations: two length-prefixed location packs (take-off, then recovery)
 *
 * A zero length marks an unknown location. When the recovery element is omitted, the recovery location is the
 * take-off location. Encoding omits the recovery element when the two locations are equal.
 */
struct airbases_codec {
	using value_type = airbases;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<airbases> {
		auto reader = byte_reader{in};
		auto result = airbases{};

		auto const take_off = read_location(reader);
		if (!take_off) {
			return std::unexpected(take_off.error());
		}
		result.take_off = *take_off;

		if (reader.empty()) {
			result.recovery = result.take_off;
			return result;
		}

		auto const recovery = read_location(reader);
		if (!recovery) {
			return std::unexpected(recovery.error());
		}
		result.recovery = *recovery;
		return detail::finish(reader, std::move(result));
	}

	[[nodiscard]]
	auto encode(airbases const& value, byte_writer& out) const -> encode_result {
		return put_location(value.take_off, out).and_then([&] {
			return value.recovery == value.take_off ? encode_result{} : put_location(value.recovery, out);
		});
	}

	[[nodiscard]]
	auto to_string(airbases const& value) const -> std::string {
		auto const text = [](std::optional<location> const& place) {
			return place ? location_codec{}.to_string(*place) : std::string{"unknown"};
		};
		return std::format("{{take-off: {}, recovery: {}}}", text(value.take_off), text(value.recovery));
	}

private:
	[[nodiscard]]
	static auto read_location(byte_reader& reader) -> field<std::optional<location>> {
		auto const bytes = detail::read_prefixed(reader);
		if (!bytes) {
			return std::unexpected(bytes.error());
		}
		if (bytes->empty()) {
			return std::optional<location>{};
		}
		return location_codec{}.decode(*bytes).transform([](location place) { return std::optional{place}; });
	}

	[[nodiscard]]
	static auto put_location(std::optional<location> const& place, byte_writer& out) -> encode_result {
		if (!place) {
			out.put_ber_length(0);
			return {};
		}
		return out.put_length_prefixed([&] { return location_codec{}.encode(*place, out); });
	}
};

// =====================================================================================================================
// Payload List (tag 138)
// =====================================================================================================================

/// Kind of a payload
enum class payload_type : uint32_t {
	electro_optical = 0,
	lidar = 1,
	radar = 2,
	sigint = 3,
};

[[nodiscard]]
constexpr auto to_string(payload_type value) -> std::string_view {
	using enum payload_type;
	switch (value) {
		case electro_optical:
			return "Electro Optical MI Sensor";
		case lidar:
			return "LIDAR";
		case radar:
			return "RADAR";
		case sigint:
			return "SIGINT";
	}
	return {};
}

/// One payload of the platform
struct payload_record {
	uint32_t id = 0;
	payload_type type = payload_type::electro_optical;  ///< Can hold a value that `payload_type` does not name
	std::string name;

	friend auto operator==(payload_record const&, payload_record const&) -> bool = default;
};

/// Payload record: BER-OID ID, BER-OID type, then the length-prefixed name
struct payload_record_codec {
	using value_type = payload_record;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<payload_record> {
		auto reader = byte_reader{in};
		auto const id = detail::read_oid(reader);
		if (!id) {
			return std::unexpected(id.error());
		}
		auto const type = detail::read_oid(reader);
		if (!type) {
			return std::unexpected(type.error());
		}
		auto const name = detail::read_prefixed(reader);
		if (!name) {
			return std::unexpected(name.error());
		}

		return detail::finish(
		    reader,
		    payload_record{.id = *id, .type = static_cast<payload_type>(*type), .name = detail::to_text(*name)}
		);
	}

	[[nodiscard]]
	auto encode(payload_record const& value, byte_writer& out) const -> encode_result {
		out.put_ber_oid(value.id);
		out.put_ber_oid(static_cast<uint32_t>(value.type));
		detail::put_prefixed_text(value.name, out);
		return {};
	}

	[[nodiscard]]
	auto to_string(payload_record const& value) const -> std::string {
		auto const type = misb0601::to_string(value.type);
		return std::format(
		    "{{id: {}, type: {}, name: \"{}\"}}",
		    value.id,
		    type.empty() ? std::format("{}", static_cast<uint32_t>(value.type)) : std::string{type},
		    value.name
		);
	}
};

/// Payloads of the platform
struct payloads {
	uint32_t count = 0;  ///< Number of payloads on the platform
	std::vector<payload_record> records;

	friend auto operator==(payloads const&, payloads const&) -> bool = default;
};

/// Payload List: BER-OID payload count, then a series of payload records
struct payloads_codec {
	using value_type = payloads;

	static constexpr codec::series<payload_record_codec> records_format{};

	[[nodiscard]]
	auto decode(byte_span in) const -> field<payloads> {
		auto reader = byte_reader{in};
		auto const count = detail::read_oid(reader);
		if (!count) {
			return std::unexpected(count.error());
		}
		return records_format.decode(reader.rest()).transform([&](std::vector<payload_record> records) {
			return payloads{.count = *count, .records = std::move(records)};
		});
	}

	[[nodiscard]]
	auto encode(payloads const& value, byte_writer& out) const -> encode_result {
		out.put_ber_oid(value.count);
		return records_format.encode(value.records, out);
	}

	[[nodiscard]]
	auto to_string(payloads const& value) const -> std::string {
		return std::format("{{count: {}, payloads: {}}}", value.count, records_format.to_string(value.records));
	}
};

// =====================================================================================================================
// Weapons Stores (tag 140)
// =====================================================================================================================

/// General status of a weapons store
enum class weapon_status : uint8_t {
	off = 0,
	initialization = 1,
	ready_degraded = 2,
	ready_all_up_round = 3,
	launch = 4,
	free_flight = 5,
	abort = 6,
	miss_fire = 7,
	hang_fire = 8,
	jettisoned = 9,
	stepped_over = 10,
	no_status_available = 11,
};

[[nodiscard]]
constexpr auto to_string(weapon_status value) -> std::string_view {
	using enum weapon_status;
	switch (value) {
		case off:
			return "Off";
		case initialization:
			return "Initialization";
		case ready_degraded:
			return "Ready/Degraded";
		case ready_all_up_round:
			return "Ready/All Up Round";
		case launch:
			return "Launch";
		case free_flight:
			return "Free Flight";
		case abort:
			return "Abort";
		case miss_fire:
			return "Miss Fire";
		case hang_fire:
			return "Hang Fire";
		case jettisoned:
			return "Jettisoned";
		case stepped_over:
			return "Stepped Over";
		case no_status_available:
			return "No Status Available";
	}
	return {};
}

/// One weapons store and its status
struct weapons_store {
	// Bits of `engagement_status`
	static constexpr uint8_t fuze_enabled = 0x01;
	static constexpr uint8_t laser_enabled = 0x02;
	static constexpr uint8_t target_enabled = 0x04;
	static constexpr uint8_t weapon_armed = 0x08;

	uint32_t station_id = 0;
	uint32_t hardpoint_id = 0;
	uint32_t carriage_id = 0;
	uint32_t store_id = 0;
	weapon_status general_status = weapon_status::off;  ///< Can hold a value that `weapon_status` does not name
	uint8_t engagement_status = 0;
	std::string weapon_type;

	friend auto operator==(weapons_store const&, weapons_store const&) -> bool = default;
};

/**
 * @brief Weapons store record: four BER-OID IDs, a BER-OID status, then the length-prefixed weapon type
 *
 * The status holds the general status in bits 0 to 7 and the engagement status in bits 8 to 11.
 */
struct weapons_store_codec {
	using value_type = weapons_store;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<weapons_store> {
		auto reader = byte_reader{in};
		auto ids = std::array<uint32_t, 5>{};
		for (auto& id : ids) {
			auto const value = detail::read_oid(reader);
			if (!value) {
				return std::unexpected(value.error());
			}
			id = *value;
		}

		auto const status = ids[4];
		if ((status >> 12U) != 0) {
			return std::unexpected(field_error::invalid_value);
		}
		auto const type = detail::read_prefixed(reader);
		if (!type) {
			return std::unexpected(type.error());
		}

		return detail::finish(
		    reader,
		    weapons_store{
		        .station_id = ids[0],
		        .hardpoint_id = ids[1],
		        .carriage_id = ids[2],
		        .store_id = ids[3],
		        .general_status = static_cast<weapon_status>(status & 0xFFU),
		        .engagement_status = static_cast<uint8_t>((status >> 8U) & 0x0FU),
		        .weapon_type = detail::to_text(*type),
		    }
		);
	}

	[[nodiscard]]
	auto encode(weapons_store const& value, byte_writer& out) const -> encode_result {
		if (value.engagement_status > 0x0F) {
			return std::unexpected(field_error::invalid_value);
		}
		out.put_ber_oid(value.station_id);
		out.put_ber_oid(value.hardpoint_id);
		out.put_ber_oid(value.carriage_id);
		out.put_ber_oid(value.store_id);
		out.put_ber_oid(
		    (static_cast<uint32_t>(value.engagement_status) << 8U) | static_cast<uint32_t>(value.general_status)
		);
		detail::put_prefixed_text(value.weapon_type, out);
		return {};
	}

	[[nodiscard]]
	auto to_string(weapons_store const& value) const -> std::string {
		auto const status = misb0601::to_string(value.general_status);
		return std::format(
		    "{{station: {}, hardpoint: {}, carriage: {}, store: {}, status: {}, engagement: 0x{:X}, type: \"{}\"}}",
		    value.station_id,
		    value.hardpoint_id,
		    value.carriage_id,
		    value.store_id,
		    status.empty() ? std::format("{}", static_cast<unsigned>(value.general_status)) : std::string{status},
		    value.engagement_status,
		    value.weapon_type
		);
	}
};

// =====================================================================================================================
// Waypoint List (tag 141)
// =====================================================================================================================

/// One navigation waypoint
struct waypoint_record {
	// Bits of `info`
	static constexpr uint8_t manual_mode = 0x01;  ///< Set: manual. Clear: automated.
	static constexpr uint8_t ad_hoc_source = 0x02;  ///< Set: ad hoc. Clear: pre-planned.

	uint32_t id = 0;
	int16_t order = 0;  ///< Prosecution order
	std::optional<uint8_t> info;
	std::optional<location> position;  ///< Needs `info`

	friend auto operator==(waypoint_record const&, waypoint_record const&) -> bool = default;
};

/// Waypoint record: BER-OID ID, 2-byte signed order, then an optional 1-byte info and an optional location pack
struct waypoint_record_codec {
	using value_type = waypoint_record;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<waypoint_record> {
		auto reader = byte_reader{in};
		auto const id = detail::read_oid(reader);
		if (!id) {
			return std::unexpected(id.error());
		}
		auto const order = detail::read_exact(reader, sizeof(int16_t));
		if (!order) {
			return std::unexpected(order.error());
		}

		auto result = waypoint_record{.id = *id, .order = read_big_endian<int16_t>(*order)};
		if (reader.empty()) {
			return result;
		}

		auto const info = detail::read_oid(reader);
		if (!info) {
			return std::unexpected(info.error());
		}
		if (*info > 0x7F) {
			return std::unexpected(field_error::invalid_value);
		}
		result.info = static_cast<uint8_t>(*info);
		if (reader.empty()) {
			return result;
		}

		auto const position = location_codec{}.decode(reader.rest());
		if (!position) {
			return std::unexpected(position.error());
		}
		result.position = *position;
		return result;
	}

	[[nodiscard]]
	auto encode(waypoint_record const& value, byte_writer& out) const -> encode_result {
		if ((value.info && *value.info > 0x7F) || (value.position && !value.info)) {
			return std::unexpected(field_error::invalid_value);
		}

		out.put_ber_oid(value.id);
		out.put_big_endian(value.order);
		if (value.info) {
			out.put_u8(*value.info);
		}
		return value.position ? location_codec{}.encode(*value.position, out) : encode_result{};
	}

	[[nodiscard]]
	auto to_string(waypoint_record const& value) const -> std::string {
		auto text = std::format("{{id: {}, order: {}", value.id, value.order);
		if (value.info) {
			text += std::format(", info: 0x{:X}", *value.info);
		}
		if (value.position) {
			text += std::format(", position: {}", location_codec{}.to_string(*value.position));
		}
		return text + "}";
	}
};

// =====================================================================================================================
// View Domain (tag 142)
// =====================================================================================================================

/// Start angle and angular range of one sensor axis
struct view_interval {
	double start = 0.0;  ///< deg
	double range = 0.0;  ///< deg

	friend auto operator==(view_interval const&, view_interval const&) -> bool = default;
};

/// Possible sensor azimuth, elevation, and roll angles. An axis without a limit has no value.
struct view_ranges {
	std::optional<view_interval> azimuth;
	std::optional<view_interval> elevation;
	std::optional<view_interval> roll;

	friend auto operator==(view_ranges const&, view_ranges const&) -> bool = default;
};

/**
 * @brief View Domain: three length-prefixed intervals (azimuth, elevation, roll)
 *
 * Each interval holds two IMAPB values of half its length: the start, then the range from 0 to 360 deg. The start
 * range is 0 to 360 deg for azimuth and roll, and -180 to 180 deg for elevation. A zero length marks an absent
 * interval, and the writer can omit the last intervals. Encoding writes 3 bytes for each value.
 */
struct view_ranges_codec {
	using value_type = view_ranges;

	static constexpr auto value_length = size_t{3};
	static constexpr codec::imapb azimuth_format{0.0, 360.0, value_length};
	static constexpr codec::imapb elevation_format{-180.0, 180.0, value_length};
	static constexpr codec::imapb roll_format{0.0, 360.0, value_length};
	static constexpr codec::imapb range_format{0.0, 360.0, value_length};

	[[nodiscard]]
	auto decode(byte_span in) const -> field<view_ranges> {
		auto reader = byte_reader{in};
		auto result = view_ranges{};

		auto const axes = std::array{
		    axis{&result.azimuth, &azimuth_format},
		    axis{&result.elevation, &elevation_format},
		    axis{&result.roll, &roll_format},
		};
		for (auto const& [interval, start_format] : axes) {
			if (reader.empty()) {
				break;
			}
			auto const bytes = detail::read_prefixed(reader);
			if (!bytes) {
				return std::unexpected(bytes.error());
			}
			if (bytes->empty()) {
				continue;
			}
			if (bytes->size() % 2 != 0) {
				return std::unexpected(field_error::bad_length);
			}

			auto const half = bytes->size() / 2;
			auto const start = start_format->decode(bytes->first(half));
			if (!start) {
				return std::unexpected(start.error());
			}
			auto const range = range_format.decode(bytes->subspan(half));
			if (!range) {
				return std::unexpected(range.error());
			}
			*interval = view_interval{.start = *start, .range = *range};
		}
		return detail::finish(reader, result);
	}

	[[nodiscard]]
	auto encode(view_ranges const& value, byte_writer& out) const -> encode_result {
		auto const axes = std::array{
		    const_axis{&value.azimuth, &azimuth_format},
		    const_axis{&value.elevation, &elevation_format},
		    const_axis{&value.roll, &roll_format},
		};

		// Omit the absent intervals at the end
		auto count = axes.size();
		while (count > 0 && !*axes[count - 1].interval) {
			--count;
		}

		for (auto i = size_t{0}; i < count; ++i) {
			auto const& [interval, start_format] = axes[i];
			if (!*interval) {
				out.put_ber_length(0);
				continue;
			}
			auto status = out.put_length_prefixed([&] {
				return start_format->encode((*interval)->start, out).and_then([&] {
					return range_format.encode((*interval)->range, out);
				});
			});
			if (!status) {
				return status;
			}
		}
		return {};
	}

	[[nodiscard]]
	auto to_string(view_ranges const& value) const -> std::string {
		auto const text = [](std::optional<view_interval> const& interval) {
			return interval ? std::format("{{start: {:.4f}, range: {:.4f}}}", interval->start, interval->range)
			                : std::string{"absent"};
		};
		return std::format(
		    "{{azimuth: {}, elevation: {}, roll: {}}}",
		    text(value.azimuth),
		    text(value.elevation),
		    text(value.roll)
		);
	}

private:
	struct axis {
		std::optional<view_interval>* interval;
		codec::imapb const* start_format;
	};

	struct const_axis {
		std::optional<view_interval> const* interval;
		codec::imapb const* start_format;
	};
};

}  // namespace klv::misb0601
