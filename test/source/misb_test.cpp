#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdint>
#include <string>

#include "klv_test_util.hpp"
#include "klv/misb0102.hpp"
#include "klv/misb0601.hpp"
#include "klv/packet.hpp"

using namespace klv;
using Catch::Approx;
using klv_test::append_field;
using klv_test::append_packet;
using klv_test::hex;
using klv_test::make_bytes;

namespace kv = klv::misb0601;
namespace sec = klv::misb0102;

using uas_record = record<kv::uas_datalink_ls>;

static_assert(universal_set_definition<kv::uas_datalink_ls>);
static_assert(has_checksum<kv::uas_datalink_ls>);
static_assert(local_set_definition<sec::security_ls>);
static_assert(!universal_set_definition<sec::security_ls>);
static_assert(kv::uas_datalink_ls::fields::size == 30);
static_assert(sec::security_ls::fields::size == 15);

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
