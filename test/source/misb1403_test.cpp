#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <vector>

#include "klv_test_util.hpp"
#include "klv/misb0102.hpp"
#include "klv/misb0601.hpp"
#include "klv/misb1204.hpp"
#include "klv/misb1206.hpp"
#include "klv/misb1403.hpp"
#include "klv/packet.hpp"

using namespace klv;
using klv_test::hex;
using klv_test::micros;

namespace kv = klv::misb0601;
namespace sar = klv::misb1206;
namespace sec = klv::misb0102;
namespace st = klv::misb1403;

using uas_record = record<kv::uas_datalink_ls>;

namespace {

/// A SARMI record with each item of Table 1
[[nodiscard]]
auto make_imagery_record() -> uas_record {
	// The security set of the MISB ST 0902 example: no conditional items
	auto const security = decode_set<sec::security_ls>(
	    hex("01 01 01 02 01 07 03 05 2F 2F 55 53 41 0C 01 07 0D 06 00 55 00 53 00 41 16 02 00 0A")
	);
	REQUIRE(security);

	// The example of MISB ST 1204 Table 7
	auto const core_id = misb1204::core_id_codec{}.decode(hex(
	    "01 70 F5 92 F0 23 73 36 4A F8 AA 91 62 C0 0F 2E B2 DA 16 B7 43 41 00 08 41 A0 BE 36 5B 5A B9 6A 36 45"
	));
	REQUIRE(core_id);

	auto radar = record<sar::sar_motion_imagery_ls>{};
	radar.set<sar::grazing_angle>(30.0);
	radar.set<sar::ground_plane_squint_angle>(-10.0);
	radar.set<sar::look_direction>(sar::look_side::right);
	radar.set<sar::image_plane>(sar::image_plane_type::ground);
	radar.set<sar::range_resolution>(0.5);
	radar.set<sar::cross_range_resolution>(0.5);
	radar.set<sar::range_image_plane_pixel_size>(0.25);
	radar.set<sar::cross_range_image_plane_pixel_size>(0.25);
	radar.set<sar::image_rows>(uint16_t{1024});
	radar.set<sar::image_columns>(uint16_t{2048});
	radar.set<sar::range_direction_angle>(90.0);
	radar.set<sar::true_north>(45.0);
	radar.set<sar::range_layover_angle>(270.0);
	radar.set<sar::document_version>(uint8_t{2});

	auto uas = uas_record{};
	uas.set<kv::precision_time_stamp>(micros(1'700'000'000'000'000));
	uas.set<kv::mission_id>("SARMI 1");
	uas.set<kv::platform_designation>("MQ-1C");
	uas.set<kv::image_source_sensor>("SAR");
	uas.set<kv::image_coordinate_system>("WGS-84");
	uas.set<kv::sensor_latitude>(34.0);
	uas.set<kv::sensor_longitude>(-117.0);
	uas.set<kv::slant_range>(12'000.0);
	uas.set<kv::frame_center_latitude>(34.1);
	uas.set<kv::frame_center_longitude>(-117.1);
	uas.set<kv::corner_latitude_point_1_full>(34.11);
	uas.set<kv::corner_longitude_point_1_full>(-117.11);
	uas.set<kv::corner_latitude_point_2_full>(34.11);
	uas.set<kv::corner_longitude_point_2_full>(-117.09);
	uas.set<kv::corner_latitude_point_3_full>(34.09);
	uas.set<kv::corner_longitude_point_3_full>(-117.09);
	uas.set<kv::corner_latitude_point_4_full>(34.09);
	uas.set<kv::corner_longitude_point_4_full>(-117.11);
	uas.set<kv::security_local_set>(*security);
	uas.set<kv::uas_datalink_ls_version_number>(uint8_t{19});
	uas.set<kv::sensor_ellipsoid_height_extended>(7'500.0);
	uas.set<kv::frame_center_height_above_ellipsoid>(400.0);
	uas.set<kv::miis_core_identifier>(*core_id);
	uas.set<kv::sar_motion_imagery_local_set>(radar);
	return uas;
}

/// The record with the items of Table 2
[[nodiscard]]
auto with_reference_frame(uas_record uas) -> uas_record {
	auto radar = *uas.get<kv::sar_motion_imagery_local_set>();
	radar.set<sar::reference_frame_precision_time_stamp>(micros(1'699'999'990'000'000));
	radar.set<sar::reference_frame_grazing_angle>(31.0);
	radar.set<sar::reference_frame_ground_plane_squint_angle>(-11.0);
	radar.set<sar::reference_frame_range_direction_angle>(91.0);
	radar.set<sar::reference_frame_range_layover_angle>(271.0);
	uas.set<kv::sar_motion_imagery_local_set>(radar);
	return uas;
}

/// The items from `first` to `last`, each missing
[[nodiscard]]
auto missing_range(st::item first, st::item last) -> std::vector<st::finding> {
	auto findings = std::vector<st::finding>{};
	for (auto index = static_cast<size_t>(first); index <= static_cast<size_t>(last); ++index) {
		findings.push_back({.item = static_cast<st::item>(index), .reason = shortfall::missing});
	}
	return findings;
}

/// Encode and decode the record as a packet
[[nodiscard]]
auto round_trip(uas_record const& uas) -> uas_record {
	auto const bytes = encode_packet(uas);
	REQUIRE(bytes);
	auto const decoded = decode_packet<kv::uas_datalink_ls>(*bytes);
	REQUIRE(decoded);
	return *decoded;
}

}  // namespace

TEST_CASE("KLV ST 1403 SAR imagery needs the items of Table 1", "[klv][misb1403]") {
	auto const imagery = round_trip(make_imagery_record());
	CHECK(st::check(imagery, st::product::imagery).empty());

	auto const table_2 =
	    missing_range(st::item::reference_frame_precision_time_stamp, st::item::reference_frame_range_layover_angle);
	CHECK(st::check(imagery, st::product::coherent_change) == table_2);

	auto const change = round_trip(with_reference_frame(make_imagery_record()));
	CHECK(st::check(change, st::product::imagery).empty());
	CHECK(st::check(change, st::product::coherent_change).empty());
}

TEST_CASE("KLV ST 1403 SAR items come from the nested ST 1206 set", "[klv][misb1403]") {
	auto uas = make_imagery_record();

	auto radar = *uas.get<kv::sar_motion_imagery_local_set>();
	radar.erase<sar::true_north>();
	radar.set_empty<sar::image_rows>();
	uas.set<kv::sar_motion_imagery_local_set>(radar);
	CHECK(
	    st::check(uas, st::product::imagery)
	    == std::vector<st::finding>{
	        {.item = st::item::image_rows, .reason = shortfall::missing},
	        {.item = st::item::true_north, .reason = shortfall::missing},
	    }
	);

	uas.erase<kv::sar_motion_imagery_local_set>();
	CHECK(st::check(uas, st::product::imagery) == missing_range(st::item::grazing_angle, st::item::document_version));
	CHECK(
	    st::check(uas, st::product::coherent_change)
	    == missing_range(st::item::grazing_angle, st::item::reference_frame_range_layover_angle)
	);
}

TEST_CASE("KLV ST 1403 accepts only the tags of its tables", "[klv][misb1403]") {
	auto uas = make_imagery_record();

	SECTION("tag 75 or tag 104, but not both") {
		uas.erase<kv::sensor_ellipsoid_height_extended>();
		uas.set<kv::sensor_ellipsoid_height>(7'500.0);
		CHECK(st::check(uas, st::product::imagery).empty());

		uas.set<kv::sensor_ellipsoid_height_extended>(7'500.0);
		CHECK(
		    st::check(uas, st::product::imagery)
		    == std::vector{st::finding{.item = st::item::sensor_ellipsoid_height, .reason = shortfall::conflict}}
		);
	}

	SECTION("the MSL altitudes and the offset corners do not meet a requirement") {
		uas.erase<kv::sensor_ellipsoid_height_extended>();
		uas.set<kv::sensor_true_altitude>(7'500.0);
		uas.erase<kv::frame_center_height_above_ellipsoid>();
		uas.set<kv::frame_center_elevation>(400.0);
		uas.erase<kv::corner_latitude_point_3_full>();
		uas.set<kv::offset_corner_latitude_point_3>(-0.01);
		CHECK(
		    st::check(uas, st::product::imagery)
		    == std::vector<st::finding>{
		        {.item = st::item::corner_latitude_point_3, .reason = shortfall::missing},
		        {.item = st::item::sensor_ellipsoid_height, .reason = shortfall::missing},
		        {.item = st::item::frame_center_height_above_ellipsoid, .reason = shortfall::missing},
		    }
		);
	}

	SECTION("the error indicator is a report") {
		uas.set_error_indicator<kv::corner_latitude_point_1_full>();
		CHECK(st::check(uas, st::product::imagery).empty());
	}
}

TEST_CASE("KLV ST 1403 requirements have names with their tags", "[klv][misb1403]") {
	CHECK(to_string(st::item::corner_longitude_point_4) == "Corner Longitude Point 4 (89)");
	CHECK(to_string(st::item::sensor_ellipsoid_height) == "Sensor Ellipsoid Height (75 | 104)");
	CHECK(to_string(st::item::document_version) == "Document Version (95/28)");
	CHECK(to_string(st::item::reference_frame_range_layover_angle) == "Reference Frame Range Layover Angle (95/27)");
	CHECK(st::table_1_count == 42);
	CHECK(st::item_count == 47);
	for (auto index = size_t{0}; index < st::item_count; ++index) {
		CHECK_FALSE(to_string(static_cast<st::item>(index)).empty());
	}
}
