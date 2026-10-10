// ST 0809.2 gives no example packet. A separate script calculated the bytes of the example packet below.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

#include "klv_test_util.hpp"
#include "klv/misb0809.hpp"
#include "klv/packet.hpp"

using namespace klv;
using klv_test::hex;
using klv_test::micros;

namespace met = klv::misb0809;

namespace {

/// Meteorological Metadata Local Set packet with all 39 items. The value has 259 bytes.
constexpr auto weather_packet = std::string_view{
    "06 0E 2B 34 02 2B 01 01 0E 01 03 01 0E 00 00 00 82 01 03 "
    "01 08 00 06 0A 24 18 20 22 40 "  // 2023-11-14T22:13:20.123456Z
    "02 02 00 02 "  // ST 0809.2
    "03 0A 53 74 61 74 69 6F 6E 20 31 32 "  // "Station 12"
    "04 08 40 43 5C 00 00 00 00 00 "  // 38.71875 deg
    "05 08 C0 53 4A 00 00 00 00 00 "  // -77.15625 deg
    "06 04 47 C5 E6 80 "  // 101325 Pa
    "07 04 41 FC 00 00 "  // 31.5 degC
    "08 04 41 C2 00 00 "  // 24.25 degC
    "09 04 42 16 00 00 "  // 37.5 %
    "0A 07 63 75 6D 75 6C 75 73 "  // "cumulus"
    "0B 04 44 9C 40 00 "  // 1250 m
    "0C 04 45 2B E0 00 "  // 2750 m
    "0D 04 44 BB 80 00 "  // 1500 m
    "0E 04 43 D2 40 00 "  // 420.5 ppm
    "0F 04 3D 80 00 00 "  // 0.0625 ppm
    "10 04 41 B4 00 00 "  // 22.5 degC
    "11 04 41 B4 00 00 "  // 22.5 degC
    "12 04 41 86 00 00 "  // 16.75 degC
    "13 04 42 5E 00 00 "  // 55.5 %
    "14 04 41 54 00 00 "  // 13.25 degC
    "15 04 3C 00 00 00 "  // 0.0078125 degC/m
    "16 0D 70 61 72 74 6C 79 20 63 6C 6F 75 64 79 "  // "partly cloudy"
    "17 04 45 AB E0 00 "  // 5500 K
    "18 04 68 61 7A 65 "  // "haze"
    "19 04 41 48 00 00 "  // 12.5 km
    "1A 04 46 6A 60 00 "  // 15000 lx
    "1B 04 47 A6 04 00 "  // 85000 lx
    "1C 04 47 C3 50 00 "  // 100000 lx
    "1D 01 01 "  // Fog present
    "1E 04 42 20 00 00 "  // 40 m
    "1F 04 41 48 00 00 "  // 12.5 %
    "20 04 3C 80 00 00 "  // 0.015625 1/m
    "21 04 72 61 69 6E "  // "rain"
    "22 04 40 20 00 00 "  // 2.5 mm/h
    "23 04 40 A8 00 00 "  // 5.25 cm
    "24 04 40 F0 00 00 "  // 7.5 urad
    "25 04 42 28 00 00 "  // 42 Hz
    "26 04 3E 00 00 00 "  // 0.125
    "27 04 41 94 00 00"  // 18.5 Hz
};

[[nodiscard]]
auto make_weather() -> record<met::meteorological_ls> {
	auto rec = record<met::meteorological_ls>{};
	rec.set<met::precision_time_stamp>(micros(1'700'000'000'123'456));
	rec.set<met::version>(uint16_t{2});
	rec.set<met::reporting_id>("Station 12");
	rec.set<met::device_latitude>(38.71875);
	rec.set<met::device_longitude>(-77.15625);
	rec.set<met::barometric_pressure>(101325.0F);
	rec.set<met::black_globe_temperature>(31.5F);
	rec.set<met::wet_bulb_temperature_heat_index>(24.25F);
	rec.set<met::cloud_cover_percentage>(37.5F);
	rec.set<met::cloud_sky_cover_type>("cumulus");
	rec.set<met::cloud_base_height>(1250.0F);
	rec.set<met::cloud_top_height>(2750.0F);
	rec.set<met::cloud_thickness>(1500.0F);
	rec.set<met::carbon_dioxide_concentration>(420.5F);
	rec.set<met::ozone_concentration>(0.0625F);
	rec.set<met::temperature>(22.5F);
	rec.set<met::dry_bulb_temperature>(22.5F);
	rec.set<met::wet_bulb_temperature>(16.75F);
	rec.set<met::relative_humidity>(55.5F);
	rec.set<met::dew_point>(13.25F);
	rec.set<met::temperature_lapse_rate>(0.0078125F);
	rec.set<met::lighting_conditions>("partly cloudy");
	rec.set<met::ambient_light_color_temperature>(5500.0F);
	rec.set<met::visibility_conditions>("haze");
	rec.set<met::visibility>(12.5F);
	rec.set<met::solar_illumination_diffuse>(15000.0F);
	rec.set<met::solar_illumination_direct>(85000.0F);
	rec.set<met::total_illumination_direct>(100000.0F);
	rec.set<met::fog_presence>(met::fog::present);
	rec.set<met::fog_thickness>(40.0F);
	rec.set<met::fog_cover_percentage>(12.5F);
	rec.set<met::fog_extinction_coefficient>(0.015625F);
	rec.set<met::precipitation_type>("rain");
	rec.set<met::precipitation_rate>(2.5F);
	rec.set<met::r_naught>(5.25F);
	rec.set<met::theta_naught>(7.5F);
	rec.set<met::greenwood_frequency>(42.0F);
	rec.set<met::rytov_parameter>(0.125F);
	rec.set<met::tyler_tracking_frequency>(18.5F);
	return rec;
}

}  // namespace

TEST_CASE("KLV ST 0809 Meteorological set packet", "[klv][misb0809]") {
	auto const decoded = decode_packet<met::meteorological_ls>(hex(weather_packet));
	REQUIRE(decoded);
	CHECK(decoded->unknown().empty());
	CHECK(*decoded == make_weather());
	CHECK(encode_packet(make_weather()) == hex(weather_packet));

	auto const text = describe(*decoded);
	auto const entry = [&](std::string_view name) {
		auto const found = std::ranges::find(text, name, &field_text::name);
		REQUIRE(found != text.end());
		return *found;
	};
	CHECK(entry("Precision Time Stamp").value == std::string{"2023-11-14T22:13:20.123456Z"});
	CHECK(entry("Device Longitude").value == std::string{"-77.15625"});
	CHECK(entry("Device Longitude").units == "deg");
	CHECK(entry("Barometric Pressure").value == std::string{"101325"});
	CHECK(entry("Fog Presence").value == std::string{"Present"});
	CHECK(entry("Theta Naught").units == "urad");
}

TEST_CASE("KLV ST 0809 encoding writes the time stamp and the version first", "[klv][misb0809]") {
	// The version comes before the time stamp and uses one byte. Decoding accepts both.
	auto const bytes = hex(
	    "06 0E 2B 34 02 2B 01 01 0E 01 03 01 0E 00 00 00 10 "
	    "1D 01 00 02 01 02 01 08 00 06 0A 24 18 20 22 40"
	);
	auto const decoded = decode_packet<met::meteorological_ls>(bytes);
	REQUIRE(decoded);
	CHECK(decoded->get<met::precision_time_stamp>() == micros(1'700'000'000'123'456));
	CHECK(decoded->get<met::version>() == uint16_t{2});
	CHECK(decoded->get<met::fog_presence>() == met::fog::absent);

	CHECK(
	    encode_packet(*decoded)
	    == hex(
	        "06 0E 2B 34 02 2B 01 01 0E 01 03 01 0E 00 00 00 11 "
	        "01 08 00 06 0A 24 18 20 22 40 02 02 00 02 1D 01 00"
	    )
	);
}

TEST_CASE("KLV ST 0809 Fog Presence", "[klv][misb0809]") {
	constexpr auto format = met::fog_presence::format;

	CHECK(format.decode(hex("00")) == met::fog::absent);
	CHECK(format.decode(hex("01")) == met::fog::present);
	CHECK(format.decode(hex("02")) == std::unexpected(field_error::invalid_value));
	CHECK(format.decode(hex("00 01")) == std::unexpected(field_error::bad_length));

	CHECK(encode_value(format, met::fog::present) == hex("01"));
	CHECK(encode_value(format, met::fog{2}) == std::unexpected(field_error::invalid_value));
	CHECK(format.to_string(met::fog::absent) == "Absent");
}

TEST_CASE("KLV ST 0809 item lengths", "[klv][misb0809]") {
	// The latitude and the longitude use 8 bytes. The other real numbers use 4 bytes.
	CHECK(met::device_latitude::format.decode(hex("42 1A DC 00")) == std::unexpected(field_error::bad_length));
	CHECK(
	    met::barometric_pressure::format.decode(hex("40 F8 BC D0 00 00 00 00"))
	    == std::unexpected(field_error::bad_length)
	);
	CHECK(
	    met::precision_time_stamp::format.decode(hex("06 0A 24 18 20 22 40"))
	    == std::unexpected(field_error::bad_length)
	);

	// A text item holds at most 40 bytes
	auto const longest = std::string(40, 'x');
	CHECK(encode_value(met::reporting_id::format, longest).has_value());
	CHECK(encode_value(met::reporting_id::format, longest + "x") == std::unexpected(field_error::bad_length));
}
