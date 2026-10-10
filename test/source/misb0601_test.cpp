#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

#include "klv_test_util.hpp"
#include "klv/misb0102.hpp"
#include "klv/misb0601.hpp"
#include "klv/misb0806.hpp"
#include "klv/misb0903.hpp"
#include "klv/misb1002.hpp"
#include "klv/misb1204.hpp"
#include "klv/misb1206.hpp"
#include "klv/misb1601.hpp"
#include "klv/misb1602.hpp"
#include "klv/packet.hpp"

using namespace klv;
using Catch::Approx;
using klv_test::append_field;
using klv_test::append_packet;
using klv_test::hex;
using klv_test::make_bytes;
using klv_test::micros;

namespace kv = klv::misb0601;
namespace rvt = klv::misb0806;
namespace sec = klv::misb0102;
namespace vmti = klv::misb0903;

using uas_record = record<kv::uas_datalink_ls>;

static_assert(keyed_set_definition<kv::uas_datalink_ls> && local_set_definition<kv::uas_datalink_ls>);
static_assert(has_checksum<kv::uas_datalink_ls>);
static_assert(kv::uas_datalink_ls::fields::size == 141);

namespace {

/// Append the checksum field and its value, computed over `packet` plus the checksum key and length
auto append_checksum(byte_vector& packet) -> void {
	packet.push_back(std::byte{0x01});
	packet.push_back(std::byte{0x02});
	auto const checksum = packet_checksum(packet);
	packet.push_back(static_cast<std::byte>(checksum >> 8U));
	packet.push_back(static_cast<std::byte>(checksum & 0xFFU));
}

/// A UAS Datalink LS packet with the given value fields and a valid checksum
[[nodiscard]]
auto make_uas_packet(byte_vector const& fields) -> byte_vector {
	// The value length includes the 4 checksum bytes
	auto value = fields;
	value.resize(fields.size() + 4);

	auto packet = byte_vector{};
	append_packet(packet, kv::uas_datalink_ls::key, value);
	packet.resize(packet.size() - 4);
	append_checksum(packet);
	return packet;
}

/// Tags of the items of an encoded local set, in order
[[nodiscard]]
auto tags_of(byte_span bytes) -> std::vector<uint32_t> {
	auto tags = std::vector<uint32_t>{};
	auto reader = byte_reader{bytes};
	while (!reader.empty()) {
		auto const tag = reader.read_ber_oid();
		auto const length = reader.read_ber_length();
		if (!tag || !length || !reader.take(*length)) {
			break;
		}
		tags.push_back(*tag);
	}
	return tags;
}

}  // namespace

