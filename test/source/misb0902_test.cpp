// The example packets are the examples of MISB ST 0902.8 Annex C.

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <string_view>
#include <vector>

#include "klv_test_util.hpp"
#include "klv/misb0102.hpp"
#include "klv/misb0601.hpp"
#include "klv/misb0902.hpp"
#include "klv/packet.hpp"

using namespace klv;
using klv_test::hex;
using klv_test::micros;

namespace kv = klv::misb0601;
namespace sec = klv::misb0102;
namespace mm = klv::misb0902;

using uas_record = record<kv::uas_datalink_ls>;

namespace {

/// "Dynamic & Constant" packet of Table 10: each MISMMS item
constexpr auto complete_packet = std::string_view{
    "06 0E 2B 34 02 0B 01 01 0E 01 03 01 01 00 00 00 81 D2 02 08 00 04 60 50 58 4E 01 80 03 0A 4D 69 "
    "73 73 69 6F 6E 20 31 32 05 02 71 C2 06 02 FD 3D 07 02 08 B8 0A 08 50 72 65 64 61 74 6F 72 0B 07 "
    "45 4F 20 4E 6F 73 65 0C 0E 47 65 6F 64 65 74 69 63 20 57 47 53 38 34 0D 04 55 95 B6 6D 0E 04 5B "
    "53 60 C4 0F 02 C2 21 10 02 CD 9C 11 02 D9 17 12 04 72 4A 0A 20 13 04 87 F8 4B 86 14 04 7D C5 5E "
    "CE 15 04 03 83 09 26 16 02 12 81 17 04 F1 01 A2 29 18 04 14 BC 08 2B 19 02 34 F3 30 1C 01 01 01 "
    "02 01 07 03 05 2F 2F 55 53 41 0C 01 07 0D 06 00 55 00 53 00 41 16 02 00 0A 41 01 06 5E 22 01 70 "
    "F5 92 F0 23 73 36 4A F8 AA 91 62 C0 0F 2E B2 DA 16 B7 43 41 00 08 41 A0 BE 36 5B 5A B9 6A 36 45 "
    "01 02 3E 1E"
};

/// "Dynamic Only" packet of Table 11: the items that change often
constexpr auto dynamic_packet = std::string_view{
    "06 0E 2B 34 02 0B 01 01 0E 01 03 01 01 00 00 00 61 02 08 00 04 60 50 58 4E 01 80 05 02 71 C2 06 "
    "02 FD 3D 07 02 08 B8 0D 04 55 95 B6 6D 0E 04 5B 53 60 C4 0F 02 C2 21 10 02 CD 9C 11 02 D9 17 12 "
    "04 72 4A 0A 20 13 04 87 F8 4B 86 14 04 7D C5 5E CE 15 04 03 83 09 26 16 02 12 81 17 04 F1 01 A2 "
    "29 18 04 14 BC 08 2B 19 02 34 F3 41 01 06 01 02 5C 2B"
};

/// Precision Time Stamp of the example packets
constexpr auto example_time = int64_t{1'231'798'102'000'000};

/// The requirements that the dynamic packet does not meet
auto const constant_items = std::vector{
    mm::item::mission_id,
    mm::item::platform_designation,
    mm::item::image_source_sensor,
    mm::item::image_coordinate_system,
    mm::item::security_classification,
    mm::item::classifying_country_coding_method,
    mm::item::classifying_country,
    mm::item::object_country_coding_method,
    mm::item::object_country_codes,
    mm::item::security_metadata_version,
    mm::item::miis_core_identifier,
};

[[nodiscard]]
auto decode_example(std::string_view text) -> uas_record {
	auto const decoded = decode_packet<kv::uas_datalink_ls>(hex(text));
	REQUIRE(decoded);
	return *decoded;
}

/// The example packet with a new Precision Time Stamp
[[nodiscard]]
auto at_time(std::string_view text, std::chrono::microseconds offset) -> uas_record {
	auto rec = decode_example(text);
	rec.set<kv::precision_time_stamp>(micros(example_time) + offset);
	return rec;
}

[[nodiscard]]
auto missing(std::vector<mm::item> const& items) -> std::vector<mm::finding> {
	auto findings = std::vector<mm::finding>{};
	for (auto const which : items) {
		findings.push_back({.item = which, .reason = shortfall::missing});
	}
	return findings;
}

}  // namespace

TEST_CASE("KLV ST 0902 complete example packet meets each requirement", "[klv][misb0902]") {
	auto const uas = decode_example(complete_packet);
	CHECK(uas.unknown().empty());
	CHECK(uas.get<kv::precision_time_stamp>() == micros(example_time));
	CHECK(mm::check(uas).empty());

	// The example omits the conditional security items
	auto const& security = uas.get<kv::security_local_set>();
	REQUIRE(security);
	CHECK_FALSE(security->has<sec::sci_shi_information>());
	CHECK_FALSE(security->has<sec::caveats>());
	CHECK_FALSE(security->has<sec::releasing_instructions>());

	// The check does not require the checksum, so a record to encode meets the requirements
	auto unsent = uas;
	unsent.erase<kv::checksum>();
	CHECK(mm::check(unsent).empty());
}

TEST_CASE("KLV ST 0902 dynamic example packet does not report the constant items", "[klv][misb0902]") {
	auto const uas = decode_example(dynamic_packet);
	CHECK(mm::check(uas) == missing(constant_items));
	CHECK(mm::reports(uas, mm::item::sensor_altitude));
	CHECK_FALSE(mm::reports(uas, mm::item::mission_id));
}

TEST_CASE("KLV ST 0902 requirements accept any one of their tags", "[klv][misb0902]") {
	auto uas = decode_example(complete_packet);

	uas.erase<kv::platform_pitch_angle>();
	CHECK(mm::check(uas) == missing({mm::item::platform_pitch_angle}));
	uas.set<kv::platform_pitch_angle_full>(-0.5);
	CHECK(mm::check(uas).empty());

	uas.erase<kv::target_width>();
	uas.set<kv::target_width_extended>(700.0);
	uas.erase<kv::frame_center_elevation>();
	uas.set<kv::frame_center_height_above_ellipsoid>(3'200.0);
	CHECK(mm::check(uas).empty());

	SECTION("tag 75 and tag 104 exclude each other") {
		uas.erase<kv::sensor_true_altitude>();
		uas.set<kv::sensor_ellipsoid_height_extended>(14'000.0);
		CHECK(mm::check(uas).empty());

		uas.set<kv::sensor_ellipsoid_height>(14'000.0);
		auto const conflict = mm::finding{.item = mm::item::sensor_altitude, .reason = shortfall::conflict};
		CHECK(mm::check(uas) == std::vector{conflict});

		uas.erase<kv::sensor_ellipsoid_height>();
		uas.erase<kv::sensor_ellipsoid_height_extended>();
		CHECK(mm::check(uas) == missing({mm::item::sensor_altitude}));
	}
}

TEST_CASE("KLV ST 0902 empty items and items that did not decode are not reports", "[klv][misb0902]") {
	auto uas = decode_example(complete_packet);

	uas.set_empty<kv::mission_id>();
	uas.assign<kv::slant_range>(std::unexpected(field_error::out_of_range));
	uas.set_error_indicator<kv::sensor_latitude>();
	CHECK(reports<kv::sensor_latitude>(uas));
	CHECK(mm::check(uas) == missing({mm::item::mission_id, mm::item::slant_range}));

	auto security = *uas.get<kv::security_local_set>();
	security.erase<sec::object_country_codes>();
	uas.set<kv::security_local_set>(security);
	CHECK_FALSE(reports<kv::security_local_set, sec::object_country_codes>(uas));
	CHECK(reports<kv::security_local_set, sec::object_country_coding_method>(uas));
	CHECK(mm::check(uas) == missing({mm::item::mission_id, mm::item::slant_range, mm::item::object_country_codes}));

	uas.assign<kv::security_local_set>(std::unexpected(field_error::bad_length));
	CHECK_FALSE(reports<kv::security_local_set, sec::security_classification>(uas));
}

TEST_CASE("KLV ST 0902 report monitor applies the 30-second rule", "[klv][misb0902]") {
	using std::chrono::microseconds;
	using std::chrono::seconds;

	auto monitor = mm::report_monitor{};
	CHECK(monitor.overdue().empty());

	REQUIRE(monitor.add(at_time(complete_packet, seconds{0})));
	REQUIRE(monitor.add(at_time(dynamic_packet, seconds{10})));
	REQUIRE(monitor.add(at_time(dynamic_packet, seconds{30})));
	CHECK(monitor.overdue().empty());

	REQUIRE(monitor.add(at_time(dynamic_packet, seconds{30} + microseconds{1})));
	CHECK(monitor.overdue() == constant_items);

	// An earlier time stamp does not move the time back
	REQUIRE(monitor.add(at_time(dynamic_packet, seconds{20})));
	CHECK(monitor.overdue() == constant_items);

	REQUIRE(monitor.add(at_time(complete_packet, seconds{31})));
	CHECK(monitor.overdue().empty());

	auto untimed = at_time(complete_packet, seconds{90});
	untimed.erase<kv::precision_time_stamp>();
	CHECK_FALSE(monitor.add(untimed));
	CHECK(monitor.overdue().empty());

	SECTION("the interval of each item starts at the first record") {
		auto stream = mm::report_monitor{};
		REQUIRE(stream.add(at_time(dynamic_packet, seconds{0})));
		REQUIRE(stream.add(at_time(dynamic_packet, seconds{30})));
		CHECK(stream.overdue().empty());
		REQUIRE(stream.add(at_time(dynamic_packet, seconds{31})));
		CHECK(stream.overdue() == constant_items);
	}
}

TEST_CASE("KLV ST 0902 requirements have names with their tags", "[klv][misb0902]") {
	CHECK(to_string(mm::item::precision_time_stamp) == "Precision Time Stamp (2)");
	CHECK(to_string(mm::item::sensor_altitude) == "Sensor Altitude (15 | 75 | 104)");
	CHECK(to_string(mm::item::security_metadata_version) == "Security Metadata Version (48/22)");
	CHECK(to_string(mm::item::miis_core_identifier) == "MIIS Core Identifier (94)");
	CHECK(to_string(shortfall::conflict) == "conflict");
	for (auto index = size_t{0}; index < mm::item_count; ++index) {
		CHECK_FALSE(to_string(static_cast<mm::item>(index)).empty());
	}
}