TEST_CASE("KLV ST 0601 example values decode", "[klv][misb0601]") {
	// Raw values from the example Local Set in MISB ST 0601
	auto fields = byte_vector{};
	append_field(fields, 2, hex("00 04 59 F4 A6 AA 4A A8"));
	append_field(fields, 3, hex("4D 69 73 73 69 6F 6E 20 31 32"));  // "Mission 12"
	append_field(fields, 4, hex("41 46 2D 31 30 31"));  // "AF-101"
	append_field(fields, 5, hex("71 C2"));
	append_field(fields, 6, hex("FD 3D"));
	append_field(fields, 7, hex("08 B8"));
	append_field(fields, 8, hex("93"));
	append_field(fields, 9, hex("9F"));
	append_field(fields, 10, hex("4D 51 31 2D 42"));  // "MQ1-B"
	append_field(fields, 11, hex("45 4F"));  // "EO"
	append_field(fields, 12, hex("57 47 53 2D 38 34"));
	append_field(fields, 13, hex("55 95 B6 6D"));
	append_field(fields, 14, hex("5B 53 60 C4"));
	append_field(fields, 15, hex("C2 21"));
	append_field(fields, 16, hex("CD 9C"));
	append_field(fields, 17, hex("D9 17"));
	append_field(fields, 18, hex("72 4A 0A 20"));
	append_field(fields, 19, hex("87 F8 4B 86"));
	append_field(fields, 20, hex("7D C5 5E CE"));
	append_field(fields, 21, hex("03 83 09 26"));
	append_field(fields, 22, hex("12 81"));
	append_field(fields, 23, hex("F1 01 A2 29"));
	append_field(fields, 24, hex("14 BC 08 2B"));
	append_field(fields, 25, hex("34 F3"));

	auto const uas = decode_packet<kv::uas_datalink_ls>(make_uas_packet(fields));
	REQUIRE(uas);

	auto const value = [&]<typename E>(std::type_identity<E>) {
		auto const& field = uas->get<E>();
		REQUIRE(field);
		return *field;
	};

	CHECK(value(std::type_identity<kv::precision_time_stamp>{}).time_since_epoch().count() == 1'224'807'209'913'000);
	CHECK(value(std::type_identity<kv::mission_id>{}) == "Mission 12");
	CHECK(value(std::type_identity<kv::platform_tail_number>{}) == "AF-101");
	CHECK(value(std::type_identity<kv::platform_heading_angle>{}) == Approx(159.9743648).margin(1e-6));
	CHECK(value(std::type_identity<kv::platform_pitch_angle>{}) == Approx(-0.4315317).margin(1e-6));
	CHECK(value(std::type_identity<kv::platform_roll_angle>{}) == Approx(3.4058657).margin(1e-6));
	CHECK(value(std::type_identity<kv::platform_true_airspeed>{}) == 147);
	CHECK(value(std::type_identity<kv::platform_indicated_airspeed>{}) == 159);
	CHECK(value(std::type_identity<kv::platform_designation>{}) == "MQ1-B");
	CHECK(value(std::type_identity<kv::image_source_sensor>{}) == "EO");
	CHECK(value(std::type_identity<kv::image_coordinate_system>{}) == "WGS-84");
	CHECK(value(std::type_identity<kv::sensor_latitude>{}) == Approx(60.17682297).margin(1e-7));
	CHECK(value(std::type_identity<kv::sensor_longitude>{}) == Approx(128.42675904).margin(1e-7));
	CHECK(value(std::type_identity<kv::sensor_true_altitude>{}) == Approx(14190.71946).margin(1e-4));
	CHECK(value(std::type_identity<kv::sensor_horizontal_field_of_view>{}) == Approx(144.5712978).margin(1e-6));
	CHECK(value(std::type_identity<kv::sensor_vertical_field_of_view>{}) == Approx(152.6436255).margin(1e-6));
	CHECK(value(std::type_identity<kv::sensor_relative_azimuth_angle>{}) == Approx(160.71921144).margin(1e-7));
	CHECK(value(std::type_identity<kv::sensor_relative_elevation_angle>{}) == Approx(-168.79232483).margin(1e-7));
	CHECK(value(std::type_identity<kv::sensor_relative_roll_angle>{}) == Approx(176.86543765).margin(1e-7));
	CHECK(value(std::type_identity<kv::slant_range>{}) == Approx(68590.98330).margin(1e-4));
	CHECK(value(std::type_identity<kv::target_width>{}) == Approx(722.819867).margin(1e-5));
	CHECK(value(std::type_identity<kv::frame_center_latitude>{}) == Approx(-10.54238863).margin(1e-7));
	CHECK(value(std::type_identity<kv::frame_center_longitude>{}) == Approx(29.15789012).margin(1e-7));
	CHECK(value(std::type_identity<kv::frame_center_elevation>{}) == Approx(3216.03723).margin(1e-4));

	CHECK(uas->get<kv::checksum>().has_value());
	CHECK(uas->unknown().empty());
}

TEST_CASE("KLV ST 0601 zero angles are valid values", "[klv][misb0601]") {
	// Heading 0 (north), sensor azimuth 0, and sensor roll 0 are common. The old library decoded them as errors.
	auto fields = byte_vector{};
	append_field(fields, 5, hex("00 00"));
	append_field(fields, 18, hex("00 00 00 00"));
	append_field(fields, 20, hex("00 00 00 00"));
	append_field(fields, 16, hex("00 00"));

	auto const uas = decode_packet<kv::uas_datalink_ls>(make_uas_packet(fields));
	REQUIRE(uas);
	CHECK(uas->get<kv::platform_heading_angle>() == 0.0);
	CHECK(uas->get<kv::sensor_relative_azimuth_angle>() == 0.0);
	CHECK(uas->get<kv::sensor_relative_roll_angle>() == 0.0);
	CHECK(uas->get<kv::sensor_horizontal_field_of_view>() == 0.0);
}

TEST_CASE("KLV ST 0601 decodes a nested ST 0102 security set in tag 48", "[klv][misb0601]") {
	auto security = byte_vector{};
	append_field(security, 1, hex("01"));
	append_field(security, 2, hex("01"));
	append_field(security, 3, hex("2F 2F 55 53"));  // "//US"
	append_field(security, 12, hex("40"));
	append_field(security, 22, hex("00 0C"));

	auto fields = byte_vector{};
	append_field(fields, 48, security);

	auto const uas = decode_packet<kv::uas_datalink_ls>(make_uas_packet(fields));
	REQUIRE(uas);

	auto const& nested = uas->get<kv::security_local_set>();
	REQUIRE(nested);
	CHECK(nested->get<sec::security_classification>() == sec::classification::unclassified);
	CHECK(
	    nested->get<sec::classifying_country_coding_method>() == sec::classifying_country_coding::iso_3166_two_letter
	);
	CHECK(nested->get<sec::classifying_country>() == std::string{"//US"});
	CHECK(nested->get<sec::object_country_coding_method>() == sec::object_country_coding::genc_admin_sub);
	CHECK(nested->get<sec::version>() == uint16_t{12});

	auto const text = describe(*uas);
	REQUIRE(text.size() >= 2);
	CHECK(text[0].name == "Security Local Set.Security Classification");
	CHECK(text[0].value == std::string{"UNCLASSIFIED"});
}

TEST_CASE("KLV ST 0601 record encodes the nested set", "[klv][misb0601]") {
	auto security = record<sec::security_ls>{};
	security.set<sec::security_classification>(sec::classification::secret);
	security.set<sec::classifying_country>("//US");

	auto uas = uas_record{};
	uas.set<kv::precision_time_stamp>(timestamp{std::chrono::microseconds{1'000'000}});
	uas.set<kv::security_local_set>(security);

	auto const bytes = encode_packet(uas);
	REQUIRE(bytes);

	auto const decoded = decode_packet<kv::uas_datalink_ls>(*bytes);
	REQUIRE(decoded);
	CHECK(decoded->get<kv::security_local_set>() == security);
	CHECK(decoded->get<kv::precision_time_stamp>() == timestamp{std::chrono::microseconds{1'000'000}});
}

TEST_CASE("KLV ST 0601 extended mapped fields decode at the ends of their ranges", "[klv][misb0601]") {
	auto fields = byte_vector{};
	append_field(fields, 26, hex("7F FF"));
	append_field(fields, 27, hex("80 01"));
	append_field(fields, 35, hex("FF FF"));
	append_field(fields, 36, hex("FF"));
	append_field(fields, 37, hex("FF FF"));
	append_field(fields, 38, hex("00 00"));
	append_field(fields, 43, hex("7F"));
	append_field(fields, 45, hex("FF FF"));
	append_field(fields, 51, hex("7F FF"));
	append_field(fields, 55, hex("FF"));
	append_field(fields, 57, hex("FF FF FF FF"));
	append_field(fields, 58, hex("FF FF"));
	append_field(fields, 69, hex("FF FF"));
	append_field(fields, 79, hex("80 01"));
	append_field(fields, 83, hex("7F FF FF FF"));
	append_field(fields, 93, hex("80 00 00 01"));

	auto const uas = decode_packet<kv::uas_datalink_ls>(make_uas_packet(fields));
	REQUIRE(uas);

	auto const value = [&]<typename E>(std::type_identity<E>) {
		auto const& field = uas->get<E>();
		REQUIRE(field);
		return *field;
	};

	CHECK(value(std::type_identity<kv::offset_corner_latitude_point_1>{}) == Approx(0.075));
	CHECK(value(std::type_identity<kv::offset_corner_longitude_point_1>{}) == Approx(-0.075));
	CHECK(value(std::type_identity<kv::wind_direction>{}) == Approx(360.0));
	CHECK(value(std::type_identity<kv::wind_speed>{}) == Approx(100.0));
	CHECK(value(std::type_identity<kv::static_pressure>{}) == Approx(5'000.0));
	CHECK(value(std::type_identity<kv::density_altitude>{}) == Approx(-900.0));
	CHECK(value(std::type_identity<kv::target_track_gate_width>{}) == Approx(254.0));
	CHECK(value(std::type_identity<kv::target_error_estimate_ce90>{}) == Approx(4'095.0));
	CHECK(value(std::type_identity<kv::platform_vertical_speed>{}) == Approx(180.0));
	CHECK(value(std::type_identity<kv::relative_humidity>{}) == Approx(100.0));
	CHECK(value(std::type_identity<kv::ground_range>{}) == Approx(5'000'000.0));
	CHECK(value(std::type_identity<kv::platform_fuel_remaining>{}) == Approx(10'000.0));
	CHECK(value(std::type_identity<kv::alternate_platform_altitude>{}) == Approx(19'000.0));
	CHECK(value(std::type_identity<kv::sensor_north_velocity>{}) == Approx(-327.0));
	CHECK(value(std::type_identity<kv::corner_longitude_point_1_full>{}) == Approx(180.0));
	CHECK(value(std::type_identity<kv::platform_sideslip_angle_full>{}) == Approx(-180.0));
	CHECK(uas->unknown().empty());
}

TEST_CASE("KLV ST 0601 extended signed fields decode the error value", "[klv][misb0601]") {
	auto fields = byte_vector{};
	append_field(fields, 32, hex("80 00"));
	append_field(fields, 50, hex("80 00"));
	append_field(fields, 80, hex("80 00"));
	append_field(fields, 90, hex("80 00 00 00"));

	auto const uas = decode_packet<kv::uas_datalink_ls>(make_uas_packet(fields));
	REQUIRE(uas);
	CHECK(uas->get<kv::offset_corner_latitude_point_4>().error() == field_error::error_indicator);
	CHECK(uas->get<kv::platform_angle_of_attack>().error() == field_error::error_indicator);
	CHECK(uas->get<kv::sensor_east_velocity>().error() == field_error::error_indicator);
	CHECK(uas->get<kv::platform_pitch_angle_full>().error() == field_error::error_indicator);
}

TEST_CASE("KLV ST 0601 enumeration fields decode with display names", "[klv][misb0601]") {
	auto fields = byte_vector{};
	append_field(fields, 34, hex("02"));
	append_field(fields, 63, hex("06"));
	append_field(fields, 77, hex("02"));
	append_field(fields, 125, hex("0C"));
	append_field(fields, 126, hex("05"));

	auto const uas = decode_packet<kv::uas_datalink_ls>(make_uas_packet(fields));
	REQUIRE(uas);
	CHECK(uas->get<kv::icing_detected>() == kv::icing_state::icing_detected);
	CHECK(uas->get<kv::sensor_field_of_view_name>() == kv::field_of_view_name::ultranarrow_2x);
	CHECK(uas->get<kv::operational_mode>() == kv::operation_type::training);
	CHECK(uas->get<kv::platform_status>() == kv::platform_state::landed_parked);
	CHECK(uas->get<kv::sensor_control_mode>() == kv::control_mode::auto_holding_position);

	auto const text = describe(*uas);
	REQUIRE(text.size() == 6);
	CHECK(text[0].name == "Icing Detected");
	CHECK(text[0].value == std::string{"Icing Detected"});
	CHECK(text[1].value == std::string{"2x Ultranarrow"});
	CHECK(text[2].value == std::string{"Training"});
	CHECK(text[3].value == std::string{"Landed-Parked"});
	CHECK(text[4].value == std::string{"Auto - Holding Position"});
}

TEST_CASE("KLV ST 0601 rejects an undefined enumeration value", "[klv][misb0601]") {
	auto fields = byte_vector{};
	append_field(fields, 34, hex("03"));
	append_field(fields, 125, hex("0D"));

	auto const uas = decode_packet<kv::uas_datalink_ls>(make_uas_packet(fields));
	REQUIRE(uas);
	CHECK(uas->get<kv::icing_detected>().error() == field_error::invalid_value);
	CHECK(uas->get<kv::platform_status>().error() == field_error::invalid_value);
}

TEST_CASE("KLV ST 0601 variable-length integer fields decode short encodings", "[klv][misb0601]") {
	auto fields = byte_vector{};
	append_field(fields, 39, hex("E2"));
	append_field(fields, 110, hex("01 51 80"));
	append_field(fields, 111, hex("0B B8"));
	append_field(fields, 133, hex("02 00"));
	append_field(fields, 136, hex("25"));
	append_field(fields, 137, hex("FF FE"));

	auto const uas = decode_packet<kv::uas_datalink_ls>(make_uas_packet(fields));
	REQUIRE(uas);
	CHECK(uas->get<kv::outside_air_temperature>() == -30.0);
	CHECK(uas->get<kv::time_airborne>() == uint32_t{86'400});
	CHECK(uas->get<kv::propulsion_unit_speed>() == uint32_t{3'000});
	CHECK(uas->get<kv::onboard_mi_storage_capacity>() == uint32_t{512});
	CHECK(uas->get<kv::leap_seconds>() == int32_t{37});
	CHECK(uas->get<kv::correction_offset>() == int64_t{-2});
}

TEST_CASE("KLV ST 0601 extended fields round-trip through a packet", "[klv][misb0601]") {
	auto const start = timestamp{std::chrono::microseconds{1'700'000'000'000'000}};

	// The example of MISB ST 1204.3 section 7.4.2
	auto const core_id = misb1204::parse_core_id(
	    "0170:F592-F023-7336-4AF8-AA91-62C0-0F2E-B2DA/16B7-4341-0008-41A0-BE36-5B5A-B96A-3645:D3"
	);
	REQUIRE(core_id);

	auto uas = uas_record{};
	uas.set<kv::precision_time_stamp>(start);
	uas.set<kv::offset_corner_latitude_point_2>(-0.0125);
	uas.set<kv::wind_direction>(270.0);
	uas.set<kv::platform_call_sign>("RAVEN 21");
	uas.set<kv::laser_prf_code>(uint16_t{1'688});
	uas.set<kv::sensor_field_of_view_name>(kv::field_of_view_name::continuous_zoom);
	uas.set<kv::alternate_platform_latitude>(-33.8688);
	uas.set<kv::event_start_time_utc>(start);
	uas.set<kv::operational_mode>(kv::operation_type::exercise);
	uas.set<kv::corner_longitude_point_3_full>(151.2093);
	uas.set<kv::miis_core_identifier>(*core_id);
	uas.set<kv::stream_designator>("BLUE");
	uas.set<kv::target_id>("TGT-0042");
	uas.set<kv::take_off_time>(start);
	uas.set<kv::correction_offset>(int64_t{-1'500});
	uas.set_error_indicator<kv::platform_sideslip_angle>();

	auto const bytes = encode_packet(uas);
	REQUIRE(bytes);

	auto const decoded = decode_packet<kv::uas_datalink_ls>(*bytes);
	REQUIRE(decoded);
	CHECK(decoded->get<kv::offset_corner_latitude_point_2>().value() == Approx(-0.0125).margin(1e-5));
	CHECK(decoded->get<kv::wind_direction>().value() == Approx(270.0).margin(1e-2));
	CHECK(decoded->get<kv::platform_call_sign>() == std::string{"RAVEN 21"});
	CHECK(decoded->get<kv::laser_prf_code>() == uint16_t{1'688});
	CHECK(decoded->get<kv::sensor_field_of_view_name>() == kv::field_of_view_name::continuous_zoom);
	CHECK(decoded->get<kv::alternate_platform_latitude>().value() == Approx(-33.8688).margin(1e-7));
	CHECK(decoded->get<kv::event_start_time_utc>() == start);
	CHECK(decoded->get<kv::operational_mode>() == kv::operation_type::exercise);
	CHECK(decoded->get<kv::corner_longitude_point_3_full>().value() == Approx(151.2093).margin(1e-7));
	CHECK(decoded->get<kv::miis_core_identifier>() == *core_id);
	CHECK(decoded->get<kv::stream_designator>() == std::string{"BLUE"});
	CHECK(decoded->get<kv::target_id>() == std::string{"TGT-0042"});
	CHECK(decoded->get<kv::take_off_time>() == start);
	CHECK(decoded->get<kv::correction_offset>() == int64_t{-1'500});
	CHECK(decoded->get<kv::platform_sideslip_angle>().error() == field_error::error_indicator);
	CHECK(decoded->unknown().empty());
}

TEST_CASE("KLV ST 0601 IMAPB fields decode", "[klv][misb0601]") {
	auto fields = byte_vector{};
	append_field(fields, 96, hex("00 0B 4B"));
	append_field(fields, 103, hex("4F E2 00"));
	append_field(fields, 109, hex("00 0C 40"));
	append_field(fields, 112, hex("16 80 00"));
	append_field(fields, 113, hex("01 C7 40"));
	append_field(fields, 117, hex("3D B8 00"));
	append_field(fields, 120, hex("32 00 00"));
	append_field(fields, 132, hex("02 57 C0"));
	append_field(fields, 134, hex("32 00"));  // A 2-byte encoding
	append_field(fields, 114, hex("D0 00 00"));  // NaN

	auto const uas = decode_packet<kv::uas_datalink_ls>(make_uas_packet(fields));
	REQUIRE(uas);
	CHECK(uas->get<kv::target_width_extended>() == 722.75);
	CHECK(uas->get<kv::density_altitude_extended>() == 40'000.0);
	CHECK(uas->get<kv::range_to_recovery_location>() == 12.25);
	CHECK(uas->get<kv::platform_course_angle>() == 90.0);
	CHECK(uas->get<kv::altitude_agl>() == 10.5);
	CHECK(uas->get<kv::sensor_azimuth_rate>() == -12.5);
	CHECK(uas->get<kv::onboard_mi_storage_percent_full>() == 50.0);
	CHECK(uas->get<kv::transmission_frequency>() == 2'400.0);
	CHECK(uas->get<kv::zoom_percentage>() == 50.0);
	CHECK(uas->get<kv::radar_altimeter>().error() == field_error::error_indicator);
	CHECK(uas->unknown().empty());
}

TEST_CASE("KLV ST 0601 IMAPB fields round-trip through a packet", "[klv][misb0601]") {
	auto uas = uas_record{};
	uas.set<kv::sensor_ellipsoid_height_extended>(1'234.567);
	uas.set<kv::alternate_platform_ellipsoid_height_extended>(-55.25);
	uas.set<kv::sensor_elevation_rate>(-0.75);
	uas.set<kv::sensor_roll_rate>(999.0);
	uas.set_error_indicator<kv::altitude_agl>();

	auto const bytes = encode_packet(uas);
	REQUIRE(bytes);

	auto const decoded = decode_packet<kv::uas_datalink_ls>(*bytes);
	REQUIRE(decoded);

	// One step is 2^-7 m for the heights and 2^-12 deg/s for the rates
	CHECK(decoded->get<kv::sensor_ellipsoid_height_extended>().value() == Approx(1'234.567).margin(1.0 / 128.0));
	CHECK(decoded->get<kv::alternate_platform_ellipsoid_height_extended>() == -55.25);
	CHECK(decoded->get<kv::sensor_elevation_rate>() == -0.75);
	CHECK(decoded->get<kv::sensor_roll_rate>() == 999.0);
	CHECK(decoded->get<kv::altitude_agl>().error() == field_error::error_indicator);
}

TEST_CASE("KLV ST 0601 rejects a target ID longer than 32 bytes", "[klv][misb0601]") {
	auto uas = uas_record{};
	uas.set<kv::target_id>(std::string(33, 'X'));
	CHECK_FALSE(encode_packet(uas));
}

TEST_CASE("KLV ST 0601 pack fields decode", "[klv][misb0601]") {
	auto fields = byte_vector{};
	append_field(fields, 81, hex("0A 14 5A 19"));
	append_field(fields, 115, hex("05 02 47 4F 00 04 59 F4 A6 AA 4A A8"));  // ID 5, "GO", time stamp
	append_field(fields, 116, hex("01 02 81 00"));
	append_field(fields, 121, hex("01 03"));
	append_field(fields, 122, hex("01 0E 03 55 53 41 00 03 44 45 55"));  // GENC, "USA", absent, "DEU"
	append_field(fields, 127, hex("81 EA 30 87 69"));  // 30000 / 1001
	append_field(fields, 128, hex("0C 01 00 00 03 20 00 00 05 78 56 49 53"));  // ID 1, 400 to 700 nm, "VIS"
	append_field(fields, 130, hex("0B 40 40 00 00 19 B0 00 00 07 D0 00"));  // Take-off only
	append_field(fields, 138, hex("02 06 00 00 03 45 4F 31 07 01 02 04 53 41 52 31"));
	append_field(fields, 139, hex("05"));
	append_field(fields, 140, hex("0A 01 02 00 03 92 04 03 41 47 4D"));
	append_field(fields, 141, hex("0C 01 00 01 03 28 00 00 00 32 00 00 00 03 02 FF FF"));
	append_field(fields, 142, hex("04 00 00 5A 00 06 16 80 00 2D 00 00"));  // Azimuth, elevation, no roll

	auto const uas = decode_packet<kv::uas_datalink_ls>(make_uas_packet(fields));
	REQUIRE(uas);

	CHECK(uas->get<kv::image_horizon_pixel_pack>() == kv::horizon_pixels{.x0 = 10, .y0 = 20, .x1 = 90, .y1 = 25});
	CHECK(
	    uas->get<kv::control_command>() ==
	    std::vector{kv::command_record{
	        .id = 5,
	        .text = "GO",
	        .time = timestamp{std::chrono::microseconds{1'224'807'209'913'000}},
	    }}
	);
	CHECK(uas->get<kv::control_command_verification_list>() == std::vector<uint32_t>{1, 2, 128});
	CHECK(uas->get<kv::active_wavelength_list>() == std::vector<uint32_t>{1, 3});
	CHECK(
	    uas->get<kv::country_codes>() ==
	    kv::countries{
	        .coding_method = sec::classifying_country_coding::genc_three_letter,
	        .overflight_country = "USA",
	        .country_of_manufacture = "DEU",
	    }
	);
	CHECK(uas->get<kv::sensor_frame_rate_pack>() == kv::frame_rate{.numerator = 30'000, .denominator = 1'001});
	CHECK(
	    uas->get<kv::wavelengths_list>() ==
	    std::vector{kv::wavelength_record{.id = 1, .minimum = 400.0, .maximum = 700.0, .name = "VIS"}}
	);

	// The recovery location is omitted, so it is the take-off location
	auto const base = kv::location{.latitude = 38.5, .longitude = -77.25, .height = 100.0};
	CHECK(uas->get<kv::airbase_locations>() == kv::airbases{.take_off = base, .recovery = base});

	CHECK(
	    uas->get<kv::payload_list>() ==
	    kv::payloads{
	        .count = 2,
	        .records =
	            {
	                kv::payload_record{.id = 0, .type = kv::payload_type::electro_optical, .name = "EO1"},
	                kv::payload_record{.id = 1, .type = kv::payload_type::radar, .name = "SAR1"},
	            },
	    }
	);
	CHECK(uas->get<kv::active_payloads>() == uint64_t{5});
	CHECK(
	    uas->get<kv::weapons_stores>() ==
	    std::vector{kv::weapons_store{
	        .station_id = 1,
	        .hardpoint_id = 2,
	        .carriage_id = 0,
	        .store_id = 3,
	        .general_status = kv::weapon_status::launch,
	        .engagement_status = kv::weapons_store::fuze_enabled | kv::weapons_store::weapon_armed,
	        .weapon_type = "AGM",
	    }}
	);
	CHECK(
	    uas->get<kv::waypoint_list>() ==
	    std::vector{
	        kv::waypoint_record{
	            .id = 1,
	            .order = 1,
	            .info = uint8_t{kv::waypoint_record::manual_mode | kv::waypoint_record::ad_hoc_source},
	            .position = kv::location{.latitude = -10.0, .longitude = 20.0},
	        },
	        kv::waypoint_record{.id = 2, .order = -1},
	    }
	);
	CHECK(
	    uas->get<kv::view_domain>() ==
	    kv::view_ranges{
	        .azimuth = kv::view_interval{.start = 0.0, .range = 360.0},
	        .elevation = kv::view_interval{.start = -90.0, .range = 180.0},
	    }
	);
	CHECK(uas->unknown().empty());
}

TEST_CASE("KLV ST 0601 pack fields describe their values", "[klv][misb0601]") {
	auto fields = byte_vector{};
	append_field(fields, 127, hex("1E"));
	append_field(fields, 140, hex("0A 01 02 00 03 92 04 03 41 47 4D"));

	auto const uas = decode_packet<kv::uas_datalink_ls>(make_uas_packet(fields));
	REQUIRE(uas);

	auto const text = describe(*uas);
	REQUIRE(text.size() == 3);
	CHECK(text[0].value == std::string{"30"});
	CHECK(text[0].units == "fps");
	CHECK(
	    text[1].value ==
	    std::string{"[{station: 1, hardpoint: 2, carriage: 0, store: 3, status: Launch, engagement: 0x9, type: \"AGM\"}]"}
	);
}

TEST_CASE("KLV ST 0601 pack fields reject malformed values", "[klv][misb0601]") {
	auto fields = byte_vector{};
	append_field(fields, 81, hex("0A 14 5A 19 00"));  // 5 bytes
	append_field(fields, 115, hex("05 02 47 4F 00 01 02"));  // A 3-byte time stamp
	append_field(fields, 122, hex("01 0E 00 00 00 00"));  // A fifth element
	append_field(fields, 127, hex("1E 00"));  // Denominator 0
	append_field(fields, 128, hex("05 01 02"));  // The length overruns the value
	append_field(fields, 140, hex("0A 01 02 00 03 C0 04 03 41 47 4D"));  // Status bits above bit 11
	append_field(fields, 142, hex("03 00 00 5A"));  // An odd interval length

	auto const uas = decode_packet<kv::uas_datalink_ls>(make_uas_packet(fields));
	REQUIRE(uas);
	CHECK(uas->get<kv::image_horizon_pixel_pack>().error() == field_error::bad_length);
	CHECK(uas->get<kv::control_command>().error() == field_error::bad_length);
	CHECK(uas->get<kv::country_codes>().error() == field_error::invalid_value);
	CHECK(uas->get<kv::sensor_frame_rate_pack>().error() == field_error::invalid_value);
	CHECK(uas->get<kv::wavelengths_list>().error() == field_error::invalid_value);
	CHECK(uas->get<kv::weapons_stores>().error() == field_error::invalid_value);
	CHECK(uas->get<kv::view_domain>().error() == field_error::bad_length);
}

TEST_CASE("KLV ST 0601 pack fields decode their optional parts", "[klv][misb0601]") {
	auto fields = byte_vector{};
	append_field(fields, 81, hex("00 FF 64 7F 00 00 00 00 00 00 00 00 FF FF FF FF FF FF FF FF"));
	append_field(fields, 115, hex("81 00 00"));  // ID 128, empty text, no time stamp
	append_field(fields, 122, hex("01 01 02 55 53"));  // Only the overflight country
	append_field(fields, 130, hex("00 08 40 40 00 00 19 B0 00 00"));  // Unknown take-off, recovery without height
	append_field(fields, 142, hex("00 04 00 00 5A 00"));  // No azimuth limit, a 4-byte elevation interval

	auto const uas = decode_packet<kv::uas_datalink_ls>(make_uas_packet(fields));
	REQUIRE(uas);

	auto const& horizon = uas->get<kv::image_horizon_pixel_pack>();
	REQUIRE(horizon);
	REQUIRE(horizon->locations);
	CHECK(horizon->locations->latitude0 == Approx(-90.0));
	CHECK(horizon->locations->longitude0 == Approx(-180.0));
	CHECK(horizon->locations->latitude1 == Approx(90.0));
	CHECK(horizon->locations->longitude1 == Approx(180.0));

	CHECK(uas->get<kv::control_command>() == std::vector{kv::command_record{.id = 128}});
	CHECK(
	    uas->get<kv::country_codes>() ==
	    kv::countries{.coding_method = sec::classifying_country_coding::iso_3166_two_letter, .overflight_country = "US"}
	);
	CHECK(
	    uas->get<kv::airbase_locations>() ==
	    kv::airbases{.recovery = kv::location{.latitude = 38.5, .longitude = -77.25}}
	);

	// With 2 bytes for each value, IMAPB(-180, 180, 2) maps 0x0000 to -180
	CHECK(
	    uas->get<kv::view_domain>() ==
	    kv::view_ranges{.elevation = kv::view_interval{.start = -180.0, .range = 360.0}}
	);
}

TEST_CASE("KLV ST 0601 pack fields round-trip through a packet", "[klv][misb0601]") {
	auto const base = kv::location{.latitude = 38.5, .longitude = -77.25, .height = 100.0};
	auto const field_site = kv::location{.latitude = -10.0, .longitude = 20.0};

	auto const horizon = kv::horizon_pixels{.x0 = 1, .y0 = 2, .x1 = 3, .y1 = 4};
	auto const command = kv::command_record{
	    .id = 300,
	    .text = "RETURN TO BASE",
	    .time = timestamp{std::chrono::microseconds{1'700'000'000'000'000}},
	};
	auto const countries = kv::countries{
	    .coding_method = sec::classifying_country_coding::genc_two_letter,
	    .overflight_country = "US",
	    .operator_country = "CA",
	};
	auto const rate = kv::frame_rate{.numerator = 60'000, .denominator = 1'001};
	auto const wavelengths = std::vector{
	    kv::wavelength_record{.id = 1, .minimum = 400.0, .maximum = 700.0, .name = "VIS"},
	    kv::wavelength_record{.id = 2, .minimum = 8'000.0, .maximum = 14'000.0, .name = "LWIR"},
	};
	auto const airbases = kv::airbases{.take_off = base, .recovery = field_site};
	auto const payloads = kv::payloads{
	    .count = 1,
	    .records = {kv::payload_record{.id = 0, .type = kv::payload_type::lidar, .name = "L1"}},
	};
	auto const stores = std::vector{kv::weapons_store{
	    .station_id = 200,
	    .general_status = kv::weapon_status::ready_all_up_round,
	    .engagement_status = kv::weapons_store::target_enabled,
	    .weapon_type = "GBU",
	}};
	auto const waypoints = std::vector{
	    kv::waypoint_record{.id = 7, .order = 3, .info = uint8_t{0}, .position = base},
	    kv::waypoint_record{.id = 8, .order = -2, .info = kv::waypoint_record::ad_hoc_source},
	};
	auto const view = kv::view_ranges{.elevation = kv::view_interval{.start = -90.0, .range = 180.0}};

	auto uas = uas_record{};
	uas.set<kv::image_horizon_pixel_pack>(horizon);
	uas.add<kv::control_command>(command);
	uas.set<kv::control_command_verification_list>({1, 300, 70'000});
	uas.set<kv::active_wavelength_list>({2});
	uas.set<kv::country_codes>(countries);
	uas.set<kv::sensor_frame_rate_pack>(rate);
	uas.set<kv::wavelengths_list>(wavelengths);
	uas.set<kv::airbase_locations>(airbases);
	uas.set<kv::payload_list>(payloads);
	uas.set<kv::active_payloads>(uint64_t{1});
	uas.set<kv::weapons_stores>(stores);
	uas.set<kv::waypoint_list>(waypoints);
	uas.set<kv::view_domain>(view);

	auto const bytes = encode_packet(uas);
	REQUIRE(bytes);

	auto const decoded = decode_packet<kv::uas_datalink_ls>(*bytes);
	REQUIRE(decoded);
	CHECK(decoded->get<kv::image_horizon_pixel_pack>() == horizon);
	CHECK(decoded->get<kv::control_command>() == std::vector{command});
	CHECK(decoded->get<kv::control_command_verification_list>() == std::vector<uint32_t>{1, 300, 70'000});
	CHECK(decoded->get<kv::active_wavelength_list>() == std::vector<uint32_t>{2});
	CHECK(decoded->get<kv::country_codes>() == countries);
	CHECK(decoded->get<kv::sensor_frame_rate_pack>() == rate);
	CHECK(decoded->get<kv::wavelengths_list>() == wavelengths);
	CHECK(decoded->get<kv::airbase_locations>() == airbases);
	CHECK(decoded->get<kv::payload_list>() == payloads);
	CHECK(decoded->get<kv::active_payloads>() == uint64_t{1});
	CHECK(decoded->get<kv::weapons_stores>() == stores);
	CHECK(decoded->get<kv::waypoint_list>() == waypoints);
	CHECK(decoded->get<kv::view_domain>() == view);
	CHECK(decoded->unknown().empty());
}

TEST_CASE("KLV ST 0601 pack encoding omits trailing absent elements", "[klv][misb0601]") {
	auto const encode_value = []<typename E>(std::type_identity<E>, value_t<E> const& value) {
		auto out = byte_vector{};
		auto writer = byte_writer{out};
		REQUIRE(E::format.encode(value, writer));
		return out;
	};

	auto const site = kv::location{.latitude = 38.5, .longitude = -77.25};
	CHECK(
	    encode_value(std::type_identity<kv::airbase_locations>{}, kv::airbases{.take_off = site, .recovery = site}) ==
	    hex("08 40 40 00 00 19 B0 00 00")
	);
	CHECK(
	    encode_value(std::type_identity<kv::country_codes>{}, kv::countries{.overflight_country = "US"}) ==
	    hex("01 01 02 55 53")
	);
	CHECK(
	    encode_value(std::type_identity<kv::country_codes>{}, kv::countries{.country_of_manufacture = "US"}) ==
	    hex("01 01 00 00 02 55 53")
	);
	CHECK(
	    encode_value(
	        std::type_identity<kv::view_domain>{},
	        kv::view_ranges{.elevation = kv::view_interval{.start = -90.0, .range = 180.0}}
	    ) == hex("00 06 16 80 00 2D 00 00")
	);
	CHECK(encode_value(std::type_identity<kv::sensor_frame_rate_pack>{}, kv::frame_rate{.numerator = 30}) == hex("1E"));
}

TEST_CASE("KLV ST 0601 waypoint encoding needs info before a position", "[klv][misb0601]") {
	auto out = byte_vector{};
	auto writer = byte_writer{out};
	auto const waypoint = kv::waypoint_record{.id = 1, .position = kv::location{}};
	CHECK(kv::waypoint_record_codec{}.encode(waypoint, writer) == std::unexpected(field_error::invalid_value));
}

TEST_CASE("KLV ST 0601 decodes an SDCC-FLP after the items that it covers", "[klv][misb0601][misb1010]") {
	auto const bytes = hex(
	    "41 01 0D 0D 04 55 95 B6 6D 0E 04 5B 53 60 C4 05 02 71 C2 "
	    "66 13 03 B3 04 80 3F 80 00 00 40 00 00 00 00 00 00 00 20 00 00"
	);

	auto const rec = decode_set<kv::uas_datalink_ls>(bytes);
	REQUIRE(rec);
	CHECK(*rec->get<kv::sensor_latitude>() == Approx(60.176822966978335));
	auto const& sdcc = rec->get<kv::sdcc_flp>();
	REQUIRE(sdcc);
	REQUIRE(sdcc->size() == 1);
	CHECK(sdcc->front().size == 3);
	CHECK(sdcc->front().members == std::vector<uint32_t>{13, 14, 5});
	CHECK(sdcc->front().sigma == std::vector{1.0, 2.0, 0.0});
	CHECK(sdcc->front().rho == std::vector{-0.5, 0.0, 0.0});

	// The encoder writes the covered items directly before the pack, in the order of its members
	CHECK(encode_set(*rec) == bytes);
}

TEST_CASE("KLV ST 0601 encodes the covered items directly before each SDCC-FLP", "[klv][misb0601][misb1010]") {
	auto uas = record<kv::uas_datalink_ls>{};
	uas.set<kv::platform_heading_angle>(90.0);
	uas.set<kv::mission_id>("M");
	uas.set<kv::sensor_latitude>(10.0);
	uas.add_unknown(unknown_field<uint32_t>{.id = 200, .value = hex("AB")});
	uas.add<kv::sdcc_flp>(sdcc_flp{
	    .size = 2,
	    .members = {kv::sensor_latitude::tag, kv::platform_heading_angle::tag},
	    .sigma = {1.0, 2.0},
	    .rho_length = 0,
	});
	uas.add<kv::sdcc_flp>(sdcc_flp{.size = 1, .members = {200}, .sigma = {3.0}, .rho_length = 0});

	auto const bytes = encode_set(uas);
	REQUIRE(bytes);
	CHECK(tags_of(*bytes) == std::vector<uint32_t>{kv::mission_id::tag, 13, 5, 102, 200, 102});

	auto const decoded = decode_set<kv::uas_datalink_ls>(*bytes);
	REQUIRE(decoded);
	CHECK(decoded->get<kv::sdcc_flp>() == uas.get<kv::sdcc_flp>());
	CHECK(encode_set(*decoded) == *bytes);

	SECTION("two packs cannot cover the same item") {
		uas.add<kv::sdcc_flp>(sdcc_flp{.size = 1, .members = {5}, .sigma = {4.0}, .rho_length = 0});
		CHECK(encode_set(uas).error().code == error_code::invalid_value);
	}

	SECTION("each covered item needs a value") {
		uas.erase<kv::sensor_latitude>();
		CHECK(encode_set(uas).error().code == error_code::invalid_value);
	}
}

TEST_CASE("KLV ST 0601 omits the checksum of a nested set", "[klv][misb0601][misb0903]") {
	// VMTI Local Set with Frame Width 1920 and the checksum 0xABCD. ST 0903.6-120 forbids this checksum.
	auto const bytes = hex("4A 08 08 02 07 80 01 02 AB CD");

	auto const rec = decode_set<kv::uas_datalink_ls>(bytes);
	REQUIRE(rec);
	auto const& moving = rec->get<kv::vmti_local_set>();
	REQUIRE(moving);
	CHECK(moving->get<vmti::frame_width>() == uint32_t{1920});
	CHECK(moving->get<vmti::checksum>() == uint16_t{0xABCD});

	// The encoder omits the checksum of an embedded set
	CHECK(encode_set(*rec) == hex("4A 04 08 02 07 80"));
}

TEST_CASE("KLV ST 0601 packet with each nested set round-trips", "[klv][misb0601]") {
	auto poi = record<rvt::poi_ls>{};
	poi.set<rvt::poi_number>(4);
	poi.set<rvt::poi_type>(rvt::interest_type::hostile);
	auto remote = record<rvt::rvt_ls>{};
	remote.set<rvt::mgrs_zone>(17);
	remote.add<rvt::poi_local_set>(poi);
	remote.add<rvt::poi_local_set>(poi);

	auto target = record<vmti::vtarget_ls>{};
	target.set<vmti::target_location>(vmti::location{.latitude = 10.0, .longitude = 20.0, .height = 30.0});
	auto moving = record<vmti::vmti_ls>{};
	moving.set<vmti::frame_width>(1920);
	moving.set<vmti::vtarget_series>({vmti::vtarget{.id = 7, .set = target}});

	auto radar = record<misb1206::sar_motion_imagery_ls>{};
	radar.set<misb1206::grazing_angle>(64.0);
	radar.set<misb1206::document_version>(1);

	auto range = record<misb1002::range_image_ls>{};
	range.set<misb1002::document_version>(2);
	range.add<misb1002::section_data_pack>(misb1002::section_data{
	    .section_x = 1,
	    .section_y = 1,
	    .measurements = mdarray<double>{.sizes = {2}, .elements = {10.0, 20.0}, .element_size = 4},
	    .plane_constant = 0.5,
	});

	auto registration = record<misb1601::geo_registration_ls>{};
	registration.set<misb1601::algorithm_name>("ALGO");

	auto composite = record<misb1602::composite_imaging_ls>{};
	composite.set<misb1602::z_order>(2);

	auto segment = record<kv::uas_datalink_ls>{};
	segment.set<kv::alternate_platform_name>("LARRY");

	auto uas = record<kv::uas_datalink_ls>{};
	uas.set<kv::precision_time_stamp>(micros(1'000'000));
	uas.set<kv::rvt_local_set>(remote);
	uas.set<kv::vmti_local_set>(moving);
	uas.set<kv::sar_motion_imagery_local_set>(radar);
	uas.set<kv::range_image_local_set>(range);
	uas.set<kv::geo_registration_local_set>(registration);
	uas.set<kv::composite_imaging_local_set>(composite);
	uas.add<kv::segment_local_set>(segment);
	uas.add<kv::amend_local_set>(segment);
	uas.set<kv::mission_id>("M");
	uas.add<kv::sdcc_flp>(sdcc_flp{.size = 1, .members = {kv::mission_id::tag}, .sigma = {2.5}, .rho_length = 0});
	uas.add<kv::control_command>(kv::command_record{.id = 1, .text = "UP"});
	uas.add<kv::control_command>(kv::command_record{.id = 2, .text = "DOWN"});

	auto const bytes = encode_packet(uas);
	REQUIRE(bytes);
	auto decoded = decode_packet<kv::uas_datalink_ls>(*bytes);
	REQUIRE(decoded);
	decoded->erase<kv::checksum>();
	CHECK(*decoded == uas);

	auto names = std::vector<std::string>{};
	for (auto const& entry : describe(*decoded)) {
		names.push_back(entry.name);
	}
	CHECK(std::ranges::contains(names, "RVT Local Set.POI Local Set[1].POI Type"));
	CHECK(std::ranges::contains(names, "VMTI Local Set.Frame Width"));
	CHECK(std::ranges::contains(names, "Range Image Local Set.Section Data Pack"));
	CHECK(std::ranges::contains(names, "Segment Local Set[0].Alternate Platform Name"));
	CHECK(std::ranges::contains(names, "Control Command"));
}
